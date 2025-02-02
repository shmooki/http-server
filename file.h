#ifndef file_h
#define file_h
#include <pthread.h>
#include <ctype.h>

extern int client_count;
extern pthread_mutex_t log_mutex;

void openLogFile();
void closeLogFile(char* errorMessage);
void clientConnected(int client_socket);
void clientDisconnected(int client_socket);
void openBufferFile(char* buffer);
void openResponseFile(char *response);
void printHexDump(const char* buffer, ssize_t len);

#endif