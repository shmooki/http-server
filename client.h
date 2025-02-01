#ifndef client_h
#define client_h

void *handle_client(void* arg);
char *url_decode(const char* url);
char *get_file_ext(const char* request);

#endif