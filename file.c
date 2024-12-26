#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "file.h"

int client_count = 0;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// prints when server was started to activity_log.txt
void openLogFile(){
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }
    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);
    fprintf(file, "-----------------------------------------------\n");
    fprintf(file, "Activity log started on %sServer has been started.\n", asctime(local_time));
    fclose(file);
}

// prints when server was stopped to activity_log.txt
void closeLogFile(char* errorMessage){
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }

    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);
    
    // if server closes due to an error logic
    if(errorMessage == NULL){
        fprintf(file, "Server has been stopped.\nActivity log closed on %s", asctime(local_time));
        fclose(file);
    }
    else{
        fprintf(file, "Server has been stopped due to an error:\n");
        fprintf(file, "'%s'\n", errorMessage);
        fprintf(file, "Activity log closed on %s", asctime(local_time));
        fclose(file);
    }
    
}

// prints when a client has been connected to server to activity_log.txt
void clientConnected(int client_socket){
    client_count++;
    pthread_mutex_lock(&log_mutex);
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Client connected to server. There are %d client(s) connected.\n", client_count);
    fclose(file);

    /* 
        client socket will be a value >3 because it is a file descriptor
        file descriptor 0 = stdin, 1 = stdout, 2 = stderr
        therefore server_socket in main will == 3 and every socket afterwards
        shall have a value >3
    */
    printf("\nClient %d has connected to server.", client_socket - 3);
    fflush(stdout);

    pthread_mutex_unlock(&log_mutex);
}

// prints when a client has been disconnected to server to activity_log.txt
void clientDisconnected(int client_socket){
    pthread_mutex_lock(&log_mutex);
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }

    if (client_count > 0) {
        client_count--;
    }
    fprintf(file, "Client disconnected from server. There are %d client(s) connected to the server.\n", client_count);
    fclose(file);

    /* 
        client socket will be a value > 3 because it is a file descriptor
        file descriptor 0 = stdin, 1 = stdout, 2 = stderr
        therefore server_fd in main will == 3 and every client_fd afterwards
        shall have a value > 3
    */
    printf("\nClient %d has disconnected from server.", client_socket - 3);
    fflush(stdout);
          
    pthread_mutex_unlock(&log_mutex);
}

void openBufferFile(char* buffer){
    FILE *file = fopen("http_request.txt", "a");
    fprintf(file, "%s", buffer);
    fclose(file);
}