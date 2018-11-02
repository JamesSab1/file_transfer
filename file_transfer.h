#include <unistd.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <errno.h>

const int BUFSIZE = 1024;
char buffer[BUFSIZE] = {0};
#define PORT 8080


ssize_t readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    void *ptr;

    ptr = vptr;
    nleft = n;
    while(nleft > 0)
    {
        if((nread = read(fd, ptr, nleft)) < 0)
        {
            if(errno == EINTR)
                nread = 0; //call again
            else
                return -1;
        }
        else if(nread == 0)
            break; //EOF
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft); // >=0
}


//write n bytes to a descriptor
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const void *ptr;

    ptr = vptr;
    nleft = n;
    while(nleft > 0)
    {
        if((nwritten = write(fd, ptr, nleft)) <=0)
        {
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0; // call again
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}


//read file size sent from file_client
long long get_file_size(int socket)
{
    long long file_size = 0;
    long long *fs = &file_size;
    int s = read(socket, fs, sizeof(long long));
    if(s < 0)
    {
        if(errno == EINTR)
            s = 0;
        else
        {
            printf("BYTES READ ERROR: %d", errno);
            return -1;        
        }
    }
    return file_size;
}
