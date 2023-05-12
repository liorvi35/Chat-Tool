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

void performance_client(char *ip, char *port, char *type, char *param)
{
    int server_sock = 0;
    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_sock <= 0)
    {
        perror("socket() failed");
    }
}