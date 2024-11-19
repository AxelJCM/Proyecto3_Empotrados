
#include "webcam.h"


int handle_client(CaptureBuffer *buffers, int client_fd) 
{
    // Send the HTTP MJPEG headers
    char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Access-Control-Allow-Origin: *\r\n\r\n";
    
    if (send(client_fd, header, strlen(header), 0) == -1){
        perror("Sending HTTP MJPEG headers failed. Disconnecting.");
        close(client_fd);
        return 0;
    }


    if(start_stream() == -1) return -1;
    if(init_gamepad() == -1) return -1;

    // Start video capturing loop
    if (capture_video(buffers, client_fd) == -1) 
        perror("Failure while capturing video.");

    close_gamepad();
    stop_stream();


    printf("Client disconnected.\n");
    
    return 0;
}



int main() {

    CaptureBuffer *buffers;

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Ignore SIGPIPE to handle client disconnects gracefully
    signal(SIGPIPE, SIG_IGN);

    if (init_webcam(&buffers) == -1) {
        fprintf(stderr, "Failed to initialize device\n");
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        if (handle_client(buffers, client_fd) == -1)
            perror("Socket creation failed");
    }


    close_webcam(buffers);

    return 0;
}

