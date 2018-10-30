
// file_server.cpp - file server in c++ 
#include <errno.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <unistd.h> 
#include "file_transfer.h"


int write_file(const char* filename, int socket)
{
    //get file size
    unsigned long long int file_size = 0;
    unsigned long long int *fs = &file_size;
    int s = read(socket, fs, sizeof(long long int));
    if(s == -1 && errno != EINTR) //errno == EINTR will retry?
    {
        printf("BYTES READ ERROR: %d", errno);
        return -1;        
    }

    FILE* output_file = fopen(filename, "w");

    ssize_t bytes_rec = 0;
    size_t bytes_write = 0;
    //read and write bit by bit until done
    while((bytes_rec = read(socket , buffer, BUFSIZE)) > 0)
    {
        if(bytes_rec == -1 && errno != EINTR)
        {
            //handle errors
            printf("BYTES READ ERROR: %d", errno);
            return -1;        
        }

        //change buffer size at te end of the file
        if(file_size < BUFSIZE)//better than bytes_rec < BUFSIZE?
        {
            char last_buffer[bytes_rec]= {0};
            bytes_write = fwrite(buffer, sizeof(char), sizeof(last_buffer), output_file);
            if (bytes_write == -1 && errno != EINTR)
            {
                //handle errors
                printf("BYTES WRITTEN ERROR: %d", errno);
                return -1;
                
            }
            break;
        }
        bytes_write = fwrite(buffer, sizeof(char), sizeof(buffer), output_file);
        if (bytes_write == -1 && errno != EINTR)
            {
                //handle errors
                printf("BYTES WRITTEN ERROR: %d", errno);
                return -1;
                
            }

        file_size -= bytes_write;
    }
     
    fclose(output_file);
    return 0;
}


int server_listen(const char* filename)
{
    int server_fd, new_socket;
    struct sockaddr_in address; //though connect() wants a struct sockaddr*, you can still
                                        //use a struct sockaddr_in and cast it
    int opt = 1; 
    int addrlen = sizeof(address);


       
    // Creating socket file descriptor 
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  //IPv4 over TCP
    { 
        perror("socket failed"); 
        return -1; 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        return -1; 
    } 
    address.sin_family = AF_INET;  //sockaddr_in to be used with IPv4 so we set it to AF_INET
    address.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY is used to accept all IPS                                                   
    address.sin_port = htons(PORT); //sin_port must be in Network Byte Order - by using htons()
       
    // Attaching socket to PORT found in header, note the cast to sockaddr from sockaddr_in
    if(bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        return -1; 
    } 
    if(listen(server_fd, 3) < 0) 
    { 
        perror("listen failed"); 
        return -1; 
    } 
    if((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept failed"); 
        return -1; 
    }

    if(write_file(filename, new_socket) < 0)
    {
        //do some error checking
        printf("Error writing file, closing socket\n");
        return -1;
    }

    close(new_socket);

    return 0;
}



int main(int argc, char const *argv[]) 
{
    if(argc != 2)
    {
        printf("Usage: ./file_server filename");
        return -1;
    }

    if(server_listen(argv[1]) == -1)
        exit(EXIT_FAILURE);
    
    printf("%s write successful\n", argv[1]);
    return 0; 
} 
