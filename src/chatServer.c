#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/poll.h>
#include <unistd.h>

#define SO_REUSEPORT 15

void chat_server(char *port)
{
    int optval = 0, server_sock = 0, client_sock = 0;

    socklen_t addr_size = 0;

    ssize_t bytes_recv = 0, bytes_sent = 0;

    struct sockaddr_in server_addr = {0}, client_addr = {0};
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    struct pollfd fds[2] = {0};
    memset(fds, 0, sizeof(fds));

    char buffer[BUFSIZ] = {0};
    memset(buffer, 0, sizeof(buffer));

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock <= 0)
    {
        perror("socket() failed");
        exit(errno);
    }

    optval = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt() failed");
        close(server_sock);
        exit(errno);
    }

    addr_size = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port));

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        close(server_sock);
        exit(errno);
    }

    if (listen(server_sock, 1) < 0)
    {
        perror("listen() failed");
        close(server_sock);
        exit(errno);
    }

    fprintf(stdout, "Waiting for client connection...\n");

ACCEPT:
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (client_sock <= 0)
    {
        perror("listen() failed");
        close(server_sock);
        exit(errno);
    }
    fprintf(stdout, "Client has been connected!\n");

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = client_sock;
    fds[1].events = POLLIN;

    while (true)
    {
        if (poll(fds, 2, -1) <= 0)
        {
            perror("poll() failed");
            close(client_sock);
            close(server_sock);
            exit(errno);
        }
        else
        {
            if (fds[0].revents & POLLIN)
            {
                memset(buffer, 0, BUFSIZ);

                fgets(buffer, BUFSIZ, stdin);
                buffer[strlen(buffer) - 1] = '\0';

                fprintf(stdout, "%sMe >> %s\n", "\033[1A\033[2K", buffer);

                bytes_sent = send(client_sock, buffer, strlen(buffer) + 1, 0);
                if (bytes_sent < 0)
                {
                    perror("send() failed");
                    close(client_sock);
                    close(server_sock);
                    exit(errno);
                }
                else if (bytes_sent == 0)
                {
                    fprintf(stderr, "Client has disconnected. Waiting for new connection...\n");
                    goto ACCEPT;
                }
            }

            if (fds[1].revents & POLLIN)
            {
                memset(buffer, 0, BUFSIZ);

                bytes_recv = recv(client_sock, buffer, BUFSIZ, 0);
                if (bytes_recv < 0)
                {
                    perror("recv() failed");
                    exit(errno);
                }
                else if (bytes_recv == 0)
                {
                    fprintf(stderr, "Client has disconnected. Waiting for new connection...\n");
                    goto ACCEPT;
                }
                else
                {
                    fprintf(stdout, "Client >> %s\n", buffer);
                }
            }
        }
    }
}