
// file_client.cpp - file client in c++ to send files
#include <unistd.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <string>
#include <fstream>
#include <sys/sendfile.h>
#include <errno.h>

#define PORT 8080 
   
int main(int argc, char const *argv[]) 
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
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //note cast to sockaddr
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 


	/*** FILE PART ***/

    const int BUFSIZE = 1024;
    char buffer[BUFSIZE] = {0};

    FILE *input_file = fopen(argv[1], "r");

    if (input_file == NULL)
    {
        printf("No file\n");
        exit(1);
    }

    //get file size and send to server
    fseek(input_file, 0, SEEK_END);
    long long int file_size = ftell(input_file);
    rewind(input_file);
    long long int *fs = &file_size;
    write(sock, fs, sizeof(long long int));

    while (1) 
    {
        // Read data into buffer nd store amount read in bytes_read
        int bytes_read = fread(buffer, sizeof(char), sizeof(buffer), input_file);
        if (bytes_read == 0) //file read complete
            break;

        if (bytes_read < 0)
        {
            // handle errors
            printf("BYTES READ ERROR: %d", errno);
        }

        //You need a loop for the write, because not all of the data may be written
        //in one call; write will return how many bytes were written. p keeps
        //track of where in the buffer we are, while we decrement bytes_read
        //to keep track of how many bytes are left to write.
        void *p = buffer;
        while (bytes_read > 0) 
        {
            int bytes_written = write(sock, p, bytes_read);
            if (bytes_written <= 0)
            {
                //handle errors
                printf("BYTES WRITTEN ERROR: %d", errno);
            }
            bytes_read -= bytes_written;
            p += bytes_written;
            
        }
    }

    fclose(input_file);
    close(sock);


    /* sendfile another option sendfile(socket, input_file, NULL, BUFSIZ)) although this is less portable.
       If you keep the buffer less than say 16K it will likely stay in L1 cache and remain quick.
    */

    return 0; 
} 
