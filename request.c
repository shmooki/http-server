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
#include <time.h>
#include "file.h"

#define buffer_size 104857600 

void *processCommands(void *arg){
    int client_socket = *((int *)arg);
    free(arg);
    clientConnected(client_socket);

    char *buffer = (char*)malloc(buffer_size * sizeof(char));
    if(!buffer){
        perror("\nError: Failed to allocate memory for buffer");
        close(client_socket);
        pthread_exit(NULL);
    }
    
    while(1){
        ssize_t bytes_recv = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_recv < 0){
            perror("\nError: receive failed");
            clientDisconnected(client_socket);
            close(client_socket);
            free(buffer);
            buffer = NULL;
            pthread_exit(NULL);
        }
        else if(bytes_recv == 0){
            clientDisconnected(client_socket);
            close(client_socket);
            free(buffer);
            buffer = NULL;
            pthread_exit(NULL);
        }
        else{
            // process http request
        }
    }
    free(buffer);
    buffer = NULL;
    return NULL;
}