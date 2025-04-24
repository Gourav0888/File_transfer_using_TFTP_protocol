/*Client main file*/

#include "tftp.h"
#include "tftp_client.h"

tftp_packet client_pac;
int connect_flag;
int main() 
{
    tftp_client_t client;
    memset(&client, 0, sizeof(client));  // Initialize client structure
    
    //by default normal mode
    client_pac.body.request.mode = NORMAL_MODE;
    int choice;
    while (1)
    {
        printf("1. Connect\n2. Put\n3. Get\n4. Mode\n5. Exit\n");
        scanf("%d", &choice);
        switch (choice)
        {
            case CONNECT:
                connect_to_server(&client);
                break;

            case PUT_FILE:
                if(connect_flag)
                    put_file(&client);
                else
                {
                    printf("ERROR: Put failed, first connect to server\n");
                }
                break;            

            case GET_FILE:
                if(connect_flag)
                    get_file(&client);
                else
                {
                    printf("ERROR: Get failed, first connect to server\n");
                }
                break;            

            case CHANGE_MODE:
                change_mode();
                break;

            case EXIT:
                printf("Exiting...\n");
                return 0;
        
            default:
                printf("Invalid option\nTry again...\n");
                break;
        }
        
    }
    return 0;
}

// This function is to initialize socket with given server IP, no packets sent to server in this function
void connect_to_server(tftp_client_t *client) 
{
    // Create UDP socket
    client->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client->sockfd==-1)
    {
        perror("ERROR: Client socket creation failed\n");
        return ;
    }
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;  // Wait up to 5 seconds
    timeout.tv_usec = TIMEOUT_U_SEC;
    setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    printf("INFO: Client socket created\n");
    client->server_addr.sin_family = AF_INET;
    printf("Enter server information: \n");
    printf("Enter IP address: ");
    char ip[20];
    scanf("%s", ip);
    client->server_addr.sin_addr.s_addr = inet_addr(ip);

    printf("Enter PORT number: ");
    int port;
    scanf("%d", &port);
    client->server_addr.sin_port = htons(port);

    //validate port and ip address
    //validate port
    if(!(port>1023 && port<49152))
    {
        printf("ERROR: Invalid port number\n");
        return ;
    }

    //validate ip address
    if(strlen(ip)>6)
    {
        int i=0, dot=0;
        while (ip[i])
        {
            if(ip[i]=='.')
                dot++;
            i++;
        }
        if(dot!=3)
        {
            printf("ERROR: Invalid IP address\n");
            return ;
        }
        int range[4];
        sscanf(ip, "%d.%d.%d.%d", &range[0], &range[1], &range[2], &range[3]);
        for (int i = 0; i < 4; i++)
        {
            if(!(range[i]>=0 && range[i]<=255))
            {
                printf("ERROR: Invalid IP address\n");
                return ;
            }
        }
        
    }
    else
    {
        printf("ERROR: Invalid IP address\n");
        return ;
    }
    
    client->server_len = sizeof(client->server_addr);
    // printf("server info\n");
    // printf("server port: %d\n", ntohs(client->server_addr.sin_port));
    // printf("server ip: %s\n", inet_ntoa(client->server_addr.sin_addr));
    // printf("len = %u\n", client->server_len);
    printf("INFO: Connection successfull\n");
    connect_flag = 1;
}

