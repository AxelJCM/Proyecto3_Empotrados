#include "webcam.h"
#include "filtros.h"

// Shared state for the live_streaming status
volatile int live_streaming = 0;
pthread_mutex_t stream_mutex;
CaptureBuffer *buffers;  // Buffer to capture frames

// Gamepad variables
static int gamepad_fd;
pthread_t gamepad_thread;
pthread_mutex_t key_mutex;
unsigned char button_pressed;

// Frame processing variables
unsigned char* rgb_frame;
unsigned char* mjpeg_frame;
unsigned char* send_frame;
unsigned char take_picture;
char filter = 0;


// Listens to gamepad button presses
void *button_listener(void *arg) 
{
    // 1 read to clear buffer
    if (read(gamepad_fd, &button_pressed, sizeof(button_pressed)) < 0) {
        perror("Error reading gamepad.");
        return NULL;
    }

    while (1) {
        usleep(100000);  // Sleep to reduce CPU usage

        // Read button state from the device
        if (read(gamepad_fd, &button_pressed, sizeof(button_pressed)) < 0) {
            perror("Error reading gamepad.");
            break;
        }

        if (button_pressed == 0)  // No button pressed
            continue;
            
            
        pthread_mutex_lock(&key_mutex);

        switch (button_pressed)
        {
            case 0x01:
                take_picture = 1;
                printf("Taking picture...\n");
                break;
            case 0x02:
                filter = (filter == 'b') ? 0 : 'b'; 
                printf("Blur filter.\n");
                break;
            case 0x04:
                filter = (filter == 'g') ? 0 : 'g'; 
                printf("Grayscale filter.\n");
                break;
            case 0x08:
                filter = (filter == 'r') ? 0 : 'r'; 
                printf("Reflect filter.\n");
                break;
            case 0x10:
                filter = (filter == 's') ? 0 : 's'; 
                printf("Sepia filter.\n");
                break;
            case 0x20:
                filter = (filter == 'e') ? 0 : 'e'; 
                printf("Edges filter.\n");
                break;
            case 0x40:
                filter = (filter == 'p') ? 0 : 'p'; 
                printf("Pixelate filter.\n");
                break;
            case 0x80:
                filter = (filter == 'z') ? 0 : 'z'; 
                printf("Sharpen filter.\n");
                break;
            default:
                break;
        }

        pthread_mutex_unlock(&key_mutex);

    }

    return NULL;
}


// Stream video over an HTTP connection
void *stream_video(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    printf("Stream started.\n");
    
    char response_header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Access-Control-Allow-Origin: *\r\n\r\n";

    send(client_sock, response_header, strlen(response_header), 0);

    while (1) {
        // Get an MJPEG frame
        int frame_size;
        int frame_index = capture_video_frame(&frame_size);
        if (frame_index == -1) {
            perror("Failed to get frame");
            break;
        }

        send_frame = (unsigned char *)buffers[frame_index].start;


        // Frame processing
        pthread_mutex_lock(&key_mutex);
        if (take_picture || filter) 
        {
            rgb_frame = decode_mjpeg_to_rgb(send_frame, (size_t)frame_size);

            if (rgb_frame) 
            {
                if (filter) {
                    if (apply_filter(rgb_frame, WIDTH, HEIGHT, filter) == -1) {
                        perror("Error applying the filter.");
                        pthread_mutex_unlock(&key_mutex);
                        break;
                    }
                    else {
                        mjpeg_frame = encode_rgb_to_mjpeg(rgb_frame, &frame_size);
                        if (mjpeg_frame) 
                            send_frame = mjpeg_frame;
                        else
                            send_frame = (unsigned char *)buffers[frame_index].start;
                    }
                }

                if (take_picture) {
                    if (save_frame_as_bmp(rgb_frame) == -1) {
                        perror("Error saving bmp picture.");
                    }
                    take_picture = 0;
                }
                
                free(rgb_frame);
                rgb_frame = NULL;
            }
        }
        pthread_mutex_unlock(&key_mutex);
        
        // Send frame boundary
        if (send(client_sock, FRAME_BOUNDARY, strlen(FRAME_BOUNDARY), 0) == -1) break;
        // Send frame header
        char frame_header[128];
        int header_len = snprintf(frame_header, sizeof(frame_header), FRAME_HEADER, frame_size);
        if (send(client_sock, frame_header, header_len, 0) == -1) break;
        // Send frame data
        if (send(client_sock, send_frame, frame_size, 0) == -1) break;
        


        if (mjpeg_frame) {
            free(mjpeg_frame);
            mjpeg_frame = NULL;
        }

        pthread_mutex_lock(&stream_mutex);
        if (!live_streaming) {
            pthread_mutex_unlock(&stream_mutex);
            break;
        }
        pthread_mutex_unlock(&stream_mutex);

        usleep(1000);
    }

    close(client_sock);
    printf("Stream closed.\n");
    return NULL;
}


// Actualización de la función de manejo de solicitudes
void handle_request(int client_sock) {
    char buffer[4096];
    recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (strncmp(buffer, "POST /start", 11) == 0) 
    {
        pthread_mutex_lock(&stream_mutex);
        live_streaming = 1;
        pthread_mutex_unlock(&stream_mutex);
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "live_streaming started";
        send(client_sock, response, strlen(response), 0);
    } 
    else if (strncmp(buffer, "POST /stop", 10) == 0) 
    {
        pthread_mutex_lock(&stream_mutex);
        live_streaming = 0;
        pthread_mutex_unlock(&stream_mutex);
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "live_streaming stopped";
        send(client_sock, response, strlen(response), 0);
    } 
    else if (strncmp(buffer, "GET /video-stream", 17) == 0) 
    {
        pthread_t stream_thread;
        int *arg = malloc(sizeof(int));
        *arg = client_sock;
        pthread_create(&stream_thread, NULL, stream_video, arg);
        pthread_detach(stream_thread);
        return;
    } 
    else {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
    }

    close(client_sock);
}



// Main server loop
int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);

    if (init_webcam(&buffers) == -1) {
        fprintf(stderr, "Failed to initialize device\n");
        exit(EXIT_FAILURE);
    }

    gamepad_fd = open(CONTROL_DEVICE, O_RDONLY);
    if (gamepad_fd < 0) {
        perror("Failed to open gamepad device");
        close_webcam(buffers);
        exit(EXIT_FAILURE);
    }
    printf("Gamepad device opened. Configuring...\n");


    pthread_mutex_init(&stream_mutex, NULL);
    pthread_mutex_init(&key_mutex, NULL);

    // Start the button listener thread
    if (pthread_create(&gamepad_thread, NULL, button_listener, NULL) != 0) {
        perror("Failed to create gamepad thread");
        close_webcam(buffers);
        close(gamepad_fd);
        exit(EXIT_FAILURE);
    }
    pthread_detach(gamepad_thread);
    printf("Gamepad device ready.\n");

    // Socket configuration //

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        close_webcam(buffers);
        close(gamepad_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        close_webcam(buffers);
        close(gamepad_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        close_webcam(buffers);
        close(gamepad_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080\n");

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&address, &addr_len);
        if (client_sock < 0) {
            perror("Accept client failed");
            continue;
        }

        handle_request(client_sock);
    }


    // Closing of devices

    close(server_fd);
    close_webcam(buffers);
    close(gamepad_fd);

    pthread_mutex_destroy(&stream_mutex);
    pthread_mutex_destroy(&key_mutex);

    printf("Gamepad device closed.\n");
    return 0;
}



