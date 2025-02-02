#ifndef client_h
#define client_h

void *handle_client(void* arg);
char *url_decode(const char *url);
char *get_file_ext(const char *request);
char *get_mime_type(const char *file_ext);
void http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len);

#endif