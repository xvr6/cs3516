#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 8888

void handle_client(int client_sock);
void send_response(int client_sock, const char *status, const char *content_type, const char *body);
void send_file_response(int client_sock, const char *filename);

int main(int argc, char *argv[]) {
    int server_sock, client_sock, port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Get port number from command line or use default
    port = (argc == 2) ? atoi(argv[1]) : DEFAULT_PORT;

    // Create server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("ERROR: Unable to open socket");
        return 1;
    }

    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Unable to bind");
        close(server_sock);
        return 1;
    }

    // Start listening for connections
    if (listen(server_sock, 10) < 0) {
        perror("ERROR: Unable to listen");
        close(server_sock);
        return 1;
    }

    printf("Server is listening on port %d...\n", port);

    // Accept and handle incoming connections
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("ERROR: Unable to accept connection");
            continue;
        }

        handle_client(client_sock);
        close(client_sock);  // Close the connection after handling
    }

    // Close the server socket (not reached due to infinite loop, unless interrupted)
    close(server_sock);
    return 0;
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Read the HTTP request from the client
    int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("ERROR: Failed to read from socket");
        return;
    }

    // Parse the HTTP request to check if it's a GET request
    if (strncmp(buffer, "GET ", 4) == 0) {
        // Get the requested file name from the GET request
        char *file_name = strtok(buffer + 4, " ");
        if (file_name[0] == '/') file_name++;  // Remove leading '/'

        // Default to index.html if no file is specified
        if (strlen(file_name) == 0) {
            file_name = "index.html";
        }

        send_file_response(client_sock, file_name);
    } else {
        // Send a 400 Bad Request response for non-GET requests
        send_response(client_sock, "400 Bad Request", "text/plain", "Only GET method is supported.");
    }
}

void send_response(int client_sock, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    int body_length = strlen(body);

    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             status, content_type, body_length, body);

    send(client_sock, response, strlen(response), 0);
}

void send_file_response(int client_sock, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Send 404 Not Found response if the file does not exist
        send_response(client_sock, "404 Not Found", "text/plain", "404 Not Found");
        return;
    }

    // Get the file contents
    char file_content[BUFFER_SIZE];
    size_t bytes_read = fread(file_content, 1, sizeof(file_content) - 1, file);
    file_content[bytes_read] = '\0';  // Null-terminate the file content
    fclose(file);

    // Send the file content as HTTP response
    send_response(client_sock, "200 OK", "text/html", file_content);
}
