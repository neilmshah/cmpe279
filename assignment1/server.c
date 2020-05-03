// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pwd.h>
#include <sys/types.h>

#define PORT 8080 

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    char *hello = "Hello from server"; 
       
    // First part - Setting up the socket

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    //This is where the split occurs, this is the second part where we listen for data and process info from the client
    //Here we should have a new user where the process forks and privileges are dropped

    //Create a new user 'guest' that will run in a new forked process with no root privileges
    char *guestUser = "nobody";
    struct passwd *pw;

    //Try to fetch user info
    if( (pw= getpwnam(guestUser)) == NULL ) {
        printf("Failed to fetch user information\n");
        exit(1);
    }

    //Get ID of the user
    pid_t guestUserID = pw->pw_uid;
    printf("Guest user ID = %ld \n", (long) guestUserID);

    //Fork a new child process and get pID
    pid_t pID = fork();

    // For pID=0, child process will be executed, for pID !=0, parent process will be executed
    if (pID==0) {
        //This is the child pricess
        printf("In Child process, Attempting to drop privileges\n");

        if( setuid(guestUserID) != 0) {
            printf("Error. Unable to drop privileges. Exiting \n");
            exit(1);
        } 
        else {
            printf("Successfully dropped privileges! Listening and processing connection from client. \n");
        }

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 
        valread = read( new_socket , buffer, 1024); 
        printf("%s\n",buffer ); 
        send(new_socket , hello , strlen(hello) , 0 ); 
        printf("Hello message sent\n"); 
    } 
    else {
        //This is the parent process
        //printf("In parent process, my pID is = %d \n", getpid());
    }

    //Wait for child process to complete before exiting
    pid_t wpid;
    int status = 0;
    while ((wpid = wait(&status)) > 0);

    return 0; 
} 
