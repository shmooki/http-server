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
#include "request.h"

#define PORT 8080

// opens activity_log.txt and saves relevant commands for eventviewing
void openLogFile(){
    FILE* file = fopen("activity_log.txt", "a");
    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);
    fprintf(file, "Activity log started on %sServer has been started.\n", asctime(local_time));
    fclose(file);
}

void closeLogFile(){
    FILE* file = fopen("activity_log.txt", "a");
    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);
    fprintf(file, "Activity log closed on %s\n.Server has been stopped.", asctime(local_time));
    fclose(file);
}

void clientConnected(int client_count){
    FILE* file = fopen("activity_log.txt", "a");
    fprintf(file, "Client %d connected to server\n", client_count);
    fclose(file);
}
int main(){
    // open activity_log to document requests
    openLogFile();  

    // create & bind sockets
    int server_socket;
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

    // for printing number of clients connected to activity_log.txt
    int client_count = 0;
    while(1){
        *client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if(client_socket< 0){
            perror("\nError: Failed to accept client connection");
            exit(EXIT_FAILURE);
        }
        printf("\nClient connected to server.");
        fflush(stdout);
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, processCommands, (void*)client_socket);
        pthread_detach(thread_id);
        
        client_count++;
        clientConnected(client_count);
    }

    close(server_socket);
    closeLogFile();
    return 0;
}