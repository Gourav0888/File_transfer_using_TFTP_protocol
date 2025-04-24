/* Common file for server & client */

#include "tftp.h"

int send_file(int sockfd, struct sockaddr_in receiver_addr, tftp_packet *sender_pac, char *filename, char mode) 
{
    socklen_t addr_len = sizeof(receiver_addr);
    // Implement file sending logic here
    int index=0, fd, ack_flag=0;
    unsigned long int block_number=0;
    fd = open(filename, O_RDONLY);
    char ch, data_duffer[512];
    while (read(fd, &ch, 1)>0)
    {
        if(mode==NETASCII_MODE && ch=='\n')
        {
            data_duffer[index++] = '\r';
            data_duffer[index++] = '\n';
        }

        if(mode!=OCTET_MODE)    
            data_duffer[index++] = ch;

        //sending byte by byte
        if (mode==OCTET_MODE)
        {
            //clear data buffer
            memset(sender_pac->body.data_packet.data, 0, 512);
            block_number++;
            //send data pac
            sender_pac->opcode = DATA;
            sender_pac->body.data_packet.block_number = block_number;
            sender_pac->body.data_packet.data[0] = ch; 
            sender_pac->body.data_packet.block_size = 1;
            sendto(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));     
            printf("INFO: Data sent with block number %lu\t", block_number);
            
            ack_flag = 1;
        }
        //sending 512 bytes per packet (normal mode or netascii mode)
        else if(index==512)
        {
            //clear data buffer
            memset(sender_pac->body.data_packet.data, 0, 512);
            block_number++;
            //send data pac
            sender_pac->opcode = DATA;
            sender_pac->body.data_packet.block_number = block_number;
            sender_pac->body.data_packet.block_size = index;
            strcpy(sender_pac->body.data_packet.data, data_duffer);
            sendto(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
            printf("INFO: Data sent with block number %lu | Bytes: %d\t", block_number, index);
            ack_flag = 1; 
            //reset index for next packet
            index = 0;
        }

        //receive acknowledment
        if(ack_flag)
        {
            recvfrom(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, &addr_len);
            if(sender_pac->opcode == ACK)
            {
                // printf("INFO: Acknowledment received for block number %d\n", block_number);
                printf("ACK: %s %lu\n",sender_pac->body.ack_packet.ack_msg, sender_pac->body.ack_packet.block_number);
            }
            else
            {
                printf("ERROR %d: %s\n", sender_pac->body.error_packet.error_code, sender_pac->body.error_packet.error_msg);
                printf("INFO: Resending data for block number %lu\n", block_number);
                block_number--;
                //going backward to fetch last block data again
                if(mode==NORMAL_MODE || mode == NETASCII_MODE)
                    lseek(fd, -512, SEEK_CUR);   
                else 
                    lseek(fd, -1, SEEK_CUR);
            }
            ack_flag = 0;
        }

    }
    // Send remaining data if any (last partial block)
    if (index > 0) 
    {
        memset(sender_pac->body.data_packet.data, 0, 512);
        block_number++;
        sender_pac->opcode = DATA;
        sender_pac->body.data_packet.block_number = block_number;
        memcpy(sender_pac->body.data_packet.data, data_duffer, index); 
        sender_pac->body.data_packet.block_size = index;
        sendto(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
        printf("INFO: Final block sent with block number %lu | Bytes: %d\t", block_number, index);

        // Receive ACK
        recvfrom(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, &addr_len);
        if (sender_pac->opcode != ACK)  
        {
            printf("ERROR: Final ACK not received\n");
            return FAILURE;
        }
        printf("ACK: %s %lu\n",sender_pac->body.ack_packet.ack_msg, sender_pac->body.ack_packet.block_number);
    }
    
    while (1)
    { 
        //acknowledment for complete file sent
        sender_pac->opcode = ACK;
        strcpy(sender_pac->body.ack_packet.ack_msg, "Complete file transferred");
        sendto(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));

        //receive ack for file sent
        recvfrom(sockfd, sender_pac, sizeof(tftp_packet), 0, (struct sockaddr*)&receiver_addr, &addr_len);
        if (sender_pac->opcode==ACK)
        {
            close(fd);
            return SUCCESS;
        }
    }
}
int receive_file(int sockfd, struct sockaddr_in sender_addr, tftp_packet *receiver_pac, int fd)
{
    socklen_t addr_len = sizeof(sender_addr);
    unsigned long int block_number = 0;

    while (1)
    {
        memset(receiver_pac, 0, sizeof(tftp_packet));

        int ret_value = recvfrom(sockfd, receiver_pac, sizeof(tftp_packet), 0, (struct sockaddr *)&sender_addr, &addr_len);

        if (receiver_pac->opcode == ACK)
        {
            sendto(sockfd, receiver_pac, sizeof(tftp_packet), 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr));
            close(fd);
            return SUCCESS;
        }

        if (receiver_pac->opcode == DATA)
        {
            block_number = receiver_pac->body.data_packet.block_number;

            int header_size = sizeof(uint16_t) + sizeof(unsigned long int) + sizeof(int) + 10 /*padding bytes*/;
            int packet_len = ret_value - header_size;
            int data_size = receiver_pac->body.data_packet.block_size;
            // printf("header size = %d\npacket_len=%d\nret_value=%d\n", header_size, packet_len, ret_value);
            // printf("data size = %lu\n", strlen(receiver_pac->body.data_packet.data));
            printf("INFO: Data received with block number %lu | Bytes: %d\n", block_number, data_size);

            if (packet_len > 0)
            {
                if (packet_len < 512)
                {
                    // Save the data
                    write(fd, receiver_pac->body.data_packet.data, data_size);

                    printf("INFO: Final short block received. File transfer complete.\n");

                    // Send ACK for this block
                    receiver_pac->opcode = ACK;
                    receiver_pac->body.ack_packet.block_number = block_number;
                    strcpy(receiver_pac->body.ack_packet.ack_msg, "Acknowledgment for final block");
                    sendto(sockfd, receiver_pac, sizeof(tftp_packet), 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr));

                    close(fd);
                    return SUCCESS;
                }
                else if (packet_len == 512)
                {
                    write(fd, receiver_pac->body.data_packet.data, data_size);

                    receiver_pac->opcode = ACK;
                    receiver_pac->body.ack_packet.block_number = block_number;
                    strcpy(receiver_pac->body.ack_packet.ack_msg, "Acknowledgment for block number");
                    sendto(sockfd, receiver_pac, sizeof(tftp_packet), 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr));
                }
            }
            else
            {
                printf("INFO: Zero-length data block received. Terminating transfer.\n");
                close(fd);
                return SUCCESS;
            }
        }
    }
}
