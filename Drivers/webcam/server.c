
#include "webcam.h"

#define BUFFER_SIZE 1024

// Procesa solicitudes HTTP
void process_request(int client_fd, CaptureBuffer *buffers) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_fd, buffer, sizeof(buffer));
    printf("Request received:\n%s\n", buffer);

    if (strstr(buffer, "GET /video-feed")) {
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
        printf("Streaming video feed...\n");
        start_stream();

        if (capture_video(buffers, client_fd) == -1) {
            printf("error\n");
        }

        stop_stream();
    } else {
        char response[] = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_fd, response, sizeof(response) - 1, 0);
    }

    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    CaptureBuffer *buffers;

    signal(SIGPIPE, SIG_IGN); // Ignorar SIGPIPE para evitar crash al desconectar clientes

    // Inicializar la webcam
    if (init_webcam(&buffers) == -1) {
        fprintf(stderr, "Failed to initialize webcam\n");
        exit(EXIT_FAILURE);
    }

    // Crear el socket del servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Aceptar y procesar solicitudes de clientes
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        process_request(client_fd, buffers);
    }

    close_webcam(buffers);
    close(server_fd);

    return 0;
}