/*#----------------------------------------------------------------------------------------------------------------#
# Filename : 202300446_le9_client.c#
# Author : olor, Llagas, Mien, Raposa, Valencia#
# Last Modified : April 3, 2025 #
# Description : A proposed Guess The Pin project #
# Honor Code : This is my own program. I have not received any #
# unauthorized help in completing this work. I have not #
# copied from my classmate, friend, nor any unauthorized #
# resource. I am well aware of the policies stipulated #
# in the handbook regarding academic dishonesty. #
# If proven guilty, I won't be credited any points for #
# this exercise. #
#-------------------------------------------------------------------------------------------------------------#*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}

int main(int argc,  char *argv[]){
    
    int client_sock,  port_no,  n;
    struct sockaddr_in server_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
        printf("Usage: %s hostname port_no",  argv[0]);
        exit(1);
    }

    printf("Client starting ...\n");
    // Create a socket using TCP
    client_sock = socket(AF_INET,  SOCK_STREAM,  0);
    if (client_sock < 0) 
        die_with_error("Error: socket() Failed.");

    printf("Looking for host '%s'...\n", argv[1]);
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        die_with_error("Error: No such host.");
    }
    printf("Host found!\n");

    // Establish a connection to server
    port_no = atoi(argv[2]);
    bzero((char *) &server_addr,  sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,  
         (char *)&server_addr.sin_addr.s_addr, 
         server->h_length);
         
    server_addr.sin_port = htons(port_no);

    printf("Connecting to server at port %d...\n", port_no);
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        die_with_error("Error: connect() Failed.");

    printf("Connection successful!\n");

    // Communicate
    while (1) {
        // Receive the message from the server
        bzero(buffer, 256);
        n = recv(client_sock, buffer, 255, 0);
        if (n < 0) die_with_error("Error: recv() Failed.");
        
        printf("[server] > %s", buffer);
        
        // End communication if server exited
        if (strncmp(buffer, "exit", 4) == 0) break;
        
        // Client sends a message
        printf("< ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        
        n = send(client_sock, buffer, strlen(buffer), 0);
        if (n < 0) die_with_error("Error: send() Failed.");
        
        
        // End communication if client exited
        if (strncmp(buffer, "exit", 4) == 0) break;
    }

    printf("Closing connection ...\n");
    close(client_sock);
    
    return 0;
}
