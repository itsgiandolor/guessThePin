/*----------------------------------------------------------------------------------------------------------------#
# Filename : guessThePinServer.c#
# Author : Dolor, Llagas, Mien, Raposa, Valencia#
# Last Modified : May 13, 2025 #
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
    printf("\n\x1b[31mERROR:\x1b[0m %s\n", error_msg);
    exit(-1);
}

int main(int argc, char *argv[]){
    int server_sock, client_sock, port_no, client_size, n;
    struct sockaddr_in server_addr, client_addr;
    int serverGuessCount = 0, clientGuessCount = 0, gameOver = 0;
    int serverWins = 0, clientWins = 0;

    // Top banner
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║        🎮 GUESS THE PIN - SERVER SIDE           ║\n");
    printf("╚════════════════════════════════════════════════╝\n\n");

    if (argc < 2) {
        printf("Usage: %s <port_no>\n", argv[0]);
        exit(1);
    }

    printf("[1] Creating server socket...\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
        die_with_error("socket() Failed.");

    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    printf("[2] Binding to port %d...\n", port_no);
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("bind() Failed.");

    listen(server_sock, 5);
    printf("✔ Server listening on port %d.\n", port_no);

    client_size = sizeof(client_addr);
    printf("[3] Waiting for client connection...\n");
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock < 0)
        die_with_error("accept() Failed.");

    printf("\x1b[32mClient connected successfully!\x1b[0m\n");

    int difficulty = 1;
    int pin_lengths[] = {3, 4, 5};
    char level_names[][7] = {"Easy", "Medium", "Hard"};
    char server_replay, client_replay;
    
    repeat:
    difficulty = 1;
    send(client_sock, &difficulty, sizeof(int), 0);

    while (difficulty <= 3) {
        int pin_length = pin_lengths[difficulty-1];
        char serverPin[5], clientPin[5], guess[5], result[256];
        time_t start_time, current_time;
        gameOver = 0;
        serverGuessCount = clientGuessCount = 0;

        // Level header
        printf("\n");
        printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
        printf("┃  Level %d: %-6s   [PIN length: %d]               ┃\n", difficulty, level_names[difficulty-1], pin_length);
        printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");

        // Set server PIN
        printf("→ Enter your %d-digit PIN: ", pin_length);
        for (int i = 0; i < pin_length; i++){
            scanf(" %c", &serverPin[i]);
        }
        printf("✔ Your PIN is set: ");
        for (int i = 0; i < pin_length; i++) printf("%c", serverPin[i]);
        printf("\n\n");

        recv(client_sock, clientPin, pin_length, 0);
        send(client_sock, serverPin, pin_length, 0);
        printf("ℹ Client PIN received (hidden).\n");

        time(&start_time);
        while (!gameOver) {
            time(&current_time);
            int time_left = (difficulty==1?90:(difficulty==2?60:30)) - (current_time - start_time);

            if (time_left >= 0) {
                printf("[⏱ Time left: %2d sec]\n", time_left);
            } else {
                printf("[⏱ Time's up!] Round ended.\n");
                gameOver = 1; break;
            }

            // Server guesses
            printf("\n─[ Your Turn (Server) ]───────────────────────────\n");
            printf("Enter guess: ");
            for (int i = 0; i < pin_length; i++) scanf(" %c", &guess[i]);
            serverGuessCount++;
            send(client_sock, guess, pin_length, 0);

            bzero(result, 256);
            recv(client_sock, result, 255, 0);
            printf("Client: %s\n", result);
            if (strncmp(result, "Correct", 7) == 0){
                printf("🏁 Server wins this level!\n");
                serverWins++; gameOver = 1; break;
            }

            // Client guesses
            printf("\n─[ Client's Turn ]───────────────────────────────\n");
            bzero(guess, pin_length);
            recv(client_sock, guess, pin_length, 0);
            clientGuessCount++;

            int correct_place = 0, correct_digit = 0;
            for (int i = 0; i < pin_length; i++){
                if (guess[i] == serverPin[i]) correct_place++;
                else {
                    for (int j = 0; j < pin_length; j++){
                        if (guess[i] == serverPin[j]){ correct_digit++; break; }
                    }
                }
            }

            bzero(result, 256);
            if (correct_place == pin_length){
                sprintf(result, "Correct! You guessed it in %d attempts!", clientGuessCount);
                send(client_sock, result, strlen(result), 0);
                printf("🏁 Client wins this level!\n");
                clientWins++; gameOver = 1; break;
            } else {
                sprintf(result, "%d in place, %d wrong place", correct_place, correct_digit);
                send(client_sock, result, strlen(result), 0);
                printf("Client guess: %.*s → %s\n", pin_length, guess, result);
            }
        }

        difficulty++;
        send(client_sock, &difficulty, sizeof(int), 0);
    }
    
    printf("\nWould you like to play again?[y/n]\n");
        scanf(" %c", &server_replay);
        n = send(client_sock, &server_replay, 1, 0);
        printf("\n[Client's Turn] Waiting for client's response...\n");
        n = recv(client_sock, &client_replay, 1, 0);
        if(client_replay == 'y' && server_replay == 'y'){
            goto repeat;
        }

    // Final results
    printf("\n╔═══════════════════ GAME OVER ═══════════════════╗\n");
    printf("║  Server wins : %d                               ║\n", serverWins);
    printf("║  Client wins : %d                               ║\n", clientWins);
    printf("╠═════════════════════════════════════════════════╣\n");
    if (serverWins > clientWins)
        printf("║  🏆  Overall Winner: SERVER                     ║\n");
    else if (clientWins > serverWins)
        printf("║  🏆  Overall Winner: CLIENT                     ║\n");
    else
        printf("║  🤝  It's a TIE!                               ║\n");
    printf("╚═════════════════════════════════════════════════╝\n");

    printf("\nClosing connection...\n");
    close(client_sock);
    return 0;
}
