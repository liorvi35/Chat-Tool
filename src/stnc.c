#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void chat_client(char *, char *);
void chat_server(char *);
void performance_client(char *, char *, char *, char *);
void performance_server(char *, bool);

void general_usage()
{
    fprintf(stderr, "Usage: ./stnc [OPTIONS] [OPTIONAL]\n\nDescription:\n\tSTNC is a CMD chat tool.\n\nOptions:\n\t(client-chat) -c <IP> <PORT>.\n\t(server-chat) -s <PORT>.\n\nOptional:\n\t(client-test) -p <param> <type>.\n\n\t\tCombinations:\n\t\t\tipv4 tcp.\n\t\t\tipv4 udp.\n\t\t\tipv6 tcp.\n\t\t\tipv6 udp.\n\t\t\tuds stream.\n\t\t\tuds dgram.\n\t\t\tmmap <file>.\n\t\t\tpipe <file>.\n\n\t(server-test) -p.\n\t(server-test, quiet mode) -p -q.\n\nExamples:\n\t./stnc -c 127.0.0.1 65342\n\t./stnc -s 65342\n\n\t./stnc -c 127.0.0.1 65432 -p ipv4 tcp\n\t./stnc -s 65432 -p -q\n\nAuthors - All rights reserved (c):\n\tLior Vinman.\n\tYoad Tamar.\n");
    exit(EXIT_FAILURE);
}

void client_usage()
{
    fprintf(stderr, "Usage: ./stnc -c <IP> <PORT> [OPTIONAL]\n\nOptional:\n\t(client-test) -p <param> <type>.\n\n\t\tCombinations:\n\t\t\tipv4 tcp.\n\t\t\tipv4 udp.\n\t\t\tipv6 tcp.\n\t\t\tipv6 udp.\n\t\t\tuds stream.\n\t\t\tuds dgram.\n\t\t\tmmap <file>.\n\t\t\tpipe <file>.\n\nExamples:\n\t./stnc -c 127.0.0.1 65342\n\t./stnc -c 127.0.0.1 65432 -p ipv4 tcp\n");
    exit(EXIT_FAILURE);
}

void server_usage()
{
    fprintf(stderr, "Usage: ./stnc -s <PORT> [OPTIONAL]\n\nOptional:\n\t(server-test) -p.\n\t(server-test, quiet mode)-p -q.\n\nExamples:\n\t./stnc -s 65342\n\t./stnc -s 65432 -p -q\n");
    exit(-1);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-s")))
    {
        general_usage();
    }
    else
    {
        if (!strcmp(argv[1], "-c"))
        {
            if (argc != 4 && argc != 7)
            {
                client_usage();
            }
            else if (argc == 4)
            {
                chat_client(argv[2], argv[3]);
            }
            else
            {
                if (strcmp(argv[4], "-p"))
                {
                    client_usage();
                }
                else
                {
                    if (((!strcmp(argv[5], "ipv4") && !strcmp(argv[6], "tcp")) || (!strcmp(argv[5], "ipv4") && !strcmp(argv[6], "udp")) || (!strcmp(argv[5], "ipv6") && !strcmp(argv[6], "tcp")) || (!strcmp(argv[5], "ipv6") && !strcmp(argv[6], "udp")) || (!strcmp(argv[5], "uds") && !strcmp(argv[6], "stream")) || (!strcmp(argv[5], "uds") && !strcmp(argv[6], "dgram"))) || !strcmp(argv[5], "pipe") || !strcmp(argv[5], "mmap"))
                    {
                        performance_client(argv[2], argv[3], argv[5], argv[6]);
                    }
                    else
                    {
                        client_usage();
                    }
                }
            }
        }
        else
        {
            if (argc != 3 && argc != 4 && argc != 5)
            {
                server_usage();
            }
            else if (argc == 3)
            {
                chat_server(argv[2]);
            }
            else if (argc == 4)
            {
                if (strcmp(argv[3], "-p"))
                {
                    server_usage();
                }
                else
                {
                    performance_server(argv[2], false);
                }
            }
            else
            {
                if (strcmp(argv[3], "-p") && strcmp(argv[4], "-q"))
                {
                    server_usage();
                }
                else
                {
                    performance_server(argv[2], true);
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}