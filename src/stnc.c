/**
 * @brief this file containts implementations for chat
 * @since 11/05/2023
 * @authors Lior Vinman & Yoad Tamar
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <poll.h>
#include <sys/socket.h>

#include "stnc.h"

int main(int argc, char *argv[])
{
    srand(time(NULL));

    signal(SIGINT, exit_handle);

    if (argc < 2 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-s")))
    {
        fprintf(stderr, "Usage: ./stnc [OPTIONS] [OPTIONAL]...\n\nOpions:\n\t-c <IP> <PORT> Use the chat as a client.\n\t-s <PORT> Use the chat as a server.\n\nOptional:\n\t-p <type> <param> try chat's performance test, for client only.\n\t-p -q performace test for server.\n\nDescription:\n\tSTNS is a CLI chat tool.\n\nExamples:\n\tStart the chat as client:\n\t./stns -c 127.0.0.1 8080\n\n\tStart the chat as server:\n\t./stnt -s 8080\n");
        exit(EXIT_FAILURE);
    }
    if (!strcmp(argv[1], "-c"))
    {
        if (argc != 4 && argc != 7)
        {
            fprintf(stderr, "Usage: ./stnc -c <IP> <PORT> -p <type> <param>\n");
            exit(EXIT_FAILURE);
        }
        else if (argc == 4)
        {
            start_client(argv[2], argv[3]);
        }
        else
        {
            client_performace(argv[2], argv[3], argv[5], argv[6]);
        }
    }
    else if (!strcmp(argv[1], "-s"))
    {
        if (argc != 3 && argc != 5)
        {
            fprintf(stderr, "Usage: ./stnc -s <PORT>\n");
            exit(EXIT_FAILURE);
        }
        else if (argc == 3)
        {
            start_server(argv[2]);
        }
        else
        {
            server_performace(argv[2]);
        }
    }
}

void exit_handle(int sid)
{
    if (server_sock != 0)
    {
        close(server_sock);
    }

    if (client_sock != 0)
    {
        close(client_sock);
    }

    if (chunk != NULL)
    {
        free(chunk);
        chunk = NULL;
    }

    exit(sid == -1 ? EXIT_FAILURE : EXIT_SUCCESS);
}

void start_server(char *port)
{
    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock <= 0)
    {
        perror("socket() failed");
        exit_handle(-1);
    }

    int optval = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt() failed");
        exit_handle(-1);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in server_addr = {0}, client_addr = {0};
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port));

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit_handle(-1);
    }

    if (listen(server_sock, NUM_OF_CLIENTS) < 0)
    {
        perror("listen() failed");
        exit_handle(-1);
    }

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (client_sock <= 0)
    {
        perror("listen() failed");
        exit_handle(-1);
    }

    struct pollfd fds[MAX_EVENTS] = {0};
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = client_sock;
    fds[1].events = POLLIN;

    size_t bytes_recv = 0, bytes_sent = 0;
    time_t current_time = 0;
    char buffer[BUFSIZ] = {0}, time_format[TIME_LENGTH] = {0};
    memset(buffer, 0, sizeof(buffer));
    memset(time_format, 0, sizeof(time_format));

    while (1)
    {
        if (poll(fds, MAX_EVENTS, -1) <= 0)
        {
            perror("poll() failed");
            exit_handle(-1);
        }

        if (fds[0].revents & POLLIN)
        {
            memset(time_format, 0, sizeof(time_format));
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, BUFSIZ, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            current_time = time(NULL);
            strftime(time_format, sizeof(time_format), "%d.%m.%Y - %H:%M:%S", localtime(&current_time));
            fprintf(stdout, "\033[1A\033[2K");
            fprintf(stdout, "[%s] Me >> %s\n", time_format, buffer);

            bytes_sent = send(client_sock, buffer, strlen(buffer) + 1, 0);
            if (bytes_sent < 0)
            {
                perror("send() failed");
                exit_handle(-1);
            }
        }

        if (fds[1].revents & POLLIN)
        {
            memset(time_format, 0, sizeof(time_format));
            memset(buffer, 0, sizeof(buffer));

            bytes_recv = recv(client_sock, buffer, BUFSIZ, 0);
            if (bytes_recv < 0)
            {
                perror("recv() failed");
                exit_handle(-1);
            }
            else if (bytes_recv == 0)
            {
                fprintf(stdout, "Client has disconnected. Closing chat.\n");
                exit_handle(-2);
            }
            else
            {
                current_time = time(NULL);
                strftime(time_format, sizeof(time_format), "%d.%m.%Y - %H:%M:%S", localtime(&current_time));
                fprintf(stdout, "[%s] Server >> %s\n", time_format, buffer);
            }
        }
    }
}

void start_client(char *ip, char *port)
{
    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock <= 0)
    {
        perror("socket() failed");
        exit_handle(-1);
    }

    struct sockaddr_in server_addr = {0};
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(atoi(port));

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect() failed");
        exit_handle(-1);
    }

    struct pollfd fds[2] = {0};
    memset(&fds, 0, sizeof(fds));

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = client_sock;
    fds[1].events = POLLIN;

    size_t bytes_recv = 0, bytes_sent = 0;
    time_t current_time = 0;
    char buffer[BUFSIZ] = {0}, time_format[TIME_LENGTH] = {0};
    memset(buffer, 0, sizeof(buffer));
    memset(time_format, 0, sizeof(time_format));

    while (1)
    {
        if (poll(fds, MAX_EVENTS, -1) <= 0)
        {
            perror("poll() failed");
            exit_handle(-1);
        }
        if (fds[0].revents & POLLIN)
        {
            memset(time_format, 0, sizeof(time_format));
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, BUFSIZ, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            current_time = time(NULL);
            strftime(time_format, sizeof(time_format), "%d.%m.%Y - %H:%M:%S", localtime(&current_time));
            fprintf(stdout, "\033[1A\033[2K");
            fprintf(stdout, "[%s] Me >> %s\n", time_format, buffer);

            bytes_sent = send(client_sock, buffer, strlen(buffer) + 1, 0);
            if (bytes_sent <= 0)
            {
                perror("send() failed");
                exit_handle(-1);
            }
        }

        if (fds[1].revents & POLLIN)
        {
            memset(time_format, 0, sizeof(time_format));
            memset(buffer, 0, sizeof(buffer));

            bytes_recv = recv(client_sock, buffer, BUFSIZ, 0);
            if (bytes_recv < 0)
            {
                perror("recv() failed");
                exit_handle(-1);
            }
            if (bytes_recv == 0)
            {
                fprintf(stdout, "Server has disconnected. Closing chat.\n");
                exit_handle(-1);
            }
            else
            {
                current_time = time(NULL);
                strftime(time_format, sizeof(time_format), "%d.%m.%Y - %H:%M:%S", localtime(&current_time));
                fprintf(stdout, "[%s] Server >> %s\n", time_format, buffer);
            }
        }
    }
}

void client_performace(char *ip, char *port, char *type, char *param)
{
    chunk = (char *)calloc(MB_100, sizeof(char));
    if (chunk == NULL)
    {
        perror("calloc() failed");
        exit_handle(-1);
    }
    generate_data(chunk, MB_100);

    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock <= 0)
    {
        perror("socket() failed");
        exit_handle(-1);
    }

    struct sockaddr_in server_addr = {0};
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(atoi(port));

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect() failed");
        exit_handle(-1);
    }

    if (send(client_sock, type, strlen(type) + 1, 0) <= 0)
    {
        perror("send() failed");
        exit_handle(-1);
    }
    if (send(client_sock, param, strlen(param) + 1, 0) <= 0)
    {
        perror("send() failed");
        exit_handle(-1);
    }

    if (shutdown(client_sock, SHUT_RDWR) < 0)
    {
        perror("shutdown() failed");
        exit_handle(-1);
    }
    close(client_sock);

    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {

        client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            exit_handle(-1);
        }

        struct sockaddr_in server_addr = {0};
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip);
        server_addr.sin_port = htons(atoi(port));

        if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("connect() failed");
            exit_handle(-1);
        }

        if (send(client_sock, chunk, strlen(chunk) + 1, 0) <= 0)
        {
            perror("send() failed");
            exit_handle(-1);
        }

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("shutdown() failed");
            exit_handle(-1);
        }
        close(client_sock);
    }

    else if (!strcmp(type, "ipv4") && !strcmp(param, "udp"))
    {
        client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            exit_handle(-1);
        }

        socklen_t addr_size = sizeof(struct sockaddr_in);
        struct sockaddr_in server_addr = {0};
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip);
        server_addr.sin_port = htons(atoi(port));

        if (sendto(client_sock, chunk, strlen(chunk) + 1, 0, (struct sockaddr *)&server_addr, addr_size) <= 0)
        {
            perror("send() failed");
            exit_handle(-1);
        }

        close(client_sock);
    }

    else if (!strcmp(type, "ipv6") && !strcmp(param, "tcp"))
    {
        client_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            exit_handle(-1);
        }

        struct sockaddr_in6 server_addr = {0};
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_port = htons(atoi(port));
        if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0)
        {
            perror("inet_pton() failed");
            exit_handle(-1);
        }

        if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("connect() failed");
            exit_handle(-1);
        }

        if (send(client_sock, chunk, strlen(chunk) + 1, 0) <= 0)
        {
            perror("send() failed");
            exit_handle(-1);
        }

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("shutdown() failed");
            exit_handle(-1);
        }
        close(client_sock);
    }

    else if (!strcmp(type, "ipv6") && !strcmp(param, "udp"))
    {
        client_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            exit_handle(-1);
        }

        socklen_t addr_size = sizeof(struct sockaddr_in);
        struct sockaddr_in6 server_addr = {0};
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_port = htons(atoi(port));
        if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0)
        {
            perror("inet_pton() failed");
            exit_handle(-1);
        }

        if (sendto(client_sock, chunk, strlen(chunk) + 1, 0, (struct sockaddr *)&server_addr, addr_size) <= 0)
        {
            perror("send() failed");
            exit_handle(-1);
        }

        close(client_sock);
    }

    else if (!strcmp(type, "uds") && !strcmp(param, "stream"))
    {
    }

    else if (!strcmp(type, "uds") && !strcmp(param, "dgram"))
    {
    }

    else if (!strcmp(type, "mmap") && !strcmp(param, "filename"))
    {
    }

    else if (!strcmp(type, "pipe") && !strcmp(param, "filename"))
    {
    }
}

void server_performace(char *port)
{
    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock <= 0)
    {
        perror("socket() failed");
        exit_handle(-1);
    }

    int optval = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt() failed");
        exit_handle(-1);
    }

    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in server_addr = {0}, client_addr = {0};
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port));

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit_handle(-1);
    }

    if (listen(server_sock, NUM_OF_CLIENTS) < 0)
    {
        perror("listen() failed");
        exit_handle(-1);
    }

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (client_sock <= 0)
    {
        perror("listen() failed");
        exit_handle(-1);
    }

    char type[BUFSIZ / 128] = {0}, param[BUFSIZ / 128] = {0};
    memset(type, 0, sizeof(type));
    memset(param, 0, sizeof(param));

    if (recv(client_sock, type, BUFSIZ, 0) < 0)
    {
        perror("recv() failed");
        exit_handle(-1);
    }
    if (recv(client_sock, param, BUFSIZ, 0) < 0)
    {
        perror("recv() failed");
        exit_handle(-1);
    }

    close(client_sock);
    close(server_sock);

    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {
        server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_sock <= 0)
        {
            perror("socket() failed");
            exit_handle(-1);
        }

        int optval = 1;
        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) < 0)
        {
            perror("setsockopt() failed");
            exit_handle(-1);
        }

        socklen_t addr_size = sizeof(struct sockaddr_in);
        struct sockaddr_in server_addr = {0}, client_addr = {0};
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(atoi(port));

        if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("bind() failed");
            exit_handle(-1);
        }

        if (listen(server_sock, NUM_OF_CLIENTS) < 0)
        {
            perror("listen() failed");
            exit_handle(-1);
        }

        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        if (client_sock <= 0)
        {
            perror("listen() failed");
            exit_handle(-1);
        }

        if (recv(server_sock, NULL, MB_100, 0) <= 0)
        {
            perror("recv() failed");
            exit_handle(-1);
        }

        close(client_sock);
        close(server_sock);
    }
}

void generate_data(char *buffer, size_t size)
{
    size_t i = 0;
    for (i = 0; i < size; i++)
    {
        buffer[i] = rand() % 256;
    }
}
