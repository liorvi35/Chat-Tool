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

void chat_client(char *ipv4, char *port)
{
    int client_sock = 0;

    socklen_t addr_size = 0;

    ssize_t bytes_recv = 0, bytes_sent = 0;

    struct sockaddr_in server_addr = {0};
    memset(&server_addr, 0, sizeof(server_addr));

    struct pollfd fds[2] = {0};
    memset(fds, 0, sizeof(fds));

    char buffer[BUFSIZ] = {0};
    memset(buffer, 0, sizeof(buffer));

    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock <= 0)
    {
        perror("socket() failed");
        exit(errno);
    }

    addr_size = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipv4);
    server_addr.sin_port = htons(atoi(port));

    if (connect(client_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
    {
        perror("connect() failed");
        close(client_sock);
        exit(errno);
    }
    fprintf(stdout, "Connected to chat!\n");

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
                    exit(errno);
                }
                else if (bytes_sent == 0)
                {
                    fprintf(stderr, "Server has disconnected. Closing chat...\n");
                    close(client_sock);
                    exit(errno);
                }
            }

            if (fds[1].revents & POLLIN)
            {
                memset(buffer, 0, BUFSIZ);

                bytes_recv = recv(client_sock, buffer, BUFSIZ, 0);
                if (bytes_recv < 0)
                {
                    perror("recv() failed");
                    close(client_sock);
                    exit(errno);
                }
                else if (bytes_recv == 0)
                {
                    fprintf(stderr, "Server has disconnected. Closing chat...\n");
                    close(client_sock);
                    exit(errno);
                }
                else
                {
                    fprintf(stdout, "Server >> %s\n", buffer);
                }
            }
        }
    }
}