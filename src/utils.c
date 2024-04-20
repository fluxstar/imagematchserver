#include "../include/utils.h"

#include <arpa/inet.h>  // inet_addr()
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  // bzero()
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>  // read(), write(), close()
#include <unistd.h>

#include "../include/server.h"

// TODO: Declare a global variable to hold the file descriptor for the server
// socket
int master_fd;

// TODO: Declare a global variable to hold the mutex lock for the server socket
pthread_mutex_t socket_lock = PTHREAD_MUTEX_INITIALIZER;

// TODO: Declare a gloabl socket address struct to hold the address of the
// server
struct sockaddr_in server_addr;

/*
################################################
##############Server Functions##################
################################################
*/

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
    // TODO: create an int to hold the socket file descriptor
    // TODO: create a sockaddr_in struct to hold the address of the server

    int sd;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /**********************************************
     * IMPORTANT!
     * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM
     *SUBMISSION!!!!
     **********************************************/
    // TODO: Create a socket and save the file descriptor to sd (declared above)
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // TODO: Change the socket options to be reusable using setsockopt().
    int enable = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(int)) <
        0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // TODO: Bind the socket to the provided port.
    if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // TODO: Mark the socket as a pasive socket. (ie: a socket that will be used
    // to receive connections)
    if (listen(sd, 20) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // We save the file descriptor to a global variable so that we can use it in
    // accept_connection().
    // TODO: Save the file descriptor to the global variable master_fd
    master_fd = sd;

    printf("UTILS.O: Server Started on Port %d\n", port);
    fflush(stdout);
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {
    // TODO: create a sockaddr_in struct to hold the address of the new
    // connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int newsock;
    /**********************************************
     * IMPORTANT!
     * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM
     *SUBMISSION!!!!
     **********************************************/

    // TODO: Aquire the mutex lock
    pthread_mutex_lock(&socket_lock);

    // TODO: Accept a new connection on the passive socket and save the fd to
    // newsock
    newsock = accept(master_fd, (struct sockaddr *)&client_addr, &client_len);
    if (newsock < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // TODO: Release the mutex lock
    pthread_mutex_unlock(&socket_lock);

    // TODO: Return the file descriptor for the new client connection
    return newsock;
}

/**********************************************
 * send_file_to_client
   - socket is the file descriptor for the socket
   - buffer is the file data you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure
************************************************/
int send_file_to_client(int socket, char *buffer, int size) {
    // TODO: create a packet_t to hold the packet data
    packet_t packet;
    packet.size = htonl(size);

    // TODO: send the file size packet
    if (send(socket, &packet, sizeof(packet_t), 0) == -1) {
        perror("sending packet");
        return -1;
    }

    // TODO: send the file data
    if (send(socket, buffer, size, 0) < 0) {
        perror("Send failed");
        return -1;
    }
    // TODO: return 0 on success, -1 on failure
    return 0;
}

/**********************************************
 * get_request_server
   - fd is the file descriptor for the socket
   - filelength is a pointer to a size_t variable that will be set to the length
of the file
   - returns a pointer to the file data
************************************************/
char *get_request_server(int fd, size_t *filelength) {
    // TODO: create a packet_t to hold the packet data
    packet_t packet;

    // TODO: receive the response packet
    if (recv(fd, &packet, sizeof(packet_t), 0) < 0) {
        perror("Receive failed");
        return NULL;
    }

    // TODO: get the size of the image from the packet
    *filelength = ntohl(packet.size);

    // TODO: recieve the file data and save into a buffer variable.
    char *buffer = (char *)malloc(*filelength);
    if (recv(fd, buffer, *filelength, 0) < 0) {
        perror("Receive failed");
        free(buffer);
        return NULL;
    }

    // TODO: return the buffer
    return buffer;
}

/*
################################################
##############Client Functions##################
################################################
*/

/**********************************************
 * setup_connection
   - port is the number of the port you want the client to connect to
   - initializes the connection to the server
   - if setup_connection encounters any errors, it will call exit().
************************************************/
int setup_connection(int port) {
    // TODO: create a sockadzdr_in struct to hold the address of the server
    struct sockaddr_in server;

    // TODO: create a socket and save the file descriptor to sockfd
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("creating sock_fd");
        exit(1);
    }

    // TODO: assign IP, PORT to the sockaddr_in struct
    memset(&server, 0, sizeof(server_addr));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = server_addr.sin_addr.s_addr;
    server.sin_port = htons(port);

    // TODO: connect to the server
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connecting to server");
        exit(1);
    }

    // TODO: return the file descriptor for the socket
    return sock_fd;
}

/**********************************************
 * send_file_to_server
   - socket is the file descriptor for the socket
   - file is the file pointer to the file you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure
************************************************/
int send_file_to_server(int socket, FILE *file, int size) {
    int converted_size = htonl(size);
    if (send(socket, &converted_size, sizeof(converted_size), 0) < 0) {
        perror("Failed to send file");
        return -1;
    }

    // Allocate buffer to hold the file data
    char *buffer = malloc(size);
    if (buffer == NULL) {
        perror("Failed to allocate memory for file data");
        return -1;
    }
    // Read file content into buffer

    int bytes_read = fread(buffer, 1, size, file);
    if (bytes_read != size) {
        perror("Failed to read file data");
        free(buffer);
        return -1;
    }

    // TODO: send the file data

    int bytes_sent = send(socket, buffer, bytes_read, 0);
    if (bytes_sent < 0) {
        perror("Failed to send file data");
        free(buffer);
        return -1;
    }

    // TODO: return 0 on success, -1 on failure
    free(buffer);
    return 0;
}

/**********************************************
 * receive_file_from_server
   - socket is the file descriptor for the socket
   - filename is the name of the file you want to save
   - returns 0 on success, -1 on failure
************************************************/
int receive_file_from_server(int socket, const char *filename) {
    // TODO: create a buffer to hold the file data
    char buffer[1024];

    // TODO: open the file for writing binary data
    printf("Filename: %s\n", filename);
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Opening file");
        return -1;
    }

    // TODO: create a packet_t to hold the packet data
    packet_t packet;

    // TODO: receive the response packet
    if (recv(socket, &packet, sizeof(packet), 0) == -1) {
        perror("receiving packet");
        return -1;
    }

    // TODO: get the size of the image from the packet
    size_t filelength = ntohl(packet.size);

    // TODO: recieve the file data and write it to the file
    if (recv(socket, buffer, filelength, 0) == -1) {
        perror("receiving file data");
        fclose(fp);
        return -1;
    }

    if (fwrite(buffer, filelength, 1, fp) != 1) {
        perror("Failed to write to file");
        fclose(fp);
        return -1;
    }

    // TODO: return 0 on success, -1 on
    return 0;
}
