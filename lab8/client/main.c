#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    const int server_port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    while (1) {
        printf("> ");
        fgets(buf, BUF_SIZE, stdin);
        buf[strcspn(buf, "\n")] = '\0'; 

        if (send(sock, buf, strlen(buf), 0) == -1) {
            perror("Error sending message to server");
            exit(EXIT_FAILURE);
        }

        memset(buf, 0, BUF_SIZE);
        int total_bytes_received = 0;
        int bytes_received;
        while ((bytes_received = read(sock, buf + total_bytes_received, BUF_SIZE - total_bytes_received)) > 0) {
            total_bytes_received += bytes_received;
            if (buf[total_bytes_received - 1] == '\t') {
                break;
            }
        }
        if (bytes_received == -1) {
            perror("Error receiving message from server");
            exit(EXIT_FAILURE);
        } else if (bytes_received == 0) {
            printf("Connection closed by server\n");
            break;
        } else {
            printf("%s\n", buf? buf: "NULL");
        }
    }

    close(sock);
    return 0;
}