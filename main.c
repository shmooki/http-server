#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include "client.h"
#include "file.h"

// TODO random port function
// TODO close server_fd & client_fd added to sigInt function somehow (?)
#define PORT 8080

// global file descriptors
int server_fd = -1;
int client_fd = -1;

// thread safe access to client_fd
pthread_mutex_t client_fd_mutex = PTHREAD_MUTEX_INITIALIZER;

// signal handler for SIGINT
void sigInt(int sig){
    printf("\nShutting down server...\n");
    fflush(stdout);
    sleep(2);

    // close client socket
    pthread_mutex_lock(&client_fd_mutex);
    if (client_fd != -1) {
        close(client_fd);
        printf("\nClosed client socket.\n");
        fflush(stdout);
    }
    pthread_mutex_unlock(&client_fd_mutex);

    // close server socket
    
    if (server_fd != -1) {
        close(server_fd);
        printf("\nClosed server socket.\n");
        fflush(stdout);
    }

    closeLogFile(NULL);
    exit(EXIT_SUCCESS);
}

// function to set socket to non blocking mode (needed for certain functionalities)
void set_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("Error: Failed to get socket flags");
        return;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error: Failed to set socket to non-blocking mode");
    }
}

int main(){
    signal(SIGINT, sigInt);         //sigint handler
    openLogFile();                  // open activity_log.txt to document requests  
    char errorMessage[1024] = {0};  // for error messages in activity_log.txt

    // create server and client sockets 
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error: Failed to create socket.");
        strcpy(errorMessage, strerror(errno));      
        strcat(errorMessage, ". Failed to create socket.");                     // concatenate error so it can be printed to activity_log.txt
        close(server_fd);
        closeLogFile(errorMessage);
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.");
    fflush(stdout);
    
    // set server socket to non-blocking mode
    set_nonblocking(server_fd);

    // bind server socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))< 0){
        perror("\nError: Failed to bind to socket");
        strcpy(errorMessage, strerror(errno));
        strcat(errorMessage, ". Failed to bind to socket.");                    // concatenate error so it can be printed to activity_log.txt
        close(server_fd);
        closeLogFile(errorMessage);
        exit(EXIT_FAILURE);
    }
    printf("\nSocket bound to port %d.", PORT);

    // listen for incoming connections
    if(listen(server_fd, 5) < 0){
        perror("\nError: Failed to listen on socket");
        strcpy(errorMessage, strerror(errno));
        strcat(errorMessage, ". Failed to listen on socket.");                  // concatenate error so it can be printed to activity_log.txt
        close(server_fd);
        closeLogFile(errorMessage);
        exit(EXIT_FAILURE);
    }
    printf("\nServer listening on port %d.", PORT);
    fflush(stdout);

    // process incoming client connections
    while(1){
        pthread_mutex_lock(&client_fd_mutex);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        pthread_mutex_unlock(&client_fd_mutex);

        if(client_fd < 0){
            // no incoming connection; continue
            if(errno == EWOULDBLOCK || errno == EAGAIN){
                usleep(10000);
                continue;
            }
            else{
                perror("\nError: Failed to accept client connection");
                strcpy(errorMessage, strerror(errno));
                strcat(errorMessage, ". Failed accept client connection.");         // concatenate error so it can be printed to activity_log.txt
                close(client_fd);
                closeLogFile(errorMessage);
                continue;   // can try again w/ this error
            }
        }

        // set client socket to non-blocking mode
        set_nonblocking(client_fd);

        int *client_ptr = malloc(sizeof(int));
        if(!client_ptr){
            perror("\nError: Failed to allocate memory for client_ptr");
            strcpy(errorMessage, strerror(errno));
            strcat(errorMessage, ". Failed to allocate memory for client_ptr");         // concatenate error so it can be printed to activity_log.txt
            close(client_fd);
            closeLogFile(errorMessage);
            exit(EXIT_FAILURE);
        }
        *client_ptr = client_fd;
        pthread_t thread_id;

        // send client_fd var to new thread to send requests to server_fd
        if(pthread_create(&thread_id, NULL, handle_client, client_ptr) != 0){
            perror("\nError: Failed to create thread");
            strcpy(errorMessage, strerror(errno));
            strcat(errorMessage, ". Failed to create thread.");                 // concatenate error so it can be printed to activity_log.txt
            close(client_fd);
            free(client_ptr);
            closeLogFile(errorMessage);
            continue;   // can try again w/ this error
        }

        // detach thread so it runs independently of other
        // main doesn't have to worry about its return values or completion
        pthread_detach(thread_id);
    }

    // clean up
    close(client_fd);
    close(server_fd);
    closeLogFile(errorMessage);
    return 0;
}