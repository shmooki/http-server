#ifndef file_h
#define file_h
#include <pthread.h>

extern int client_count;
extern pthread_mutex_t log_mutex;

void openLogFile();
void closeLogFile();
void clientConnected(int client_socket);
void clientDisconnected(int client_socket);

#endif