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
#include <stdbool.h>
#include <ctype.h> // for isdigit() function

// Function to print an error message and exit the program
void die_with_error(char *error_msg){
    printf("\n\x1b[31mERROR:\x1b[0m %s\n", error_msg);
    exit(-1);
}

int main(int argc,  char *argv[]){
    int client_sock, port_no, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    int serverGuessCount = 0, clientGuessCount = 0, gameOver = 0;
    int serverWins = 0, clientWins = 0;

    // Top banner
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║      🎮GUESS THE PIN - CLIENT SIDE     ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    //Checks if correct number of arguments are passed
    if (argc < 3) {
        printf("Usage: %s <hostname> <port_no>\n", argv[0]);
        exit(1);
    }

    //Create socket
    printf("[1] Creating socket...\n");
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
        die_with_error("socket() Failed.");

    //Resolve hostname to IP address    
    printf("[2] Resolving host '%s'...\n", argv[1]);
    server = gethostbyname(argv[1]);
    if (server == NULL)
        die_with_error("No such host.");

    //Setup server address struct
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&server_addr.sin_addr.s_addr,
          server->h_length);
    port_no = atoi(argv[2]);
    server_addr.sin_port = htons(port_no);

    //Connect to server
    printf("[3] Connecting to %s:%d...\n", argv[1], port_no);
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("connect() Failed.");

    printf("\x1b[32mConnected successfully!\x1b[0m\n");

    //Game Levels logic variables
    int difficulty;
    int pin_lengths[] = {3, 4, 5}; //Difficulty based on pin lenght (Easy: 3, Medium: 4, Hard: 5)
    char level_names[][7] = {"Easy", "Medium", "Hard"}; //Labels for difficulty levels
    char server_replay, client_replay; //Stores replay decisions from client and server
    
    repeat:
    //Receives initial game difficulty level from server
    recv(client_sock, &difficulty, sizeof(int), 0);

    //Game loop start
    while (difficulty <= 3) {
        int pin_length = pin_lengths[difficulty-1];
        char serverPin[5], clientPin[5], guess[5], result[256];
        time_t start_time, current_time;
        bool isValid;
        gameOver = 0; //Rests round end flag
        serverGuessCount = clientGuessCount = 0;

        //Displays Round  header and info
        printf("\n");
        printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
        printf("┃  Level %d: %-6s   [PIN length: %d]                ┃\n", difficulty, level_names[difficulty-1], pin_length);
        printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");

        // Prompt and validate client PIN input
        do {
            isValid = true;  
            // Set Client PIN
            printf("→ Enter your %d-digit PIN: ", pin_length);
    
            for (int i = 0; i < pin_length; i++) {
                char ch;
                scanf(" %c", &ch);
    
                // Validates if the input are digits, otherwise retype PIN
                if (isdigit(ch)) {
                    clientPin[i] = ch;
                } else {
                    printf("✖ Invalid character detected. Please enter a new PIN.\n\n");
                    isValid = false;
    
                    // Clear remaining input buffer and flush line
                    while (getchar() != '\n');  
    
                    break;
                }
            }
        } while (!isValid);
        
        //Displays Client's set pin
        printf("✔ Your PIN is set: ");
        for (int i = 0; i < pin_length; i++) printf("%c", clientPin[i]);
        printf("\n\n");
        
        //Exchange pins with server
        send(client_sock, clientPin, pin_length, 0);
        recv(client_sock, serverPin, pin_length, 0);
        printf("ℹ Server PIN received (hidden).\n");//Hides Server's pin so player can't see it
        
        //Timer starts for the round
        time(&start_time);
        while (!gameOver) {
            time(&current_time);
            //Timer adjusts according to difficulty level
            int time_left = (difficulty==1?90:(difficulty==2?60:30)) - (current_time - start_time);

            //Displays time left or end round
            if (time_left >= 0) {
                printf("[⏱ Time left: %2d sec]\n", time_left);
            } else {
                printf("[⏱ Time's up!] Round ended.\n");
                gameOver = 1;
                break;
            }

            // Server's turn
            printf("\n─[ Server's Turn ]───────────────────────────────\n");
            bzero(guess, 5);
            recv(client_sock, guess, pin_length, 0);
            serverGuessCount++;

            //Checks server's guess
            int correct_place = 0, correct_digit = 0;
            for (int i = 0; i < pin_length; i++) {
                if (guess[i] == clientPin[i]) correct_place++;
                else {
                    for (int j = 0; j < pin_length; j++){
                        if (guess[i] == clientPin[j]) { correct_digit++; break; }
                    }
                }
            }

            //Displays server's guess result
            bzero(result, 256);
            if (correct_place == pin_length) {
                sprintf(result, "Correct! You guessed it");
                send(client_sock, result, strlen(result), 0);
                printf("🏁 Server wins this level!\n");
                serverWins++; gameOver = 1; break;
            } else {
                //Sends hint back to server
                sprintf(result, "%d in place, %d wrong place", correct_place, correct_digit);
                send(client_sock, result, strlen(result), 0);
                printf("Server guess: %.*s → %s\n", pin_length, guess, result);
            }

            if (gameOver) break;

            // Client's turn
            printf("\n─[ Your Turn ]──────────────────────────────────\n");
            printf("Enter guess: ");
            for (int i = 0; i < pin_length; i++) scanf(" %c", &guess[i]);
            clientGuessCount++;
            send(client_sock, guess, pin_length, 0);

            //Receives guesss result from server
            bzero(result, 256);
            recv(client_sock, result, 255, 0);
            printf("Server: %s\n", result);

            if (strncmp(result, "Correct", 7) == 0) {
                printf("🏁 You win this level in %d attempts!\n", clientGuessCount);
                clientWins++; gameOver = 1; break;
            }
        }

        //Receives next round's difficulty level from server
        recv(client_sock, &difficulty, sizeof(int), 0);
    }
    
    //Displays final scoreboard and Declares overall winner
    printf("\n╔═══════════════════ GAME OVER ═══════════════════╗\n");
    printf("║  Server wins : %d                                ║\n", serverWins);
    printf("║  Client wins : %d                                ║\n", clientWins);
    printf("╠═════════════════════════════════════════════════╣\n");
    if (serverWins > clientWins)
        printf("║  🏆  Overall Winner: SERVER                     ║\n");
    else if (clientWins > serverWins)
        printf("║  🏆  Overall Winner: CLIENT                     ║\n");
    else
        printf("║  🤝  It's a TIE!                               ║\n");
    printf("╚═════════════════════════════════════════════════╝\n");

    //Asks user and server if they want to replay
    n = recv(client_sock, &server_replay, 1, 0);
    printf("\nWould you like to play again?[y/n]\n");
    scanf(" %c", &client_replay);
    n = send(client_sock, &client_replay, 1, 0);
    //Starts over if both agree to replay
    if(client_replay == 'y' && server_replay == 'y'){
        goto repeat;
    }

    
    //Closes connection
    printf("\nClosing connection...\n");
    close(client_sock);
    return 0;
}
