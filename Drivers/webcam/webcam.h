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


#define VIDEO_DEVICE "/dev/video0"
#define WIDTH 640
#define HEIGHT 480
#define FPS 30
#define VIDEO_OUT_DIR "frames/video.mjpeg"
#define IMAGE_OUT_DIR "frames/image.jpg"

#define PORT 8080
#define IP "192.168.18.47"
#define FRAME_BOUNDARY "--frame\r\n"
#define FRAME_HEADER "Content-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n"

#define CONTROL_DEVICE "/dev/gamepad_dev"

typedef struct {
    void *start;
    size_t length;
} CaptureBuffer;


// Initializes the webcam device and prepares it for capturing
int init_webcam(CaptureBuffer **buffers);

// Initializes the gamepad device and starts a thread to listen for button presses
int init_gamepad();

// Thread function for gamepad button handling
void *button_listener(void *arg);

// Star capturing frames
int start_stream();

// Stop capturing frames
void stop_stream();

// Closes the webcam device
void close_webcam(CaptureBuffer *buffers);

// Closes the gamepad device
void close_gamepad();

// Capture and save video function
int capture_video(CaptureBuffer *buffers, int client_fd);

// Saves a frame as a .jpg file
int save_frame_as_jpg(CaptureBuffer buffer, int buf_size);

// Creates and opens file to save the recorded video
int start_video_recording();

// Closes the file and converts the video from MJPEG to AVI
int finish_video_recording();



#endif