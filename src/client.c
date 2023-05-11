#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#include "chat.h"

int main(int argc, char *argv[])
{
    int client_sock = 0;
    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock <= 0)
    {
        perror("socket() failed");
        exit(errno);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in server_addr = {0};
    memset(&server_addr, 0, addr_size);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server_addr.sin_port = htons(atoi(PORT));

    if (connect(client_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
    {
        perror("socket() failed");
        close(client_sock);
        exit(errno);
    }

    struct pollfd fds[2] = {0};
    memset(&fds, 0, sizeof(fds));

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = client_sock;
    fds[1].events = POLLIN;

    int i = 0, poll_result = 0;
    size_t bytes_recv = 0, bytes_sent = 0;
    char buffer[BUFSIZ] = {0};
    memset(buffer, 0, sizeof(buffer));

    while (1)
    {
        poll_result = poll(fds, 2, -1);
        if (poll_result <= 0)
        {
            perror("poll() failed");
            close(client_sock);
            exit(errno);
        }

        if (fds[0].revents & POLLIN)
        {
            fgets(buffer, BUFSIZ, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            bytes_sent = send(client_sock, buffer, strlen(buffer), 0);
            if (bytes_sent < 0)
            {
                perror("error in sending message");
                exit(0);
            }
            memset(buffer, 0, sizeof(buffer));
        }

        // Handle server messages
        if (fds[1].revents & POLLIN)
        {
            bytes_recv = recv(client_sock, buffer, BUFSIZ, 0);
            if (bytes_recv <= 0)
            {
                printf("error in receiving message");
                exit(0);
            }
            else
            {
                printf("Message received from server: %s\n", buffer);
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }
}