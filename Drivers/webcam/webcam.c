#include "webcam.h"

// Webcam variables
static struct v4l2_requestbuffers req;
static enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
char recording = 0;
char streaming = 0;
char take_picture = 0;
static int webcam_fd;

FILE *video_file;


// Gamepad variables
static int gamepad_fd;
pthread_t gamepad_thread;
pthread_mutex_t key_mutex;
unsigned char button_pressed;


// Aplica filtro de escala de grises
void apply_grayscale(unsigned char *rgb_data, int width, int height) {
    int total_pixels = width * height;
    for (int i = 0; i < total_pixels; ++i) {
        unsigned char *pixel = &rgb_data[i * 3];
        unsigned char gray = (pixel[0] + pixel[1] + pixel[2]) / 3;
        pixel[0] = gray; // R
        pixel[1] = gray; // G
        pixel[2] = gray; // B
    }
}


// Decodifica MJPEG a RGB utilizando libjpeg
unsigned char* decode_mjpeg_to_rgb(unsigned char *mjpeg_data, size_t mjpeg_size, int *width, int *height) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, mjpeg_data, mjpeg_size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    int row_stride = cinfo.output_width * cinfo.output_components;

    unsigned char *rgb_data = (unsigned char *)malloc(row_stride * cinfo.output_height);
    if (!rgb_data) {
        perror("Failed to allocate memory for RGB data");
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = rgb_data + cinfo.output_scanline * row_stride;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return rgb_data;
}


unsigned char* encode_rgb_to_mjpeg(unsigned char *rgb_data, int width, int height, int quality, size_t *jpeg_size) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *jpeg_data = NULL;
    unsigned long jpeg_mem_size = 0;

    // Initialize the JPEG compression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Set up the memory destination for JPEG data
    jpeg_mem_dest(&cinfo, &jpeg_data, &jpeg_mem_size);

    // Set the image width, height, and color components
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3; // RGB has 3 components
    cinfo.in_color_space = JCS_RGB;

    // Set default compression parameters and adjust quality
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE); // TRUE to limit to baseline-JPEG

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Write scanlines
    int row_stride = width * 3; // RGB has 3 bytes per pixel
    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char *row_pointer[1];
        row_pointer[0] = &rgb_data[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);

    // Save the size of the output JPEG
    *jpeg_size = jpeg_mem_size;

    // Clean up
    jpeg_destroy_compress(&cinfo);

    return jpeg_data; // Caller is responsible for freeing this memory
}


