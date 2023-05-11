/**
 * @brief this file contains declarations for chat
 * @since 11/05/2023
 * @authors Lior Vinman & Yoad Tamar
*/


#ifndef _STNC_H_

    #include <stdio.h>

    /**
     * @brief a flag to mark the header file (so wont be included more than once in total)
     */
    #define _STNC_H_

    /**
     * @brief number of clients that server can handle
     */
    #define NUM_OF_CLIENTS 1

    /**
     * @brief maximal number of client sockets
     */
    #define MAX_EVENTS NUM_OF_CLIENTS + 1

    /**
     * @brief for enabaling reuse of port
    */
    #define SO_REUSEPORT 15

    /**
     * @brief length of time in format "DD.MM.YYYY - HH:MM:SS"
    */
    #define TIME_LENGTH 25

    /**
     * @brief 100MB for generating a chunk of data
    */
    #define MB_100 100 * 1024 * 1024

    /**
     * @brief client's socket file descriptor
     */
    int client_sock = 0;

    /**
     * @brief listening socket file descriptor for server
     */
    int server_sock = 0;

    /**
     * @brief a buffer with random 100MB
    */
    char *chunk = NULL;

    /**
     * @brief this function is the client chat functionality
     * @param char* an ipv4 address of the server
     * @param char* a port of the server
    */
    void start_client(char*, char*);

    /**
     * @brief this function is the server chat functionality
     * @param char* a port to open for chat connection
    */
    void start_server(char*);

    /**
     * @brief this function handle of proggram exit & ^C managment
     * @param int signal ID
    */
    void exit_handle(int);

    /**
     * @brief this function is a performace test for client
     * @param char* an ipv4 address of the server
     * @param char* a port of the server
     * @param char* type of communication: ipv4, ipv6, uds, mmap, pipe
     * @param char* type parameter: tcp, udp, stream, dgram, filename
    */
    void client_performace(char*, char*, char*, char*);

    /**
     * @brief this function is a performace test for server
     * @param char* a port of the server
    */
    void server_performace(char*);

    /**
     * @brief this function is for generating random 100MB
     * @param char* a pointer to buffer
     * @param size_t how much data generate
    */
    void generate_data(char*, size_t);

#endif
