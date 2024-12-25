#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <pthread.h>
#include "request.h"
#include "file.h"
#include <signal.h>

#define PORT 12345

// signal handler for SIGINT
void sigInt(int sig){
    printf("\nShutting down server...");
    sleep(5);
    closeLogFile();
    exit(EXIT_SUCCESS);
}
int main(){
    signal(SIGINT, sigInt);     //sigint handler
    openLogFile();              // open activity_log to document requests  

    int server_socket;          // create & bind sockets
    int *client_socket = malloc(sizeof(int));
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error: Failed to create socket.");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.");
    fflush(stdout);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))< 0){
        perror("\nError: Failed to bind to socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("\nSocket bound to port %d.", PORT);

    if(listen(server_socket, 5) < 0){
        perror("\nError: Failed to listen on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("\nServer listening on port %d.", PORT);
    fflush(stdout);

    while(1){
        *client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if(client_socket< 0){
            perror("\nError: Failed to accept client connection");
            free(client_socket);
            continue;
        }
        printf("\nClient connected to server.");
        fflush(stdout);
        
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, processCommands, (void*)client_socket) != 0){
            perror("\nThread creation failed");
            free(client_socket);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(server_socket);
    closeLogFile();
    return 0;
}