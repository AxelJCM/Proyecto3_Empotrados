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



// Nueva función: manejar solicitudes de procesamiento de imágenes
void handle_image_processing_request(int client_sock, char *request) {
    // Extraer el cuerpo de la solicitud (imagen y filtros)
    char *body = strstr(request, "\r\n\r\n") + 4; // El cuerpo empieza después de los headers
    char *filters = strstr(body, "\nfilters=") + 9; // Obtener filtros desde el cuerpo
    char *image_data = strstr(body, "\n\n") + 2;   // La imagen binaria comienza después de los filtros

    // Escribir la imagen recibida en un archivo temporal
    FILE *input_file = fopen(TMP_INPUT_FILE, "wb");
    if (!input_file) {
        perror("Error creando archivo temporal para la imagen de entrada");
        return;
    }
    fwrite(image_data, 1, strlen(image_data), input_file);
    fclose(input_file);

    // Crear un proceso hijo para ejecutar el filtro
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo: ejecutar execute_filter
        execl("./execute_filter", "execute_filter", filters, TMP_INPUT_FILE, TMP_OUTPUT_FILE, NULL);
        perror("Error ejecutando el programa de filtros");
        exit(1);
    }

    // Esperar a que termine el proceso hijo
    int status;
    waitpid(pid, &status, 0);

    if (status != 0) {
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "Error aplicando filtros";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }

    // Leer el archivo de salida
    FILE *output_file = fopen(TMP_OUTPUT_FILE, "rb");
    if (!output_file) {
        perror("Error abriendo archivo de salida temporal");
        const char *response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
            "Error procesando imagen";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        return;
    }
    fseek(output_file, 0, SEEK_END);
    long output_size = ftell(output_file);
    rewind(output_file);

    char *output_data = malloc(output_size);
    int res = fread(output_data, 1, output_size, output_file);
    if (res < 0) perror("Error reading file.");
    fclose(output_file);

    // Enviar respuesta al cliente con la imagen procesada
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: image/bmp\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Content-Length: %ld\r\n\r\n",
             output_size);
    send(client_sock, header, strlen(header), 0);
    send(client_sock, output_data, output_size, 0);

    free(output_data);
    close(client_sock);
}

// Actualización de la función de manejo de solicitudes
void handle_request(int client_sock) {
    char buffer[4096];
    recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    if (strncmp(buffer, "POST /apply-filters", 19) == 0) {
        handle_image_processing_request(client_sock, buffer);
    } else if (strncmp(buffer, "POST /start", 11) == 0) {
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
        return;
    } else {
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



