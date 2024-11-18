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

        if (button_pressed == 0x01 && recording == 0) // Start recording
        {
            if(start_video_recording() == -1) {
                streaming = 0;
                pthread_mutex_unlock(&key_mutex);
                break;
            }
            recording = 1;
        } 
        else if (button_pressed == 0x01 && recording == 1)  // Stop recording
        {
            recording = 0;
            if (finish_video_recording() == -1) {
                streaming = 0;
                pthread_mutex_unlock(&key_mutex);
                break;
            }
        }
        else if (button_pressed == 0x02)  // Take picture
        {
            take_picture = 1;
        }

        pthread_mutex_unlock(&key_mutex);

    }

    return NULL;
}




int start_stream()
{
    if (ioctl(webcam_fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Starting capture");
        close(webcam_fd);
        return -1;
    }
    streaming = 1;

    printf("Streaming started.\n");
    return 0;
}

void stop_stream()
{
    ioctl(webcam_fd, VIDIOC_STREAMOFF, &type);
    printf("Streaming stopped.\n");
}



void close_webcam(CaptureBuffer *buffers)
{
    // Cleanup
    for (size_t i = 0; i < req.count; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }
    free(buffers);
    close(webcam_fd);
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




int capture_video(CaptureBuffer *buffers, int client_fd) 
{   
    while(1) {

        pthread_mutex_lock(&key_mutex);

        if (streaming == 0) {
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return 0;
        } 

        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // Dequeue buffer
        if (ioctl(webcam_fd, VIDIOC_DQBUF, &buf) == -1) {
            perror("Dequeueing buffer");
            streaming = 0;
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return -1;
        }
        
        
        // Write the MJPEG frame to file, buf.index indicates which buffer is ready with data
        if (recording)
            fwrite(buffers[buf.index].start, buf.bytesused, 1, video_file);

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
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return 0;
        }

        // Send frame header
        char frame_header[128];
        int header_len = snprintf(frame_header, sizeof(frame_header), FRAME_HEADER, buf.bytesused);
        if (send(client_fd, frame_header, header_len, 0) == -1){
            perror("Sending frame header failed. Disconnecting.");
            if (recording) finish_video_recording();
            streaming = 0;
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return 0;
        }

        // Send frame data
        if (send(client_fd, buffers[buf.index].start, buf.bytesused, 0) == -1){
            perror("Sending frame data failed. Disconnecting.");
            if (recording) finish_video_recording();
            streaming = 0;
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return 0;
        }

        
        // Re-queue the buffer
        if (ioctl(webcam_fd, VIDIOC_QBUF, &buf) == -1) {
            perror("Re-queueing buffer");\
            streaming = 0;
            close(client_fd);
            pthread_mutex_unlock(&key_mutex);
            return -1;
        }

        pthread_mutex_unlock(&key_mutex);
        usleep(100);
    }

    return 0;
}



