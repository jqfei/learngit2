#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

//dev2
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <port> <duration_seconds>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];
    memset(buffer, 'A', 1024); // Fill with data

    long long total_bytes = 0;
    time_t start, end;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    // Connect
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    start = time(NULL);
    end = start + duration;

    // Send data
    int send_count = 0;
    while (time(NULL) < end) {
        int sent = send(sock, buffer, 1024, 0);
        if (sent < 0) {
            perror("send");
            break;
        }
        total_bytes += sent;
        send_count++;
        if (send_count % 1000 == 0) {
            printf("Sent %d packets, total %lld bytes\n", send_count, total_bytes);
        }
    }

    double time_taken = difftime(time(NULL), start);
    double bandwidth = (total_bytes * 8.0) / (time_taken * 1000000.0); // Mbps

    printf("Total bytes sent: %lld\n", total_bytes);
    printf("Time taken: %.2f seconds\n", time_taken);
    printf("Bandwidth: %.2f Mbps\n", bandwidth);

    close(sock);
    return 0;
}