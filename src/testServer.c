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

void performance_server(char *port, bool qFlag)
{
    int server_sock = 0;
    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock <= 0)
    {
        perror("socket() failed");
    }

    int optval = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt() failed");
        close(server_sock);
        exit(errno);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in server_addr = {0}, client_addr = {0};
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
    
    int client_sock = 0;
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (client_sock <= 0)
    {
        perror("listen() failed");
        close(server_sock);
        exit(errno);
    }

    char type[BUFSIZ / 1024] = {0}, param[BUFSIZ / 1024] = {0};
    ssize_t bytes_recv = 0;
    bytes_recv = recv(client_sock, type, sizeof(type), 0);
    if(bytes_recv < 0)
    {
        perror("recv() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }

    
}