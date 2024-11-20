#ifndef __WEBCAM_H
#define __WEBCAM_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <termios.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <jpeglib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>



#define VIDEO_DEVICE "/dev/video0"
#define CONTROL_DEVICE "/dev/gamepad_dev"
#define WIDTH 640
#define HEIGHT 480
#define FPS 30
#define VIDEO_OUT_DIR "frames/video.mjpeg"
#define IMAGE_OUT_DIR "frames/photo.bmp"
#define MJPEG_QUALITY 50

#define PORT 8080
#define IP "192.168.18.47"
#define FRAME_BOUNDARY "--frame\r\n"
#define FRAME_HEADER "Content-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n"



typedef struct {
    void *start;
    size_t length;
} CaptureBuffer;


// Initializes the webcam device and prepares it for capturing
int init_webcam(CaptureBuffer **buffers);

// Closes the webcam device
void close_webcam(CaptureBuffer *buffers);

// Capture and save video function
int capture_video_frame(int *bytes_used);

// Saves a frame as a .bmp file
int save_frame_as_bmp(unsigned char *rgb_data);

// Decodes MJPEG to RGB
unsigned char* decode_mjpeg_to_rgb(unsigned char *mjpeg_data, size_t mjpeg_size);

// Encodes RGB to MJPEG
unsigned char* encode_rgb_to_mjpeg(unsigned char *rgb_data, int *jpeg_size);

// Creates and opens file to save the recorded video
int start_video_recording();

// Closes the file and converts the video from MJPEG to AVI
int finish_video_recording();



#endif