void put_file(tftp_client_t *client)
{
    // Send WRQ request and send file

    //taking file from user
    char file[256];
    printf("Enter file name you want to upload or put to server: ");
    scanf(" %[^\n]", file);

    int fd = open(file, O_RDONLY);
    if(fd==-1)
    {
        printf("Unable to open file or file not available\n");
        return;
    }
    close(fd);
    //sending write requuest to server
    client_pac.opcode = WRQ;
    strcpy(client_pac.body.request.filename, file);
    int mode = client_pac.body.request.mode;
    
    // printf("server info\n");
    // printf("server port: %d\n", ntohs(client->server_addr.sin_port));
    // printf("server ip: %s\n", inet_ntoa(client->server_addr.sin_addr));
    // printf("len = %u\n", client->server_len);
    int ret = sendto(client->sockfd, &client_pac, sizeof(client_pac), 0, (struct sockaddr*)&client->server_addr, client->server_len);

    if(ret==-1)
    {
        printf("ERROR: Request sending failed\n");
        return ;
    }
    //receive acknowledgment
    printf("INFO: Waitng for server to accept put request\n");
    memset(&client_pac, 0, sizeof(tftp_packet));
    ret = recvfrom(client->sockfd, &client_pac, sizeof(client_pac), 0, (struct sockaddr*)&client->server_addr, &client->server_len);
    sleep(1);

    if(ret<0)
    {
        printf("ERROR: Timeout or wrong server credentials\nTry again...\n");
        return ;
    }
    //check request is accepted 
    if(client_pac.opcode == ACK)
    {
        // printf("INFO: Request accepted\n");
        printf("INFO: %s\n", client_pac.body.ack_packet.ack_msg);
    }
    else
    {
        printf("ERROR %d: %s\n", client_pac.body.error_packet.error_code, client_pac.body.error_packet.error_msg);
        return ;
    }

    //if accepted upload file
    printf("INFO: Uploading file to server\n");
    sleep(1);
    if(send_file(client->sockfd, client->server_addr, &client_pac, file, mode) == FAILURE)
    {
        printf("ERROR: File not sent\n");
        return ;
    }
    printf("INFO: File uploaded successfully\n");
}
 
void get_file(tftp_client_t *client) 
{
    // Send RRQ and recive file 
    
    //taking file from user
    char file[256];
    printf("Enter file name you want to download or get from server: ");
    scanf(" %[^\n]", file);

    //sending read requuest to server
    client_pac.opcode = RRQ;
    strcpy(client_pac.body.request.filename, file);
    int mode = client_pac.body.request.mode;
    
    // printf("Server info\n");
    // printf("Server port: %d\n", ntohs(client->server_addr.sin_port));
    // printf("Server ip: %s\n", inet_ntoa(client->server_addr.sin_addr));
    // printf("len = %u\n", client->server_len);
    int ret = sendto(client->sockfd, &client_pac, sizeof(client_pac), 0, (struct sockaddr*)&client->server_addr, client->server_len);

    if(ret==-1)
    {
        printf("ERROR: Request sending failed\n");
        return ;
    }
    //receive acknowledgment
    printf("INFO: Waitng for server to accept get request\n");
    ret = recvfrom(client->sockfd, &client_pac, sizeof(client_pac), 0, (struct sockaddr*)&client->server_addr, &client->server_len);
    sleep(1);

    if(ret<0)
    {
        printf("ERROR: Timeout or wrong server credentials\nTry again...\n");
        return ;
    }

    //check request is accepted 
    if(client_pac.opcode == ACK)
    {
        // printf("INFO: Request accepted\n");
        printf("INFO: %s\n", client_pac.body.ack_packet.ack_msg);
    }
    else
    {
        printf("ERROR %d: %s\n", client_pac.body.error_packet.error_code, client_pac.body.error_packet.error_msg);
        return ;
    }

    int fd ;
    printf("INFI: Creating file\n");
    fd = open(file, O_EXCL | O_CREAT | O_WRONLY, 0666);
    if(errno==EEXIST)
    {
        //erase data from existing file
        printf("INFO: File already exist, clearing existing file data\n");
        fd = open(file, O_TRUNC | O_WRONLY);
    }
    else
    {
        printf("INFO: File does not exist creating new file\n");
    }
    printf("INFO: Downloading file from server\n");
    if(receive_file(client->sockfd, client->server_addr, &client_pac, fd)==FAILURE)
    {
        printf("ERROR: File downloading failed\n");
        return ;
    }
    printf("INFO: File downloaded successfully\n");

}

void change_mode()
{
    printf("1. Normal\n2. Octet\n3. Netascii\n");
    int choice;
    scanf("%d", &choice);

    switch (choice)
    {
        case 1:
            client_pac.body.request.mode = NORMAL_MODE;
            break;
        
        case 2:
            client_pac.body.request.mode = OCTET_MODE;
            break;

        case 3:
            client_pac.body.request.mode = NETASCII_MODE;
            break;    

        default:
            printf("ERROR: Invalid option\n");
            break;
    }
}