#include "webcam.h"

#define TMP_INPUT_FILE "temp_input.bmp"
#define TMP_OUTPUT_FILE "temp_output.bmp"

// Shared state for the live_streaming status
volatile int live_streaming = 0;
pthread_mutex_t stream_mutex;

CaptureBuffer *buffers;


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
        int frame_index = capture_video(buffers);
        if (frame_index == -1) {
            perror("Failed to get frame");
            break;
        }
        
        // Send frame boundary
        if (send(client_sock, FRAME_BOUNDARY, strlen(FRAME_BOUNDARY), 0) == -1){
            break;
        }
        
        // Send frame header
        char frame_header[128];
        int header_len = snprintf(frame_header, sizeof(frame_header), FRAME_HEADER, (int)buffers[frame_index].length);
        if (send(client_sock, frame_header, header_len, 0) == -1){
            break;
        }
        
        // Send frame data
        if (send(client_sock, buffers[frame_index].start, buffers[frame_index].length, 0) == -1){
            break;
        }

        pthread_mutex_lock(&stream_mutex);
        if (!live_streaming) {
            pthread_mutex_unlock(&stream_mutex);
            break;
        }
        pthread_mutex_unlock(&stream_mutex);
        printf("send %d\n", frame_index);
        usleep(1000000);
    }

    close(client_sock);
    printf("Stream closed.\n");
    return NULL;
}

void handle_photo_gallery_request(int client_sock, char *request) {
    // Encuentra el inicio del cuerpo
    char *body = strstr(request, "\r\n\r\n") + 4;
    if (!body) {
        fprintf(stderr, "Cuerpo de la solicitud no encontrado.\n");
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }

    // Detectar el boundary
    char *boundary_start = strstr(request, "boundary=");
    if (!boundary_start) {
        fprintf(stderr, "Boundary no encontrado en los encabezados.\n");
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }
    boundary_start += 9; // Salta "boundary="
    char boundary[256];
    snprintf(boundary, sizeof(boundary), "--%s", boundary_start);

    // Encuentra y procesa la parte del filtro
    char *filter_part = strstr(body, "name=\"filter\"");
    if (!filter_part) {
        fprintf(stderr, "Parte del filtro no encontrada.\n");
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }
    filter_part = strstr(filter_part, "\r\n\r\n") + 4; // Salta encabezados
    char filter_string[256];
    sscanf(filter_part, "%s", filter_string);

    // Encuentra y procesa la parte de la imagen
    char *image_part = strstr(body, "name=\"image\"");
    if (!image_part) {
        fprintf(stderr, "Parte de la imagen no encontrada.\n");
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }
    image_part = strstr(image_part, "\r\n\r\n") + 4; // Salta encabezados
    char *image_end = strstr(image_part, boundary) - 2; // Encuentra el final de la imagen
    if (!image_end) {
        fprintf(stderr, "Fin de la imagen no encontrado.\n");
        const char *response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }

    // Guarda la imagen en un archivo
    FILE *input_file = fopen("temp_input.bmp", "wb");
    fwrite(image_part, 1, image_end - image_part, input_file);
    fclose(input_file);

    // Ejecutar filtro
    pid_t pid = fork();
    if (pid == 0) {
        execl("./execute_filter", "execute_filter", filter_string, "temp_input.bmp", "temp_output.bmp", NULL);
        exit(1);
    }
    wait(NULL);

    // Lee y envía la imagen procesada al cliente
    FILE *output_file = fopen("temp_output.bmp", "rb");
    if (!output_file) {
        perror("Error al abrir archivo de salida");
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }
    fseek(output_file, 0, SEEK_END);
    long output_size = ftell(output_file);
    rewind(output_file);

    char *output_data = malloc(output_size);
    fread(output_data, 1, output_size, output_file);
    fclose(output_file);

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: image/bmp\r\n"
             "Content-Length: %ld\r\n\r\n",
             output_size);
    send(client_sock, header, strlen(header), 0);
    send(client_sock, output_data, output_size, 0);

    free(output_data);
    close(client_sock);
}

// Manejar solicitudes HTTP
void handle_request(int client_sock) {
    char buffer[4096];
    recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (strncmp(buffer, "POST /start", 11) == 0) {
        pthread_mutex_lock(&stream_mutex);
        live_streaming = 1;
        pthread_mutex_unlock(&stream_mutex);
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "live_streaming started";
        send(client_sock, response, strlen(response), 0);
    } else if (strncmp(buffer, "POST /stop", 10) == 0) {
        pthread_mutex_lock(&stream_mutex);
        live_streaming = 0;
        pthread_mutex_unlock(&stream_mutex);
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "live_streaming stopped";
        send(client_sock, response, strlen(response), 0);
    } else if (strncmp(buffer, "GET /video-stream", 17) == 0) {
        pthread_t stream_thread;
        int *arg = malloc(sizeof(int));
        *arg = client_sock;
        pthread_create(&stream_thread, NULL, stream_video, arg);
        pthread_detach(stream_thread);
    } else if (strncmp(buffer, "POST /apply-filter", 18) == 0) {
        handle_photo_gallery_request(client_sock, buffer);
    } else {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
    }
    close(client_sock);
}

// Main del servidor
int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);

    if (init_webcam(&buffers) == -1) {
        fprintf(stderr, "Failed to initialize webcam\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&stream_mutex, NULL);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

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