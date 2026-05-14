#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    long long total_bytes = 0;
    time_t start, end;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    // Accept
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted\n");

    start = time(NULL);

    // Receive data
    int valread;
    while ((valread = read(new_socket, buffer, 1024)) > 0) {
        total_bytes += valread;
    }

    end = time(NULL);

    double time_taken = difftime(end, start);
    double bandwidth = (total_bytes * 8.0) / (time_taken * 1000000.0); // Mbps

    printf("Total bytes received: %lld\n", total_bytes);
    printf("Time taken: %.2f seconds\n", time_taken);
    printf("Bandwidth: %.2f Mbps\n", bandwidth);

    close(new_socket);
    close(server_fd);
    return 0;
}