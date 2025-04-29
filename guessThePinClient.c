/*#----------------------------------------------------------------------------------------------------------------#
# Filename : guessThePinclient.c#
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
#-------------------------------------------------------------------------------------------------------------#*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h> // for timer functionality

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
    int serverWins = 0, clientWins = 0; 
    
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
    
    int difficulty;
    int pin_lengths[] = {3, 4, 5};
    char level_names[][7] = {"Easy", "Medium", "Hard"};
    
    recv(client_sock, &difficulty, sizeof(int), 0);
    
    while (difficulty <= 3) {
        int pin_length = pin_lengths[difficulty-1];
        char serverPin[5], clientPin[5], guess[5], result[256];
        int serverGuessCount = 0, clientGuessCount = 0;
        int gameOver = 0;
        
        printf("\n=== Level %d: %s (%d pins) ===\n", difficulty, level_names[difficulty-1], pin_length);
    
        printf("Enter your %d-digit PIN: ", pin_length);
        for (int i = 0; i < pin_length; i++) {
            scanf(" %c", &clientPin[i]);
        }
        printf("Your PIN set to: ");
        for (int i = 0; i < pin_length; i++) printf("%c ", clientPin[i]);
        printf("\n");
        
        n = send(client_sock, clientPin, pin_length, 0);
        if (n < 0) die_with_error("Error: send() Failed (sending pin).");
        
        n = recv(client_sock, serverPin, pin_length, 0);
        if (n < 0) die_with_error("Error: recv() Failed (receiving pin).");

        printf("Server PIN received.\n");
        
        // timer based on difficulty
        time_t start_time, current_time;
        
        // start timer
        time(&start_time);
        
        while (gameOver==0) {
            // timer check
            time(&current_time);
            int time_left = (difficulty == 1 ? 90 : (difficulty == 2 ? 60 : 30)) - (current_time - start_time);
            
            if (time_left >= 0) {
              printf("Time's left: %d seconds\n", time_left);
            } else {
              printf("Time's up! The round has ended.\n");
              gameOver = 1;
              break;
            }
            
            printf("\n[Server's Turn] Waiting for server's guess...\n");
            bzero(guess, 5);
            recv(client_sock, guess, pin_length, 0);
            serverGuessCount++;

            int correct_place = 0, correct_digit = 0;
            for (int i = 0; i < pin_length; i++) {
                if (guess[i] == clientPin[i]) {
                    correct_place++;
                } else {
                    for (int j = 0; j < pin_length; j++) {
                        if (i != j && guess[i] == clientPin[j]) {
                            correct_digit++;
                            break;
                        }
                    }
                }
            }

            bzero(result, 256);
            if (correct_place == pin_length) {
                sprintf(result, "Correct! You guessed it");
                send(client_sock, result, strlen(result), 0);
                printf("Server wins this level!\n");
                serverWins++;
                gameOver = 1;
                break;
            } else {
                sprintf(result, "%d correct in place, %d correct but wrong place", correct_place, correct_digit);
                send(client_sock, result, strlen(result), 0);
            }
            
            if (gameOver) break;

            printf("\n[Turn] Your turn to guess the server's PIN!\n");
            printf("Enter your guess:\n> ");
            for (int i = 0; i < pin_length; i++) scanf(" %c", &guess[i]);
            clientGuessCount++;

            n = send(client_sock, guess, pin_length, 0);
            if (n < 0) die_with_error("Error: send() Failed (client guess).");

            bzero(result, 256);
            n = recv(client_sock, result, 255, 0);
            if (n < 0) die_with_error("Error: recv() Failed (result from server).");

            printf("[Server] > %s\n", result);
        
            if (strncmp(result, "Correct", 7) == 0) {
                printf("You win this level in %d attempts!\n", clientGuessCount);
                clientWins++;
                gameOver = 1;
                break;
            }
        }

        recv(client_sock, &difficulty, sizeof(int), 0);
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
