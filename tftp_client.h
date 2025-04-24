/*Client header file*/

#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include "tftp.h"

typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
} tftp_client_t;

// Function prototypes
void connect_to_server(tftp_client_t *client);
void put_file(tftp_client_t *client);
void get_file(tftp_client_t *client);
void change_mode();

// void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);
// void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);

#endif