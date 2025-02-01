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

#define buffer_size 8192

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

char* url_decode(const char* url){
    size_t url_len = strlen(url);
    char *request = malloc(url_len + 1);  
    char *start = strchr(url, '/');
    
    // Move past the '/' character
    start++;

    // Loop through the encoded URL until '\n' or end of string, copying to output
    size_t i = 0;
    while (*start != ' ' && i < url_len - 1) {
        request[i++] = *start++;
    }

    request[i] = '\0';
    return request;
}

char *get_file_ext(const char* request){
    size_t ext_len = strlen(request);
    char *file_ext = malloc(ext_len);  
    char *start = strchr(request, '.');
    
    // Move past the '/' character
    start++;

    // Loop through the encoded URL until '\n' or end of string, copying to output
    size_t i = 0;
    while (*start != ' ' && i < ext_len) {
        file_ext[i++] = *start++;
    }

    file_ext[i] = '\0';
    return file_ext;
}

// char *get_mime_type(){

// }
// void build_response(){

// }

// thread function that handles client http requests
void *handle_client(void *arg){
    int client_fd = *((int *)arg);
    free(arg);
    clientConnected(client_fd);

    // buffer for errors to be printed to log
    char errorMessage[1024] = {0}; 

    // buffer for requests    
    char *buffer = (char*)malloc(buffer_size * sizeof(char)); 
    pthread_mutex_lock(&buffer_mutex);                                       
    if(!buffer){
        perror("\nError: Failed to allocate memory for buffer");
        strcpy(errorMessage, strerror(errno));
        closeLogFile(errorMessage);
        close(client_fd);
        pthread_mutex_unlock(&buffer_mutex); 
        pthread_exit(NULL);
    }

    // receive request from client and store in buffer 
    ssize_t total_bytes = 0;
    while(1){
        ssize_t bytes_recv = recv(client_fd, buffer, buffer_size, 0);

        // logic for non-blocking mode recv
        if(bytes_recv < 0){
            // no data available
            if(errno == EWOULDBLOCK || errno == EAGAIN){
                usleep(10000);
                continue;
            }
            else if(errno == ECONNRESET || errno == EPIPE){
                printf("\nClient forcibly disconnected.");
                fflush(stdout);
                clientDisconnected(client_fd);
                free(buffer);
                break;
            }
            else{
                perror("\nError: receive failed");
                clientDisconnected(client_fd);
                free(buffer);
                break;
            }
        }
        // client disconnected
        else if(bytes_recv == 0){
            clientDisconnected(client_fd);
            free(buffer);
            break;
        }
        // process http request
        else{
            // check for buffer overflow
            total_bytes += bytes_recv;
            if(total_bytes >= buffer_size){
                fprintf(stderr, "Buffer overflow detected.\n");
                free(buffer);
                break;
            }

            // null terminate string to ensure it's read correctly
            buffer[total_bytes] = '\0';
            openBufferFile(buffer);
            printHexDump(buffer, bytes_recv);
            
            char *decoded_buffer = url_decode(buffer);
            if(decoded_buffer){
                free(buffer);
                buffer = decoded_buffer;
            }
            printf("\n%s", buffer);
            fflush(stdout);

            char *file_ext = get_file_ext(buffer);
            printf("\n%s", file_ext);
            fflush(stdout);
        }
    }

    close(client_fd);
    pthread_mutex_unlock(&buffer_mutex);
    pthread_exit(NULL);
}
