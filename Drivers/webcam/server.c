#include "webcam.h"


// Shared state for the live_streaming status
volatile int live_streaming = 0;
pthread_mutex_t stream_mutex;

CaptureBuffer *buffers;


// Stream video over an HTTP connection
void *stream_video(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    printf("here 0\n");

    
    char response_header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Access-Control-Allow-Origin: *\r\n\r\n";

    printf("here 2\n");
    send(client_sock, response_header, strlen(response_header), 0);

    printf("here 3\n");
    
    while (1) {
        printf("here 4\n");
        
        
        // Get an MJPEG frame
        int frame_index = capture_video(buffers);
        printf("index: %d\n", frame_index);
        if (frame_index == -1) {
            perror("Failed to get frame");
            break;
        }
        printf("here 5\n");
        // Send frame boundary
        if (send(client_sock, FRAME_BOUNDARY, strlen(FRAME_BOUNDARY), 0) == -1){
            break;
        }
        printf("here 6\n");
        // Send frame header
        char frame_header[128];
        int header_len = snprintf(frame_header, sizeof(frame_header), FRAME_HEADER, (int)buffers[frame_index].length);
        if (send(client_sock, frame_header, header_len, 0) == -1){
            break;
        }
        printf("here 7\n");
        // Send frame data
        if (send(client_sock, buffers[frame_index].start, buffers[frame_index].length, 0) == -1){
            break;
        }

        printf("here 8\n");
        pthread_mutex_lock(&stream_mutex);
        if (!live_streaming) {
            pthread_mutex_unlock(&stream_mutex);
            break;
        }
        pthread_mutex_unlock(&stream_mutex);
        printf("here 9\n");
        usleep(2000000);
    }

    close(client_sock);
    printf("Stream closed.\n");
    return NULL;
}

// Handle HTTP requests
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

    pthread_mutex_init(&stream_mutex, NULL);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080\n");

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&address, &addr_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        handle_request(client_sock);
    }

    close(server_fd);
    pthread_mutex_destroy(&stream_mutex);

    close_webcam(buffers);
    return 0;
}
