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
#define MB_100 1024 * 1024 * 100

void performance_server(char *port, bool qFlag)
{
    int server_sock = 0, client_sock = 0, optval = 0;

    socklen_t addr_size = 0, addr_size6 = 0;
    struct sockaddr_in server_addr = {0}, client_addr = {0};
    memset(&server_sock, 0, sizeof(server_addr));
    memset(&client_sock, 0, sizeof(client_sock));

    char type[BUFSIZ / 1024] = {0}, param[BUFSIZ / 1024] = {0}, buffer[BUFSIZ] = {0};
    memset(type, 0, sizeof(type));
    memset(type, 0, sizeof(param));
    memset(type, 0, sizeof(buffer));

    ssize_t bytes_recv = 0;

    struct timeval start = {0}, end = {0}, diff = {0};
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(&diff, 0, sizeof(diff));

    struct sockaddr_in6 server_addr6, client_addr6 = {0};
    memset(&server_sock, 0, sizeof(server_addr));
    memset(&client_sock, 0, sizeof(client_sock));

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

    if (bind(server_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
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
    if (!qFlag)
    {
        fprintf(stdout, "Listening for connections...\n");
    }

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (client_sock <= 0)
    {
        perror("listen() failed");
        close(server_sock);
        exit(errno);
    }

    bytes_recv = recv(client_sock, type, sizeof(type), 0);
    if (bytes_recv < 0)
    {
        perror("recv() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }

    bytes_recv = recv(client_sock, param, sizeof(param), 0);
    if (bytes_recv < 0)
    {
        perror("recv() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }

    if (shutdown(client_sock, SHUT_RDWR) < 0)
    {
        perror("shutdown() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }
    close(client_sock);
    close(server_sock);

    if (!qFlag)
    {
        fprintf(stdout, "Received: (%s, %s).\n", param, type);
    }

    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {
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

        if (bind(server_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
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

        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        if (client_sock <= 0)
        {
            perror("listen() failed");
            close(server_sock);
            exit(errno);
        }

        gettimeofday(&start, NULL);
        bytes_recv = recv(client_sock, buffer, sizeof(buffer), 0);
        while (bytes_recv > 0)
        {
            bytes_recv = recv(client_sock, buffer, sizeof(buffer), 0);
        }
        if (bytes_recv < 0)
        {
            perror("recv() failed");
            close(client_sock);
            close(server_sock);
            exit(errno);
        }
        gettimeofday(&end, NULL);

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("shutdown() failed");
            close(client_sock);
            close(server_sock);
            exit(errno);
        }
        close(client_sock);
        close(server_sock);

        timersub(&diff, &end, &start);
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "tcp"))
    {
        server_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
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

        addr_size6 = sizeof(struct sockaddr_in6);
        server_addr6.sin6_family = AF_INET6;
        server_addr6.sin6_addr = in6addr_any;
        server_addr6.sin6_port = htons(atoi(port));

        if (bind(server_sock, (struct sockaddr *)&server_addr6, addr_size6) < 0)
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

        client_sock = accept(server_sock, (struct sockaddr *)&client_addr6, &addr_size6);
        if (client_sock <= 0)
        {
            perror("listen() failed");
            close(server_sock);
            exit(errno);
        }

        gettimeofday(&start, NULL);
        bytes_recv = recv(client_sock, buffer, sizeof(buffer), 0);
        while (bytes_recv > 0)
        {
            bytes_recv = recv(client_sock, buffer, sizeof(buffer), 0);
        }
        if (bytes_recv < 0)
        {
            perror("recv() failed");
            close(client_sock);
            close(server_sock);
            exit(errno);
        }
        gettimeofday(&end, NULL);

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("shutdown() failed");
            close(client_sock);
            close(server_sock);
            exit(errno);
        }
        close(client_sock);
        close(server_sock);

        timersub(&diff, &end, &start);
    }
    else if (!strcmp(type, "ipv4") && !strcmp(param, "udp"))
    {
        server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

        if (bind(server_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
        {
            perror("bind() failed");
            close(server_sock);
            exit(errno);
        }

        gettimeofday(&start, NULL);
        bytes_recv = recvfrom(server_sock, buffer, sizeof(buffer), 0, NULL, NULL);
        while (strcmp(buffer, "END"))
        {
            memset(buffer, 0, sizeof(buffer));
            bytes_recv = recvfrom(server_sock, buffer, sizeof(buffer), 0, NULL, NULL);
        }
        if (bytes_recv < 0)
        {
            perror("recv() failed");
            close(client_sock);
            close(server_sock);
            exit(errno);
        }
        gettimeofday(&end, NULL);

        close(server_sock);

        timersub(&diff, &end, &start);
        diff.tv_sec -= 1;
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "udp"))
    {
        server_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
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

        addr_size6 = sizeof(struct sockaddr_in6);
        server_addr6.sin6_family = AF_INET6;
        server_addr6.sin6_addr = in6addr_any;
        server_addr6.sin6_port = htons(atoi(port));

        gettimeofday(&start, NULL);
        bytes_recv = recvfrom(server_sock, buffer, sizeof(buffer), 0, NULL, NULL);
        while (bytes_recv > 0)
        {
            bytes_recv = recvfrom(server_sock, buffer, sizeof(buffer), 0, NULL, NULL);
        }
        if (bytes_recv < 0)
        {
            perror("recv() failed");
            close(server_sock);
            exit(errno);
        }
        gettimeofday(&end, NULL);

        close(server_sock);

        timersub(&diff, &end, &start);
    }
    fprintf(stdout, "%s_%s,%ld\n", type, param, (diff.tv_sec * 1000 + diff.tv_usec / 1000));
}