#include "webcam.h"

// Webcam variables
static struct v4l2_requestbuffers req;
static enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
static int webcam_fd;
FILE *video_file;


unsigned char* encode_rgb_to_mjpeg(unsigned char *rgb_data, int *jpeg_size) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char *mjpeg_data = NULL;
    unsigned long jpeg_mem_size = 0;

    // Initialize the JPEG compression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Set up memory destination for MJPEG
    jpeg_mem_dest(&cinfo, &mjpeg_data, &jpeg_mem_size);

    // Set the image width, height, and color components
    cinfo.image_width = WIDTH;
    cinfo.image_height = HEIGHT;
    cinfo.input_components = 3; // RGB has 3 components
    cinfo.in_color_space = JCS_RGB;

    // Compression
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, MJPEG_QUALITY, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    // Write scanlines
    int row_stride = WIDTH * 3; // RGB, 3 bytes per pixel
    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char *row_pointer[1];
        row_pointer[0] = &rgb_data[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);

    // Size of the output JPEG
    *jpeg_size = (int)jpeg_mem_size;

    // Clean up
    jpeg_destroy_compress(&cinfo);

    return mjpeg_data;
}


unsigned char* decode_mjpeg_to_rgb(unsigned char *mjpeg_data, size_t mjpeg_size) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, mjpeg_data, mjpeg_size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

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


int save_frame_as_bmp(unsigned char *rgb_data) {
    FILE *file = fopen(IMAGE_OUT_DIR, "wb");
    if (!file) {
        perror("Failed to open bmp image file.");
        return -1;
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

    int row_padded = (WIDTH * 3 + 3) & (~3);
    int file_size = 54 + row_padded * HEIGHT;

    file_header[2] = (unsigned char)(file_size);
    file_header[3] = (unsigned char)(file_size >> 8);
    file_header[4] = (unsigned char)(file_size >> 16);
    file_header[5] = (unsigned char)(file_size >> 24);

    info_header[4] = (unsigned char)(WIDTH);
    info_header[5] = (unsigned char)(WIDTH >> 8);
    info_header[6] = (unsigned char)(WIDTH >> 16);
    info_header[7] = (unsigned char)(WIDTH >> 24);
    info_header[8] = (unsigned char)(-HEIGHT);
    info_header[9] = (unsigned char)(-HEIGHT >> 8);
    info_header[10] = (unsigned char)(-HEIGHT >> 16);
    info_header[11] = (unsigned char)(-HEIGHT >> 24);

    if (fwrite(file_header, 1, 14, file) == -1){
        perror("Error writing bmp image header.");
        fclose(file);
        return -1;
    }
    if (fwrite(info_header, 1, 40, file) == -1){
        perror("Error writing bmp image header.");
        fclose(file);
        return -1;
    }

    unsigned char *row = (unsigned char *)malloc(row_padded);
    for (int y = 0; y < HEIGHT; ++y) {
        memcpy(row, &rgb_data[(HEIGHT - y - 1) * WIDTH * 3], WIDTH * 3);
        if (fwrite(row, 1, row_padded, file) == -1) {
            perror("Error writing bmp image data.");
            fclose(file);
            return -1;
        }
    }

    free(row);
    fclose(file);
    printf("Picture saved.\n");
    return 0;
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



//int capture_video(CaptureBuffer *buffers, int client_fd) 
int capture_video_frame(int *bytes_used) 
{   
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
    
    usleep(1000);
    *bytes_used = buf.bytesused;
    int index = buf.index;
    
    // Re-queue the buffer
    if (ioctl(webcam_fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Re-queueing buffer");
        //streaming = 0;
        //pthread_mutex_unlock(&key_mutex);
        return -1;
    }

    return index;
}



