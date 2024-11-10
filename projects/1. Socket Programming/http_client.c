#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void print_usage(const char *prog_name) {
    printf("Usage: %s [-p] server_url port_number\n", prog_name);
}

long calculate_rtt(struct timeval start, struct timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    return (seconds * 1000) + (microseconds / 1000);  // return RTT in milliseconds
}

int main(int argc, char *argv[]) {
    int sock, port, rtt_flag = 0;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char request[BUFFER_SIZE], response[BUFFER_SIZE];
    struct timeval start, end;
    long rtt = 0;

    // Argument parsing
    if (argc < 3 || argc > 4) {
        print_usage(argv[0]);
        return 1;
    }

    if (argc == 4 && strcmp(argv[1], "-p") == 0) {
        rtt_flag = 1;
        argv++;  // shift arguments to treat server_url as argv[1] and port as argv[2]
    }

    char *server_url = argv[1];
    port = atoi(argv[2]);

    // Split server_url into hostname and path
    char *path = strchr(server_url, '/');
    if (path) {
        *path = '\0';  // Null-terminate the hostname part
        path++;        // Move to the start of the path
    } else {
        path = "/";  // Default to root if no path is provided
    }

    // Resolve server address
    server = gethostbyname(server_url);
    if (server == NULL) {
        fprintf(stderr, "ERROR: No such host\n");
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR: Unable to open socket");
        return 1;
    }

    // Prepare server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Measure RTT if `-p` option is enabled
    if (rtt_flag) {
        gettimeofday(&start, NULL);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Connection failed");
        close(sock);
        return 1;
    }

    if (rtt_flag) {
        gettimeofday(&end, NULL);
        rtt = calculate_rtt(start, end);
        printf("RTT: %ld ms\n", rtt);
    }

    // Formulate HTTP GET request
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, server_url);

    // Send the HTTP GET request
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("ERROR: Failed to send request");
        close(sock);
        return 1;
    }

    // Receive and display the response
    ssize_t bytes_received;
    while ((bytes_received = recv(sock, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';  // Null-terminate the response
        printf("%s", response);           // Print the response to stdout
    }

    if (bytes_received < 0) {
        perror("ERROR: Failed to receive response");
    }

    printf("\n");
    // Clean up
    close(sock);
    return 0;
}
