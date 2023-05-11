/**
 * @brief this file containts implementations for chat
 * @since 11/05/2023
 * @authors Lior Vinman & Yoad Tamar
*/

#include <stdio.h>
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
    signal(SIGINT, exit_handle);

    if (argc < 2 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-s")))
    {
        fprintf(stderr, "Usage: ./stnc [OPTIONS]...\n\nOpions:\n\t-c <IP> <PORT> Use the chat as a client.\n\t-s <PORT> Use the chat as a server.\n\nDescription:\n\tSTNS is a CLI chat tool.\n\nExamples:\n\tStart the chat as client:\n\t./stns -c 127.0.0.1 8080\n\n\tStart the chat as server:\n\t./stnt -s 8080\n");
        exit(EXIT_FAILURE);
    }
    if (!strcmp(argv[1], "-c"))
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage: ./stnc -c <IP> <PORT>\n");
            exit(EXIT_FAILURE);
        }
        start_client(argv[2], argv[3]);
    }
    else if (!strcmp(argv[1], "-s"))
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: ./stnc -s <PORT>\n");
            exit(EXIT_FAILURE);
        }
        start_server(argv[2]);
    }
}

void exit_handle(int sid)
{
    if (server_sock != 0)
        close(server_sock);

    if (client_sock != 0)
        close(client_sock);

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
