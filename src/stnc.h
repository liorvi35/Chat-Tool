/**
 * @brief this file contains declarations for chat
 * @since 11/05/2023
 * @authors Lior Vinman & Yoad Tamar
*/


#ifndef _STNC_H_

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
     * @brief client's socket file descriptor
     */
    int client_sock = 0;

    /**
     * @brief listening socket file descriptor for server
     */
    int server_sock = 0;

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

#endif
