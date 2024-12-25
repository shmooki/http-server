#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "file.h"

int client_count = 1;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// opens activity_log.txt and saves relevant commands for eventviewing
void openLogFile(){
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }
    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);
    fprintf(file, "Activity log started on %sServer has been started.\n", asctime(local_time));
    fclose(file);
}

void closeLogFile(){
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }
    time_t curTime = time (NULL);
    struct tm *local_time = localtime(&curTime);
    fprintf(file, "Activity log closed on %sServer has been stopped.\n", asctime(local_time));
    fclose(file);
}

void clientConnected(){
    pthread_mutex_lock(&log_mutex);
    FILE* file = fopen("activity_log.txt", "a");
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Client %d connected to server\n", client_count);
    fclose(file);
    pthread_mutex_unlock(&log_mutex);

    if(!client_count){
        client_count++;
    }
}

void clientDisconnected(){
    FILE* file = fopen("activity_log.txt", "a");
    pthread_mutex_lock(&log_mutex);
    if(!file){
        perror("\nError: Failed to open actitvity_log.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Client %d disconnected from server\n", client_count);
    fclose(file);
    pthread_mutex_unlock(&log_mutex);

    if(client_count > 1){
        client_count--;
    }
}