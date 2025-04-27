/*#----------------------------------------------------------------------------------------------------------------#
# Filename : guessThePinclient.c#
# Author : Dolor, Llagas, Mien, Raposa, Valencia#
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
    
    int server_sock, client_sock,  port_no,  n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[256], clientPin[3], serverPin[3], guess[3], result[256];
    int serverGuessCount = 0, clientGuessCount = 0, gameOver = 0;
    
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
    
    // Get client pin
    printf("Enter a 3 digit pin\n> ");
    for (int i = 0; i < 3; i++) {
        scanf(" %c", &clientPin[i]);
    }
    printf("Client PIN set to: %c %c %c\n", clientPin[0], clientPin[1], clientPin[2]);
  
    // Send client pin to server
    n = send(client_sock, clientPin, 3, 0);
    if (n < 0) die_with_error("Error: send() Failed (sending pin).");
    
    // Receive server pin
    n = recv(client_sock, serverPin, 3, 0);
    if (n < 0) die_with_error("Error: recv() Failed (receiving pin).");
    
    printf("Server PIN received.\n");

    // Turn-based game loop
    while (!gameOver) {
        // Wait for server to send guess
        printf("\n[Turn] Waiting for server's guess...\n");
        bzero(guess, 3);
        n = recv(client_sock, guess, 3, 0);
        if (n < 0) die_with_error("Error: recv() Failed (server guess).");

        serverGuessCount++;

        // Check server's guess
        int correct_place = 0, correct_digit = 0;
        for (int i = 0; i < 3; i++) {
            if (guess[i] == clientPin[i]) {
                correct_place++;
            } else {
                for (int j = 0; j < 3; j++) {
                    if (i != j && guess[i] == clientPin[j]) {
                        correct_digit++;
                    }
                }
            }
        }

        bzero(result, 256);
        if (correct_place == 3) {
            sprintf(result, "Correct! You guessed it!");
            gameOver = 1;
            printf("Server wins in %d guess(es)!\n", serverGuessCount);
        } else {
            sprintf(result, "%d correct in place, %d correct but wrong place.", correct_place, correct_digit);
        }

        // Send result to server
        n = send(client_sock, result, strlen(result), 0);
        if (n < 0) die_with_error("Error: send() Failed (feedback to server).");

        if (gameOver) break;

        // Client turn to guess
        printf("\n[Turn] Your turn to guess the server's PIN!\n");
        printf("Enter your guess (3 digits):\n> ");
        for (int i = 0; i < 3; i++) {
            scanf(" %c", &guess[i]);
        }

        clientGuessCount++;

        // Send guess to server
        n = send(client_sock, guess, 3, 0);
        if (n < 0) die_with_error("Error: send() Failed (client guess).");

        // Receive result
        bzero(result, 256);
        n = recv(client_sock, result, 255, 0);
        if (n < 0) die_with_error("Error: recv() Failed (result from server).");

        printf("[server] > %s\n", result);

        if (strncmp(result, "Correct", 7) == 0) {
            printf("You win in %d guess(es)!\n", clientGuessCount);
            gameOver = 1;
            break;
        }
    }

    
    printf("Closing connection ...\n");
    close(client_sock);
    
    return 0;
}