void write_bmp(const char *filename, unsigned char *rgb_data, int width, int height) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    unsigned char file_header[14] = {
        'B', 'M',
        0, 0, 0, 0,
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    unsigned char info_header[40] = {
        40, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 0,
        24, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    int row_padded = (width * 3 + 3) & (~3);
    int file_size = 54 + row_padded * height;

    file_header[2] = (unsigned char)(file_size);
    file_header[3] = (unsigned char)(file_size >> 8);
    file_header[4] = (unsigned char)(file_size >> 16);
    file_header[5] = (unsigned char)(file_size >> 24);

    info_header[4] = (unsigned char)(width);
    info_header[5] = (unsigned char)(width >> 8);
    info_header[6] = (unsigned char)(width >> 16);
    info_header[7] = (unsigned char)(width >> 24);
    info_header[8] = (unsigned char)(-height);
    info_header[9] = (unsigned char)(-height >> 8);
    info_header[10] = (unsigned char)(-height >> 16);
    info_header[11] = (unsigned char)(-height >> 24);

    fwrite(file_header, 1, 14, file);
    fwrite(info_header, 1, 40, file);

    unsigned char *row = (unsigned char *)malloc(row_padded);
    for (int y = 0; y < height; ++y) {
        memcpy(row, &rgb_data[(height - y - 1) * width * 3], width * 3);
        fwrite(row, 1, row_padded, file);
    }

    free(row);
    fclose(file);
}



int init_webcam(CaptureBuffer **buffers) 
{
    webcam_fd = open(VIDEO_DEVICE, O_RDWR);
    if (webcam_fd == -1) {
        perror("Opening video device");
        return -1;
    }

    printf("Webcam device opened. Configuring...\n");

    // Set the video format to MJPEG
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(webcam_fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Setting video format");
        close(webcam_fd);
        return -1;
    }

    
    // Request buffers
    memset(&req, 0, sizeof(req));
    req.count = 4; // Number of buffers
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(webcam_fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Requesting buffer");
        close(webcam_fd);
        return -1;
    }


    // Map the buffers
    *buffers = calloc(req.count, sizeof(CaptureBuffer));
    for (size_t i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(webcam_fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("Querying buffer");
            close(webcam_fd);
            return -1;
        }

        (*buffers)[i].length = buf.length;
        (*buffers)[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, webcam_fd, buf.m.offset);

        if ((*buffers)[i].start == MAP_FAILED) {
            perror("Mapping buffer");
            close(webcam_fd);
            return -1;
        }

        // Queue buffer
        if (ioctl(webcam_fd, VIDIOC_QBUF, &buf) == -1) {
            perror("Queueing buffer");
            close(webcam_fd);
            return -1;
        }
    }

    if (ioctl(webcam_fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Starting webcam stream");
        close(webcam_fd);
        return -1;
    }

    printf("Webcam device ready.\n");

    return 0;
}


int init_gamepad()
{
    gamepad_fd = open(CONTROL_DEVICE, O_RDONLY);
    if (gamepad_fd < 0) {
        perror("Failed to open gamepad device");
        return -1;
    }

    printf("Gamepad device opened. Configuring...\n");

    pthread_mutex_init(&key_mutex, NULL);

    // Start the button listener thread
    if (pthread_create(&gamepad_thread, NULL, button_listener, NULL) != 0) {
        perror("Failed to create thread");
        return -1;
    }

    printf("Gamepad device ready.\n");

    return 0;
}


void *button_listener(void *arg) 
{
    while (1) {

        if (streaming == 0) 
            break;
        usleep(100000);  // Sleep to reduce CPU usage


        // Read button state from the device
        if (read(gamepad_fd, &button_pressed, sizeof(button_pressed)) <= 0)
            continue;

        if (button_pressed == 0)  // No button pressed
            continue;
            
            
        pthread_mutex_lock(&key_mutex);

        if (button_pressed == 0x02)  // Take picture
        {
            take_picture = 1;
        }

        pthread_mutex_unlock(&key_mutex);

    }

    return NULL;
}



void close_webcam(CaptureBuffer *buffers)
{
    ioctl(webcam_fd, VIDIOC_STREAMOFF, &type);

    // Cleanup
    for (size_t i = 0; i < req.count; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }
    free(buffers);
    close(webcam_fd);
    printf("Webcam closed.\n");
}

void close_gamepad()
{
    close(gamepad_fd);

    // Join threads
    pthread_join(gamepad_thread, NULL);
    pthread_mutex_destroy(&key_mutex);

    printf("Gamepad device closed.\n");
}




int save_frame_as_jpg(CaptureBuffer buffer, int buf_size) 
{
    FILE *file = fopen(IMAGE_OUT_DIR, "wb");
    if (!file) {
        perror("Opening file for writing");
        return -1;
    }
    
    fwrite(buffer.start, buf_size, 1, file);

    fclose(file);

    printf("Picture saved.\n");
    return 0;
}


/*

int start_video_recording()
{
    printf("Recording video...\n");
    // Open the output MJPEG file
    video_file = fopen(VIDEO_OUT_DIR, "wb");
    if (!video_file) {
        perror("Opening video output file");
        return -1;
    }
    return 0;
}

int finish_video_recording()
{
    fclose(video_file);

    // Convert to AVI with FFmpeg 
    const char *command = "ffmpeg -r 30 -i frames/video.mjpeg -c:v copy -r 30 frames/video.avi -y";
    if (system(command) == -1) {
        perror("Error running ffmpeg command");
        return -1;
    }

    printf("Recording stopped.\n");
    return 0;
}
*/


//int capture_video(CaptureBuffer *buffers, int client_fd) 
int capture_video(int *bytes_used) 
{   

    /*
    if (streaming == 0) {
        close(client_fd);
        pthread_mutex_unlock(&key_mutex);
        return 0;
    } */

    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // Dequeue buffer
    if (ioctl(webcam_fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Dequeueing buffer");
        //streaming = 0;
        //pthread_mutex_unlock(&key_mutex);
        return -1;
    }
    
    usleep(10000);

    *bytes_used = buf.bytesused;
    
    // Write the MJPEG frame to file, buf.index indicates which buffer is ready with data
    //if (recording)
    //    fwrite(buffers[buf.index].start, buf.bytesused, 1, video_file);
    
    /*
    // Apply filter
    int width;
    int height;
    size_t jpeg_size;
    unsigned char *rgb_data = decode_mjpeg_to_rgb((unsigned char *)buffers[buf.index].start, buf.bytesused, &width, &height);
    
    
    apply_grayscale(rgb_data, width, height);
    unsigned char* mjpeg_frame = encode_rgb_to_mjpeg(rgb_data, width, height, 50, &jpeg_size);

    if (take_picture) {
        if (save_frame_as_jpg(buffers[buf.index], buf.bytesused) == -1) {
            perror("Saving picture.");
            streaming = 0;
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return -1;
        }
        take_picture = 0;
    }


    // Send frame boundary
    if (send(client_fd, FRAME_BOUNDARY, strlen(FRAME_BOUNDARY), 0) == -1){
        perror("Sending frame boundary failed. Disconnecting.");
        if (recording) finish_video_recording();
        streaming = 0;
        close(client_fd);
        pthread_mutex_unlock(&key_mutex);
        return 0;
    }

    
    // Send frame header
    char frame_header[128];
    int header_len = snprintf(frame_header, sizeof(frame_header), FRAME_HEADER, (int)jpeg_size);
    if (send(client_fd, frame_header, header_len, 0) == -1){
        perror("Sending frame header failed. Disconnecting.");
        if (recording) finish_video_recording();
        streaming = 0;
        close(client_fd);
        pthread_mutex_unlock(&key_mutex);
        return 0;
    }

    // Send frame data
    if (send(client_fd, mjpeg_frame, jpeg_size, 0) == -1){
        perror("Sending frame data failed. Disconnecting.");
        if (recording) finish_video_recording();
        streaming = 0;
        close(client_fd);
        pthread_mutex_unlock(&key_mutex);
        return 0;
    }
    */

    
    // Re-queue the buffer
    if (ioctl(webcam_fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Re-queueing buffer");
        //streaming = 0;
        //pthread_mutex_unlock(&key_mutex);
        return -1;
    }


    return buf.index;
}



