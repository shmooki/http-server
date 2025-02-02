#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include "file.h"
#include <unistd.h>

int client_count = 0;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;      // mutex for thread-safe file access

// prints when server was started to server_activity.log
void openLogFile(){
    pthread_mutex_lock(&log_mutex);
    FILE* file = fopen("server_activity.log", "a");
    if(!file){
        perror("\nError: Failed to open server_activity.log");
        exit(EXIT_FAILURE);
    }
    time_t curTime = time(NULL);
    struct tm *local_time = localtime(&curTime);
    fprintf(file, "-----------------------------------------------\n");
    fprintf(file, "Activity log started on %sServer has been started.\n", asctime(local_time));
    fclose(file);
    pthread_mutex_unlock(&log_mutex);
}

// prints when server was stopped to server_activity.log
void closeLogFile(char* errorMessage){
    pthread_mutex_lock(&log_mutex);
    FILE* file = fopen("server_activity.log", "a");
    if(!file){
        perror("\nError: Failed to open server_activity.log");
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
    pthread_mutex_unlock(&log_mutex);
}

// prints when a client has been connected to server to server_activity.log
void clientConnected(int client_socket){
    pthread_mutex_lock(&log_mutex);
    client_count++;
    FILE* file = fopen("server_activity.log", "a");
    if(!file){
        perror("\nError: Failed to open server_activity.log");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "A client has connected to the server. There are %d client(s) connected.\n", client_count);
    fclose(file);

    /* 
        client socket will be a value >3 because it is a file descriptor
        file descriptor 0 = stdin, 1 = stdout, 2 = stderr
        therefore server_socket in main will == 3 and every socket afterwards
        shall have a value >3
    */
    printf("\nA client has connected to the server.");
    fflush(stdout);

    pthread_mutex_unlock(&log_mutex);
}

// prints when a client has been disconnected to server to server_activity.log
void clientDisconnected(int client_socket){
    pthread_mutex_lock(&log_mutex);
    FILE* file = fopen("server_activity.log", "a");
    if(!file){
        perror("\nError: Failed to open server_activity.log");
        exit(EXIT_FAILURE);
    }

    if (client_count > 0) {
        client_count--;
    }
    fprintf(file, "A client has disconnected from the server. There are %d client(s) connected to the server.\n", client_count);
    fclose(file);

    /* 
        client socket will be a value > 3 because it is a file descriptor
        file descriptor 0 = stdin, 1 = stdout, 2 = stderr
        therefore server_fd in main will == 3 and every client_fd afterwards
        shall have a value > 3
    */
    printf("\nA client has disconnected from the server (socket# = %d)", client_socket - 3);
    fflush(stdout);
          
    pthread_mutex_unlock(&log_mutex);
}

void openBufferFile(char* buffer){
    pthread_mutex_lock(&log_mutex);
    FILE *file = fopen("http_request.log", "a");
    if(!file){
        perror("\nError: Failed to open http_request.log");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s", buffer);
    fclose(file);
    pthread_mutex_unlock(&log_mutex);
}

void openResponseFile(char *response){
    pthread_mutex_lock(&log_mutex);

    FILE *file = fopen("http_response.log", "a");
    if(!file){
        perror("\nError: Failed to open http_request.log");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n-----------------------------------------------\n");
    fprintf(file, "%s", response);
    fclose(file);

    printf("\nA response has been sent to the client.");
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}
// for debugging
void printHexDump(const char *buffer, ssize_t len) {
    pthread_mutex_lock(&log_mutex);
    FILE *file = fopen("hex_dump.log", "a");
    if(!file){
        perror("Error: Failed to open hex_dump.log");
        exit(EXIT_FAILURE);
    }

    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);

    fprintf(file, "-----------------------------------------------\n");
    fprintf(file, "Hex Dump on %s\n", asctime(local_time));
    for (ssize_t i = 0; i < len; i++) {
        fprintf(file, "%02x ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) fprintf(file, "\n");
    }
    fprintf(file, "\n");
    fclose(file);
    pthread_mutex_unlock(&log_mutex);
}