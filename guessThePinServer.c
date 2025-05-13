/*----------------------------------------------------------------------------------------------------------------#
# Filename : guessThePinServer.c#
# Author : Dolor, Llagas, Mien, Raposa, Valencia#
# Last Modified : April 27, 2025 #
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
#include <time.h> // for timer functionality
#include <stdbool.h>


void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}

int main(int argc, char *argv[]){
    int server_sock, client_sock, port_no, client_size, n;
    char buffer[256], serverPin[3], clientPin[3], guess[3], result[256];
    struct sockaddr_in server_addr, client_addr;
    int serverGuessCount = 0, clientGuessCount = 0, gameOver = 0;
    int serverWins = 0, clientWins = 0;
    
    
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
    
    int difficulty = 1;
    int pin_lengths[] = {3, 4, 5};
    char level_names[][7] = {"Easy", "Medium", "Hard"};
    
    send(client_sock, &difficulty, sizeof(int), 0); 
    
    while (true) {
        int pin_length = pin_lengths[difficulty-1];
        char serverPin[5], clientPin[5], guess[5], result[256], replay;
        int serverGuessCount = 0, clientGuessCount = 0;
        int gameOver = 0;
        
        printf("\n=== Level %d: %s (%d pins) ===\n", difficulty, level_names[difficulty-1], pin_length);
        
        printf("Enter your %d-digit PIN: ", pin_length);
        for (int i = 0; i < pin_length; i++) {
            scanf(" %c", &serverPin[i]);
        }
        
        printf("Your PIN set to: ");
        for (int i = 0; i < pin_length; i++) printf("%c ", serverPin[i]);
        printf("\n");
        
        n = recv(client_sock, clientPin, pin_length, 0);
        if (n < 0) die_with_error("Error: recv() Failed (receiving pin).");
     
        n = send(client_sock, serverPin, pin_length, 0);
        if (n < 0) die_with_error("Error: send() Failed (sending pin).");
        
        printf("Client PIN received.\n");
        
        // timer based on difficulty
        time_t start_time, current_time;
        
        // start timer
        time(&start_time);

        while (gameOver==0) {
            
            // timer check
            time(&current_time);
            int time_left = (difficulty == 1 ? 90 : (difficulty == 2 ? 60 : 30)) - (current_time - start_time);
            
            if (time_left >= 0) {
              printf("Time left: %d seconds\n", time_left);
            } else {
                printf("Time's up! The round has ended.\n");
                gameOver = 1;
                break;
            }
            
            printf("\n[Turn] Server's turn to guess the client's PIN\n");
            printf("Enter your guess:\n> ");
            for (int i = 0; i < pin_length; i++) scanf(" %c", &guess[i]);
            serverGuessCount++;

            n = send(client_sock, guess, pin_length, 0);
            if (n < 0) die_with_error("Error: send() Failed (server guessing).");
            
            bzero(result, 256);
            recv(client_sock, result, 255, 0);
            printf("[Client] > %s\n", result);
 
            if (strncmp(result, "Correct", 7) == 0) {
                printf("Server wins this level!\n");
                serverWins++;
                gameOver = 1;
                break;
            }

            printf("\n[Client's Turn] Waiting for client's guess...\n");
            bzero(guess, pin_length);
            n = recv(client_sock, guess, pin_length, 0);
            if (n < 0) die_with_error("Error: recv() Failed (receiving guess).");
            clientGuessCount++;
            
            int correct_place = 0, correct_digit = 0;
            for (int i = 0; i < pin_length; i++) {
                if (guess[i] == serverPin[i]) {
                    correct_place++;
                } else {
                    for (int j = 0; j < pin_length; j++) {
                        if (i != j && guess[i] == serverPin[j]) {
                            correct_digit++;
                            break;
                        }
                    }
                }
            }

            bzero(result, 256);
            if (correct_place == pin_length) {
                sprintf(result, "Correct! You guessed it in %d attempts!", clientGuessCount);
                send(client_sock, result, strlen(result), 0);
                printf("Client wins this level!\n");
                clientWins++;
                gameOver = 1;
                break;
            } else {
                sprintf(result, "%d correct in place, %d correct but wrong place", correct_place, correct_digit);
                send(client_sock, result, strlen(result), 0);
            }
        }

        difficulty++;
        send(client_sock, &difficulty, sizeof(int), 0);
        printf("Would you like to play again?[y/n]\n");
        scanf("%c", replay);
        if('y'){
            continue;
        }
        else {
            break;
        }
    }
    
    printf("\n=== GAME OVER ===\n");
    printf("Server wins: %d\n", serverWins);
    printf("Client wins: %d\n", clientWins);

    if (serverWins > clientWins) {
        printf("🏆 Server is the overall winner!\n");
    } else if (clientWins > serverWins) {
        printf("🏆 Client is the overall winner!\n");
    } else {
        printf("🤝 It's a tie!\n");
    }

    printf("\nClosing connection ...\n");
    close(client_sock);
    
    return 0;
}

