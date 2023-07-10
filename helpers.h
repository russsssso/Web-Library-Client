#ifndef _HELPERS_
#define _HELPERS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFLEN 4096
#define LINELEN 1000

typedef struct {
    char *data;
    size_t size;
} buffer;

// initializes a buffer
buffer buffer_init(void);

// destroys a buffer
void buffer_destroy(buffer *buffer);

// adds data of size data_size to a buffer
void buffer_add(buffer *buffer, const char *data, size_t data_size);

// checks if a buffer is empty
int buffer_is_empty(buffer *buffer);

// finds data of size data_size in a buffer and returns its position
int buffer_find(buffer *buffer, const char *data, size_t data_size);

// finds data of size data_size in a buffer in a
// case-insensitive fashion and returns its position
int buffer_find_insensitive(buffer *buffer, const char *data, size_t data_size);

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(const char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(const char *host, const char *url, const char *query_params,
							const char **cookies, int cookies_count, const char *jwt_token);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(const char *host, const char *url, const char* content_type, const char *body_data,
							int body_data_fields_count, const char** cookies, int cookies_count, const char *jwt_token);

// computes and returns a DELETE request string (cookies can be NULL if not needed)
char *compute_delete_request(const char *host, const char *url, const char *query_params,
                            const char **cookies, int cookies_count, const char *jwt_token);

#endif
