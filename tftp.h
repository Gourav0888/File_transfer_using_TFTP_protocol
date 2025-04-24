/* Common file for server & client*/

#ifndef TFTP_H
#define TFTP_H


#include <stdint.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT                5000
#define SERVER_IP           "127.0.0.1"
#define TIMEOUT_SEC         5    // Timeout in seconds
#define TIMEOUT_U_SEC       0    // Timeout in micro seconds
#define SUCCESS             0
#define FAILURE             -1
#define NORMAL_MODE         0
#define OCTET_MODE          1
#define NETASCII_MODE       2

#define CONNECT             1
#define PUT_FILE            2
#define GET_FILE            3
#define CHANGE_MODE         4
#define EXIT                5

// TFTP OpCodes
typedef enum 
{
    RRQ = 1,  // Read Request
    WRQ = 2,  // Write Request
    DATA = 3, // Data Packet
    ACK = 4,  // Acknowledgment
    ERROR = 5 // Error Packet
} tftp_opcode;

// TFTP Packet Structure
typedef struct {
    uint16_t opcode; // Operation code (RRQ/WRQ/DATA/ACK/ERROR)
    union {
        struct {
            char filename[256];
            int mode;
        } request;  // RRQ and WRQ
        struct {
            // uint16_t block_number;
            unsigned long int block_number;
            int block_size;
            char data[512];
        } data_packet; // DATA
        struct {
            // uint16_t block_number;
            unsigned long int block_number;
            char ack_msg[256];                          
        } ack_packet; // ACK
        struct {
            uint16_t error_code;
            char error_msg[512];
        } error_packet; // ERROR
    } body;
} tftp_packet;

int send_file(int sockfd, struct sockaddr_in receiver_addr, tftp_packet  *sender_pac, char *filename, char mode);
int receive_file(int sockfd, struct sockaddr_in sender_addr, tftp_packet  *receiver_pac, int fd);


#endif // TFTP_H
