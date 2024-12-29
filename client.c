#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "file.h"

#define buffer_size 104857600 

void url_decode(const char* url){
    
}

// thread function that handles client http requests
void *handle_client(void *arg){
    int client_fd = *((int *)arg);
    clientConnected(client_fd);

    char *buffer = (char*)malloc(buffer_size * sizeof(char));
    char errorMessage[1024] = {0};                                              // for error messages in activity_log.txt
    if(!buffer){
        perror("\nError: Failed to allocate memory for buffer");
        strcpy(errorMessage, strerror(errno));
        closeLogFile(errorMessage);
        close(client_fd);
        pthread_exit(NULL);
    }

    // receive request from client and store in buffer
    ssize_t bytes_recv = recv(client_fd, buffer, buffer_size, 0);
    if (bytes_recv < 0){
        perror("\nError: receive failed");
        clientDisconnected(client_fd);
        close(client_fd);
        free(buffer);
        buffer = NULL;
        pthread_exit(NULL);
    }
    // client disconnected
    else if(bytes_recv == 0){
        clientDisconnected(client_fd);
        close(client_fd);
        free(buffer);
        buffer = NULL;
        pthread_exit(NULL);
    }
    // process http request
    else{
        if(bytes_recv >= buffer_size){
            fprintf(stderr, "Buffer overflow detected.\n");
            close(client_fd);
            free(buffer);
            pthread_exit(NULL);
        }
        printf("\nRaw Request:\n%s", buffer);
        fflush(stdout);
        openBufferFile(buffer);
        printHexDump(buffer, bytes_recv);
    }

    free(arg);
    free(buffer);
    buffer = NULL;
    return NULL;
}
