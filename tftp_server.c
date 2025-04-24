/*Server main file*/

#include "tftp.h"

void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet);

int main() 
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    tftp_packet server_packet;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd==-1)
    {
        printf("ERROR: Server socket creation failed\n");
        return 1;
    }

    // Set socket timeout option
    //TODO Use setsockopt() to set timeout option
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;  
    timeout.tv_usec = TIMEOUT_U_SEC;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);   

    // Bind the socket
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1)
    {
        perror("ERROR: Server bind failed\n");
        return 1;
    }

    printf("TFTP Server listening on port %d...\n", PORT);

    // Main loop to handle incoming requests
    while (1) 
    {
        // int n = 
        recvfrom(sockfd, &server_packet, sizeof(server_packet), 0, (struct sockaddr *)&client_addr, &client_len);
        // if (n < 0) 
        // {
        //     perror("Receive failed or timeout occurred");
        //     continue;
        // }

        handle_client(sockfd, client_addr, client_len, &server_packet);
    }

    close(sockfd);
    return 0;
}

void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet) 
{
    char filename[256]={0};
    int mode, fd;
    // Extract the TFTP operation (read or write) from the received packet
    // and call send_file or receive_file accordingly
    if(packet->opcode==WRQ)
    {
        printf("INFO: Write request received from client\n");
        strcpy(filename, packet->body.request.filename);
        mode = packet->body.request.mode;
        strcpy(filename, getenv("HOME"));
        strcat(filename, "/");
        strcat(filename, packet->body.request.filename);
        
        printf("INFO: Checking file exist or not\n");
        //check file exist or not (if exist sets errno)
        fd = open(filename, O_EXCL | O_CREAT | O_WRONLY, 0666);
        if(errno==EEXIST)
        {
            //erase data from existing file
            printf("INFO: Clearing existing file data\n");
            fd = open(filename, O_TRUNC | O_WRONLY);
        }
        else
        {
            printf("INFO: File does not exist creating new file\n");
        }
        if(fd==-1)
        {
            memset(packet, 0, sizeof(tftp_packet));
            packet->opcode = ERROR;
            packet->body.error_packet.error_code = 402;
            strcpy(packet->body.error_packet.error_msg, "File error");
            sendto(sockfd, packet, sizeof(tftp_packet), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            printf("ERROR: File error\n");
            return ;
        }
        memset(packet, 0, sizeof(tftp_packet));
        packet->opcode = ACK;
        // strcpy(packet->body.ack_packet.ack_msg, "Request accepted");
        memcpy(packet->body.ack_packet.ack_msg, "Request accepted", strlen("Request accepted")+1);

        sendto(sockfd, packet, sizeof(tftp_packet), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        printf("INFO: ACK send to client\n");
        sleep(1);

        if(receive_file(sockfd, client_addr, packet, fd)==FAILURE)
        {
            printf("ERROR: File not received\n");
            return ;
        }
        printf("INFO: File received successfully\n");
    }
    else if(packet->opcode==RRQ)
    {
        printf("INFO: Read request received from client\n");
        strcpy(filename, packet->body.request.filename);
        mode = packet->body.request.mode;
        strcpy(filename, getenv("HOME"));
        strcat(filename, "/");
        strcat(filename, packet->body.request.filename);
        
        printf("INFO: Checking file exist or not\n");
        //check file exist or not (if exist sets errno)
        fd = open(filename, O_RDONLY);
        if(fd==-1)
        {
            memset(packet, 0, sizeof(tftp_packet));
            packet->opcode = ERROR;
            packet->body.error_packet.error_code = 404;
            strcpy(packet->body.error_packet.error_msg, "File not found");
            sendto(sockfd, packet, sizeof(tftp_packet), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            printf("ERROR: File not found\n");
            return ;
        }
        close(fd);

        memset(packet, 0, sizeof(tftp_packet));
        packet->opcode = ACK;
        // strcpy(packet->body.ack_packet.ack_msg, "Request accepted, file found");
        memcpy(packet->body.ack_packet.ack_msg, "Request accepted, file found", strlen("Request accepted, file found")+1);

        sendto(sockfd, packet, sizeof(tftp_packet), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        printf("INFO: ACK send to client\n");
        sleep(1);
        if(send_file(sockfd, client_addr, packet, filename, mode)==FAILURE)
        {
            printf("ERROR: File not sent\n");
            return ;
        }
        printf("INFO: File sent successfully\n");
    }
}




