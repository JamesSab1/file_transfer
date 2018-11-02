// file_client.cpp - file client in c++ to send files
#include "file_transfer.h"

//sendfile another option sendfile(socket, input_file, NULL, BUFSIZ)) although this is less portable.
//If you keep the buffer less than say 16K it will likely stay in L1 cache and remain quick.
//read n bytes from a descriptor

int send_file(int socket, const char* filename)
{
    FILE *input_file = fopen(filename, "r");
    if (input_file == NULL)
    {
        printf("No file\n");
        return -1;
    }
    int fd = fileno(input_file);

    fseek(input_file, 0, SEEK_END);
    unsigned long long file_size = ftell(input_file);
    rewind(input_file);
    long long *fs = &file_size;
    write(socket, fs, sizeof(long long));

    while (file_size > 0) 
    {
        int bytes_read = readn(fd, buffer, BUFSIZE);
        writen(socket, buffer, BUFSIZE);
        file_size -= bytes_read;
    }
    fclose(input_file);
    return 0;
}



int client_connect(const char* filename, const char* ip)
{
    /*** SOCKET PART ***/ 
    struct sockaddr_in address; //though connect() wants a struct sockaddr*, you can still
                                //use a struct sockaddr_in and cast it at the last minute
    int sock = 0, bytes_sent; 
    struct sockaddr_in serv_addr;


    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); // make sure the struct is empty
   
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(PORT); //host to network conversion
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/Address not supported \n"); 
        return -1; 
    } 
   
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //note cast to sockaddr
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }

    if(send_file(sock, filename) == -1)
    {
        //do some error handling
        printf("Error sending file\n");
        return -1;
    }
    close(sock);
    return 0;
}

   
int main(int argc, char const *argv[]) 
{
    if(argc != 3)
    {
        printf("Usage: ./file_client filename ip_address\n");
        return -1;
    }
    if(client_connect(argv[1], argv[2]) == -1)
    {
        printf("File not sent\n");
        return -1;
    }
    printf("%s sent successfully to %s\n", argv[1], argv[2]);
    return 0;
} 
