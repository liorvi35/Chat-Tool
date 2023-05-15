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
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>

#define MB_100 1024 * 1024 * 100

unsigned long hash(const char *);

unsigned int hash_file(FILE *);

void performance_client(char *ip, char *port, char *type, char *param)
{
    int client_sock = 0;

    socklen_t addr_size = 0, addr_size6 = 0, addr_size_unix = 0;

    ssize_t bytes_to_send = 0, bytes_sent = 0;

    struct sockaddr_in server_addr = {0};
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    struct sockaddr_in6 server_addr6 = {0};
    memset(&server_addr, 0, sizeof(struct sockaddr_in6));

    char *buffer = NULL, buff[BUFSIZ] = {0};
    memset(buff, 0, sizeof(buff));

    struct stat stat = {0};
    memset(&stat, 0, sizeof(struct stat));

    void *ptr = NULL;

    struct sockaddr_un unix_addr = {0};
    memset(&unix_addr, 0, sizeof(struct sockaddr_un));

    FILE *file = NULL;

    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock <= 0)
    {
        perror("socket() failed");
    }

    addr_size = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(port));

    if (connect(client_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
    {
        perror("connect() failed");
        close(client_sock);
        exit(errno);
    }

    if (send(client_sock, type, strlen(type) + 1, 0) < 0)
    {
        perror("send() failed");
        close(client_sock);
        exit(errno);
    }

    sleep(1);

    if (send(client_sock, param, strlen(param) + 1, 0) < 0)
    {
        perror("send() failed");
        close(client_sock);
        exit(errno);
    }

    fprintf(stdout, "Sent type and param to server.\n");

    if (shutdown(client_sock, SHUT_RDWR) < 0)
    {
        perror("send() failed");
        close(client_sock);
        exit(errno);
    }
    close(client_sock);

    buffer = (char *)calloc(MB_100, sizeof(char));
    if (buffer == NULL)
    {
        perror("calloc() failed");
        close(client_sock);
        exit(errno);
    }
    memset(buffer, 'A', MB_100);

    if (strcmp(type, "mmap") && strcmp(type, "pipe"))
    {
        fprintf(stdout, "Checksum=%ld\n", hash(buffer));
    }

    if (!strcmp(type, "ipv4") && !strcmp(param, "tcp"))
    {
        client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        addr_size = sizeof(struct sockaddr_in);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip);
        server_addr.sin_port = htons(atoi(port));

        if (connect(client_sock, (struct sockaddr *)&server_addr, addr_size) < 0)
        {
            perror("connect() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        if (send(client_sock, buffer, strlen(buffer) + 1, 0) < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }
        close(client_sock);
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "tcp"))
    {
        client_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        addr_size6 = sizeof(struct sockaddr_in6);
        server_addr6.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, ip, &(server_addr6.sin6_addr)) < 0)
        {
            perror("inet_pton() failed");
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }
        server_addr6.sin6_port = htons(atoi(port));

        if (connect(client_sock, (struct sockaddr *)&server_addr6, addr_size6) < 0)
        {
            perror("connect() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        if (send(client_sock, buffer, strlen(buffer) + 1, 0) < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }
        close(client_sock);
    }
    else if (!strcmp(type, "ipv4") && !strcmp(param, "udp"))
    {
        client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        addr_size = sizeof(struct sockaddr_in);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip);
        server_addr.sin_port = htons(atoi(port));

        bytes_to_send = strlen(buffer) + 1;

        while (bytes_to_send > 0)
        {
            bytes_sent = sendto(client_sock, buffer, (bytes_to_send < BUFSIZ ? bytes_to_send : BUFSIZ), 0, (struct sockaddr *)&server_addr, addr_size);
            if (bytes_sent < 0)
            {
                perror("send() failed");
                close(client_sock);
                free(buffer);
                buffer = NULL;
                exit(errno);
            }
            bytes_to_send -= bytes_sent;
        }

        sleep(1);

        bytes_sent = sendto(client_sock, "END", strlen("END") + 1, 0, (struct sockaddr *)&server_addr, addr_size);
        if (bytes_sent < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        close(client_sock);
    }
    else if (!strcmp(type, "ipv6") && !strcmp(param, "udp"))
    {
        client_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        addr_size6 = sizeof(struct sockaddr_in6);
        server_addr6.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, ip, &server_addr6.sin6_addr) < 0)
        {
            perror("inet_pton() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }
        server_addr6.sin6_port = htons(atoi(port));

        bytes_to_send = strlen(buffer) + 1;

        while (bytes_to_send > 0)
        {
            bytes_sent = sendto(client_sock, buffer, (bytes_to_send < BUFSIZ ? bytes_to_send : BUFSIZ), 0, (struct sockaddr *)&server_addr6, addr_size6);
            if (bytes_sent < 0)
            {
                perror("send() failed");
                close(client_sock);
                free(buffer);
                buffer = NULL;
                exit(errno);
            }
            bytes_to_send -= bytes_sent;
        }

        sleep(1);

        bytes_sent = 0;
        bytes_sent = sendto(client_sock, "END", strlen("END") + 1, 0, (struct sockaddr *)&server_addr6, addr_size6);
        if (bytes_sent < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        close(client_sock);
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "stream"))
    {
        client_sock = socket(AF_UNIX, SOCK_STREAM, PF_UNIX);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        addr_size_unix = sizeof(struct sockaddr_un);
        unix_addr.sun_family = AF_UNIX;

        strncpy(unix_addr.sun_path, "/tmp/uds_socket", addr_size_unix - 1);

        if (connect(client_sock, (struct sockaddr *)&unix_addr, addr_size_unix) < 0)
        {
            perror("connect() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        if (send(client_sock, buffer, strlen(buffer) + 1, 0) < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        if (shutdown(client_sock, SHUT_RDWR) < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }
        close(client_sock);
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "dgram"))
    {
        client_sock = socket(AF_UNIX, SOCK_DGRAM, PF_UNIX);
        if (client_sock <= 0)
        {
            perror("socket() failed");
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        addr_size_unix = sizeof(struct sockaddr_un);
        unix_addr.sun_family = AF_UNIX;

        strncpy(unix_addr.sun_path, "/tmp/uds_socket", addr_size_unix - 1);

        bytes_to_send = strlen(buffer) + 1;

        while (bytes_to_send > 0)
        {
            bytes_sent = sendto(client_sock, buffer, (bytes_to_send < BUFSIZ ? bytes_to_send : BUFSIZ), 0, (struct sockaddr *)&unix_addr, addr_size_unix);
            if (bytes_sent < 0)
            {
                perror("send() failed");
                close(client_sock);
                free(buffer);
                buffer = NULL;
                exit(errno);
            }
            bytes_to_send -= bytes_sent;
        }

        sleep(1);

        bytes_sent = 0;
        bytes_sent = sendto(client_sock, "END", strlen("END") + 1, 0, (struct sockaddr *)&unix_addr, addr_size_unix);
        if (bytes_sent < 0)
        {
            perror("send() failed");
            close(client_sock);
            free(buffer);
            buffer = NULL;
            exit(errno);
        }

        close(client_sock);
    }
    else if (!strcmp(type, "pipe"))
    {
        file = fopen(param, "r");
        if (file == NULL)
        {
            perror("fopen() failed");
            exit(errno);
        }

        client_sock = open("/tmp/pipe", O_WRONLY);
        if (client_sock <= 0)
        {
            perror("open() failed");
            fclose(file);
            file = NULL;
            exit(errno);
        }

        bytes_sent = fread(buff, sizeof(char), sizeof(buff), file);
        while (bytes_sent > 0)
        {
            if (write(client_sock, buff, bytes_sent) != bytes_sent)
            {
                perror("wirte() failed");
                fclose(file);
                file = NULL;
                close(client_sock);
                exit(errno);
            }
            bytes_sent = fread(buff, sizeof(char), sizeof(buff), file);
        }

        fclose(file);
        file = NULL;
        close(client_sock);
    }
    else if (!strcmp(type, "mmap"))
    {
        client_sock = open(param, O_RDWR, S_IRUSR | S_IWUSR);
        if (client_sock <= 0)
        {
            perror("open() failed");
            exit(errno);
        }

        ptr = (char *)mmap(NULL, MB_100, PROT_READ | PROT_WRITE, MAP_SHARED, client_sock, 0);
        if (ptr == MAP_FAILED)
        {
            perror("mmap() failed");
            close(client_sock);
            exit(errno);
        }

        file = fopen(param, "rb");
        if (file == NULL)
        {
            perror("fopen() failed");
            close(client_sock);
            exit(errno);
        }

        while (fread(buff, 1, BUFSIZ, file) > 0)
        {
            memcpy(ptr, buff, BUFSIZ);
        }

        if (munmap(ptr, MB_100) < 0)
        {
            perror("munmap() failed");
            close(client_sock);
            exit(errno);
        }

        close(client_sock);
        fclose(file);
    }
    fprintf(stdout, "100MB has been sent in %s_%s.\n", type, param);
}
