
// file_server.cpp - file server in c++ 
#include <errno.h>
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <unistd.h> 

#define PORT 8080

int main(int argc, char const *argv[]) 
{
    /*** SOCKET PART ***/ 
    int server_fd, new_socket;
    struct sockaddr_in address; //though connect() wants a struct sockaddr*, you can still
  						                //use a struct sockaddr_in and cast it
    int opt = 1; 
    int addrlen = sizeof(address);


       
    // Creating socket file descriptor 
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  //IPv4 over TCP
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET;  //sockaddr_in to be used with IPv4 so we set it to AF_INET
    address.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY is used to accept all IPS
  								                       
    address.sin_port = htons( PORT ); //sin_port must be in Network Byte Order - by using htons()
       
    // Forcefully attaching socket to the port 8080, note the cast to sockaddr from sockaddr_in
    if(bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if(listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 




    /***FILE PART ***/
    const int BUFSIZE = 1024;
    char buffer[BUFSIZE] = {0};

    //get file size
    unsigned long long int file_size = 0;
    unsigned long long int *fs = &file_size;
    int s = read(new_socket, fs, sizeof(long long int));
    if(s < 0)
    {
        //handle errors
        printf("BYTES READ ERROR: %d", errno);
    }
    
    //read file
    FILE* output_file = fopen(argv[1], "w");

    int bytes_rec = 0;
    int bytes_write = 0;
    //read and write bit by bit until done
    while((bytes_rec = read(new_socket , buffer, BUFSIZE)) > 0)
    {
        if(bytes_rec < 0)
        {
        //handle errors
        printf("BYTES READ ERROR: %d", errno);
        }

        //change buffer size at te end of the file
        if(file_size < BUFSIZE)//better than bytes_rec < BUFSIZE?
        {
            char last_buffer[bytes_rec]= {0};
            fwrite(buffer, sizeof(char), sizeof(last_buffer), output_file);
            break;
        }
        bytes_write = fwrite(buffer, sizeof(char), sizeof(buffer), output_file);
        if (bytes_write <= 0)
            {
                //handle errors
                printf("BYTES WRITTEN ERROR: %d", errno);
            }

        file_size -= bytes_write;
        buffer[BUFSIZE] = {0};
    }
     
    fclose(output_file);
    close(new_socket);
    return 0; 
} 
