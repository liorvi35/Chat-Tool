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
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>

#define SO_REUSEPORT 15
#define MB_100 1024 * 1024 * 100

unsigned int hash_xor(const char *buffer, size_t len) {
  unsigned int hash = 0;
  for (size_t i = 0; i < len; i++) {
    hash ^= buffer[i];
  }
  return hash;
}

unsigned int hash_file(FILE *fp)
{
    unsigned int hash = 5381;
    char ch;
    while (fread(&ch, 1, 1, fp) == 1)
    {
        hash = (hash * 33) + ch;
    }
    return hash;
}

void performance_server(char *port, bool qFlag)
{
    int server_sock = 0, client_sock = 0, optval = 0;

    socklen_t addr_size = 0, addr_size6 = 0, addr_size_unix = 0;
    struct sockaddr_in server_addr = {0}, client_addr = {0};
    memset(&server_sock, 0, sizeof(server_addr));
    memset(&client_sock, 0, sizeof(client_sock));

    char type[BUFSIZ / 1024] = {0}, param[BUFSIZ / 1024] = {0}, buffer[BUFSIZ] = {0};
    memset(type, 0, sizeof(type));
    memset(type, 0, sizeof(param));
    memset(type, 0, sizeof(buffer));

    ssize_t bytes_recv = 0;

    struct timeval start = {0}, end = {0};
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));

    struct sockaddr_in6 server_addr6, client_addr6 = {0};
    memset(&server_sock, 0, sizeof(server_addr));
    memset(&client_sock, 0, sizeof(client_sock));

    struct stat stat = {0};
    memset(&stat, 0, sizeof(struct stat));

    void *ptr = NULL;

    struct sockaddr_un unix_addr = {0};
    memset(&unix_addr, 0, sizeof(struct sockaddr_un));

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

        end.tv_sec -= 1;
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

        if (bind(server_sock, (struct sockaddr *)&server_addr6, addr_size6) < 0)
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

        end.tv_sec -= 1;
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "stream"))
    {
        server_sock = socket(AF_UNIX, SOCK_STREAM, PF_UNIX);
        if (server_sock <= 0)
        {
            perror("socket() failed");
            exit(errno);
        }

        unlink("/tmp/uds_socket");

        addr_size_unix = sizeof(struct sockaddr_un);
        unix_addr.sun_family = AF_UNIX;

        strncpy(unix_addr.sun_path, "/tmp/uds_socket", addr_size_unix - 1);

        if (bind(server_sock, (struct sockaddr *)&unix_addr, addr_size_unix) < 0)
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

        client_sock = accept(server_sock, NULL, NULL);
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
    }
    else if (!strcmp(type, "uds") && !strcmp(param, "dgram"))
    {
        server_sock = socket(AF_UNIX, SOCK_DGRAM, PF_UNIX);
        if (server_sock <= 0)
        {
            perror("socket() failed");
            exit(errno);
        }

        unlink("/tmp/uds_socket");

        addr_size_unix = sizeof(struct sockaddr_un);
        unix_addr.sun_family = AF_UNIX;

        strncpy(unix_addr.sun_path, "/tmp/uds_socket", addr_size_unix - 1);

        if (bind(server_sock, (struct sockaddr *)&unix_addr, addr_size_unix) < 0)
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

        end.tv_sec -= 1;
    }
    else if (!strcmp(type, "pipe"))
    {
        mkfifo("/tmp/pipe", 0666);

        server_sock = open("/tmp/pipe", O_RDONLY);
        if (server_sock <= 0)
        {
            perror("open() failed");
            exit(errno);
        }

        gettimeofday(&start, NULL);
        bytes_recv = read(server_sock, buffer, BUFSIZ);
        while (bytes_recv > 0)
        {
            bytes_recv = read(server_sock, buffer, BUFSIZ);
        }

        gettimeofday(&end, NULL);
    }
    else if (!strcmp(type, "mmap"))
    {
        server_sock = open(param, O_RDWR | O_CREAT, 0666);
        if (server_sock <= 0)
        {
            perror("open() failed");
            exit(errno);
        }

        if (ftruncate(server_sock, MB_100) < 0)
        {
            perror("ftruncate() error");
            close(server_sock);
            exit(errno);
        }

        ptr = mmap(NULL, MB_100, PROT_READ | PROT_WRITE, MAP_SHARED, server_sock, 0);
        if (ptr == MAP_FAILED)
        {
            perror("mmap() failed");
            close(server_sock);
            exit(errno);
        }

        client_sock = open("mmap.bin", O_WRONLY | O_CREAT, 0666);
        if (client_sock <= 0)
        {
            perror("open() failed");
            close(server_sock);
            exit(errno);
        }

        gettimeofday(&start, NULL);
        if (write(client_sock, ptr, MB_100) < 0)
        {
            perror("write() failed");
            close(server_sock);
            exit(errno);
        }
        gettimeofday(&end, NULL);

        if (munmap(ptr, MB_100) < 0)
        {
            perror("munmap() failed");
            close(server_sock);
            exit(errno);
        }

        close(client_sock);
        close(server_sock);
    }

    if (!strcmp(type, "mmap") || !strcmp(type, "pipe"))
    {
        fprintf(stdout, "%s,%ld\n", type, ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));

        if (!qFlag)
        {
            fprintf(stdout, "Checksum=%ld\n", hash(buffer));
        }

        return;
    }

    fprintf(stdout, "%s_%s,%ld\n", type, param, ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));

    if (!qFlag)
    {
            fprintf(stdout, "Checksum=%ld\n", hash(buffer));
    }
}
