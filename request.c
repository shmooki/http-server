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

#define buffer_size 104857600 

void *processCommands(void *arg){
    int client_socket = *((int *)arg);
    char *buffer = (char*)malloc(buffer_size * sizeof(char));
    if (recv(client_socket, buffer, buffer_size, 0) < 0){
        perror("Error: send failed");
        exit(EXIT_FAILURE);
    }
    return NULL;
}