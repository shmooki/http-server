#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
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

char *get_file_name(const char *file_name){
    size_t name_len = strlen(file_name);
    char *name = malloc(name_len);  
    char *start = strdup(file_name);

    printf("DEBUG: %p\n", start);

    printf("\n%s", file_name);
    printf("\nhelo from function\n");

    size_t i = 0;
    while (*start != '.' && i < name_len) {
        name[i++] = *start++;
    }

    name[i] = '\0';
    printf("\n%s\n", name);

    free(start);
    start = NULL;
    printf("\nhiiiiii\n");
    return name;
}

char *get_mime_type(const char *file_ext){
    if (strcmp(file_ext, "html") == 0){
        return "text/html";
    } else if(strcmp(file_ext, "txt") == 0){
        return "text/plain";
    } else if(strcmp(file_ext, "png") == 0){
        return "image/png";
    } else if(strcmp(file_ext, "gif") == 0){
        return "image/gif";
    } else if(strcmp(file_ext, "mp4") == 0){
        return "video/mp4";
    } else{
        return "application/octet-stream";
    }
}

void http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len){
    const char *mime_type = get_mime_type(file_ext);
    char *header = (char *)malloc(buffer_size * sizeof(char));
    struct stat file_stat;

    snprintf(header, buffer_size, "HTTP/1.1 200 OK\r\n"
                                  "Content-type: %s\r\n"
                                  "\r\n",
                                  mime_type);   

    // search for requested file
    int file_fd = open(file_name, O_RDONLY);

    if (file_fd == -1){
        snprintf(header, buffer_size, "HTTP/1.1 404 Not Found\r\n"
                                      "Content-Type: text/html\r\n"
                                      "Content-Length: 58\r\n"
                                      "\r\n"
                                      "<html><body><h1>404 Not Found</h1></body></html>");
        *response_len = strlen(response);
        return;
    }

    // get file size
    fstat(file_fd, &file_stat);
    //off_t file_size = file_stat.st_size;

    // copy header
    *response_len = 0;
    memcpy(response, header, strlen(header));
    *response += strlen(header);

    printf("\nheader is : %s", response);
    fflush(stdout);

    // copy file
    ssize_t bytes_read;
    while((bytes_read = read(file_fd, response + *response_len, buffer_size - *response_len)) > 0){
        *response_len += bytes_read;
    }

    printf("\nresponse is : %s", response);
    fflush(stdout);
    free(header);
    close(file_fd);
}

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
            printf("\n%s", decoded_buffer);
            fflush(stdout);

            printf("\nhello\n");
            char *file_ext = get_file_ext(decoded_buffer);
            
            printf("\n%s", file_ext);
            printf("\nyeee\n");
            fflush(stdout);

            printf("AAAAA\n");
            char *file_name = get_file_name(buffer);
            printf("AAAAA\n");
            printf("\n%s", file_name);
            fflush(stdout);

            char *response = (char*)malloc(buffer_size*2*sizeof(char));
            size_t response_len;

            http_response(file_name, file_ext, response, &response_len);
            printf("\n%s", response);
            fflush(stdout);
            send(client_fd, response, response_len, 0);

            free(response);
            response = NULL;
            free(file_name);
            file_name = NULL;
        }
    }

    buffer = NULL;
    close(client_fd);
    pthread_mutex_unlock(&buffer_mutex);
    pthread_exit(NULL);
}
