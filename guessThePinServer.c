/*----------------------------------------------------------------------------------------------------------------#
# Filename : guessThePinServer.c#
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
#-------------------------------------------------------------------------------------------------------------# */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}

int main(int argc, char *argv[]){
    int server_sock, client_sock, port_no, client_size, n;
    char buffer[256], serverPin[3], clientPin[3], guess[3], result[256];
    struct sockaddr_in server_addr, client_addr;
    int serverGuessCount = 0, clientGuessCount = 0, gameOver = 0;
    
    
    if (argc < 2) {
        printf("Usage: %s port_no", argv[0]);
        exit(1);
    }

    printf("Server starting ...\n");
    // Create a socket for incoming connections
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (server_sock < 0) 
       die_with_error("Error: socket() Failed.");
       
    // Bind socket to a port
    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET; // Internet address 
    server_addr.sin_addr.s_addr = INADDR_ANY; // Any incoming interface
    server_addr.sin_port = htons(port_no); // Local port
    
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
       die_with_error("Error: bind() Failed.");
       
    // Mark the socket so it will listen for incoming connections
    listen(server_sock, 5);
    printf("Server listening to port %d ...\n", port_no);
    
    printf("Waiting for connection(s) ...\n");

    // Accept new connection
    client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    
    if (client_sock < 0) 
        die_with_error("Error: accept() Failed.");

    printf("Client succesfully connected ...\n"); 
    
    // Get server pin
    printf("Enter a 3 digit pin\n> ");
    for (int i = 0; i < 3; i++) {
        scanf(" %c", &serverPin[i]);
    }
    printf("Server PIN set to: %c %c %c\n", serverPin[0], serverPin[1], serverPin[2]);

    // Receive client pin
    n = recv(client_sock, buffer, 3, 0);
    if (n < 0) die_with_error("Error: recv() Failed (receiving pin).");

    for (int i = 0; i < 3; i++) {
        clientPin[i] = buffer[i];
    }
 
    printf("Client PIN received.\n");
    
    // Send server pin to client
    n = send(client_sock, serverPin, 3, 0);
    if (n < 0) die_with_error("Error: send() Failed (sending pin).");

    // Turn-based game loop
    while (!gameOver) {
        // Server turn to guess
        printf("\n[Turn] Server's turn to guess the client's PIN\n");
        printf("Enter your guess (3 digits):\n> ");
        for (int i = 0; i < 3; i++) {
            scanf(" %c", &guess[i]);
        }

        serverGuessCount++;

        // Send guess to client
        n = send(client_sock, guess, 3, 0);
        if (n < 0) die_with_error("Error: send() Failed (server guessing).");

        // Receive result from client
        bzero(result, 256);
        n = recv(client_sock, result, 255, 0);
        if (n < 0) die_with_error("Error: recv() Failed (result from client).");

        printf("[client] > %s\n", result);
        if (strncmp(result, "Correct", 7) == 0) {
            printf("Server wins in %d guess(es)!\n", serverGuessCount);
            gameOver = 1;
            break;
        }

        // Wait for client to send guess
        printf("\n[Turn] Waiting for client's guess...\n");
        bzero(guess, 3);
        n = recv(client_sock, guess, 3, 0);
        if (n < 0) die_with_error("Error: recv() Failed (client guess).");

        clientGuessCount++;

        // Check client's guess
        int correct_place = 0, correct_digit = 0;
        for (int i = 0; i < 3; i++) {
            if (guess[i] == serverPin[i]) {
                correct_place++;
            } else {
                for (int j = 0; j < 3; j++) {
                    if (i != j && guess[i] == serverPin[j]) {
                        correct_digit++;
                    }
                }
            }
        }

        bzero(result, 256);
        if (correct_place == 3) {
            sprintf(result, "Correct! You guessed it!");
            gameOver = 1;
            printf("Client wins in %d guess(es)!\n", clientGuessCount);
        } else {
            sprintf(result, "%d correct in place, %d correct but wrong place.", correct_place, correct_digit);
        }

        // Send result to client
        n = send(client_sock, result, strlen(result), 0);
        if (n < 0) die_with_error("Error: send() Failed (feedback to client).");
    }
    
    printf("Closing connection ...\n");
    close(client_sock);
    close(server_sock);
    
    return 0; 
}

