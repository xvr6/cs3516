#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

// Print usage information
void print_usage(const char *prog_name) {
    printf("Usage: %s [-p] server_url port_number\n", prog_name);
}

// Calculate round-trip time (RTT) in milliseconds
long calculate_rtt(struct timeval start, struct timeval end) {
    return ((end.tv_sec - start.tv_sec) * 1000) + ((end.tv_usec - start.tv_usec) / 1000);
}

int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc < 3 || argc > 4) {
        print_usage(argv[0]);
        return 1;
    }

    // Check if RTT flag is set
    int rtt_flag = (argc == 4 && strcmp(argv[1], "-p") == 0);
    if (rtt_flag) argv++;

    char *server_url = argv[1];
    int port = atoi(argv[2]);

    // Extract path from URL
    char *path = strchr(server_url, '/');
    if (path) {
        *path++ = '\0';
    } else {
        path = "/";
    }

    // Get server address
    struct hostent *server = gethostbyname(server_url);
    if (!server) {
        fprintf(stderr, "ERROR: No such host\n");
        return 1;
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR: Unable to open socket");
        return 1;
    }

    // Set up server address structure
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)};
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    struct timeval start, end;
    if (rtt_flag) gettimeofday(&start, NULL);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Connection failed");
        close(sock);
        return 1;
    }

    if (rtt_flag) {
        gettimeofday(&end, NULL);
        printf("RTT: %ld ms\n", calculate_rtt(start, end));
    }

    // Create HTTP GET request
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, server_url);

    // Send request
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("ERROR: Failed to send request");
        close(sock);
        return 1;
    }

    // Receive response
    char response[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(sock, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';
        printf("%s", response);
    }

    if (bytes_received < 0) {
        perror("ERROR: Failed to receive response");
    }

    // Close socket
    close(sock);
    return 0;
}
