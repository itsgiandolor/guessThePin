#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Error handler
void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}

// Send a clue message to client
void sendclue(char *clue_msg, char *buffer, int client_sock ){
    strcpy(buffer, clue_msg);
    int n = send(client_sock, buffer, strlen(buffer), 0);
}

// Check guess and send clues to client
void check(char pin[3], char buffer[3], int client_sock){
    int rdigit=0, rplace=0;

    // Count correct digits
    for (int i = 0; i < 3; i++) {
        for (int o = 0; o < 3; o++) {
            if (pin[o] == buffer[i]){
                rdigit++;
                break;
            }
        }
    }

    // Count correct digits in correct positions
    for (int i = 0; i < 3; i++) {
        if (pin[i] == buffer[i]){
            rplace++;
        }
    }

    bzero(buffer, 256);

    // Send feedback for correct digits
    if (rdigit == 0){
        sendclue("Nothing is correct.\n", buffer, client_sock);
    } else if (rdigit == 1){
        sendclue("1 digit is correct.\n", buffer, client_sock);
    } else {
        sprintf(buffer, "%d digits are correct.\n", rdigit);
        sendclue(buffer, buffer, client_sock);
    }

    bzero(buffer, 256);

    // Send feedback for correct positions
    if (rplace == 0){
        sendclue("Nothing is in the right place.\n", buffer, client_sock);
    } else if (rplace == 1){
        sendclue("1 digit is in the right place.\n", buffer, client_sock);
    } else {
        sprintf(buffer, "%d digits are in the right place.\n", rplace);
        sendclue(buffer, buffer, client_sock);
    }
}

int main(int argc, char *argv[]){
    int server_sock, client_sock, port_no, client_size, n;
    char buffer[256];
    struct sockaddr_in server_addr, client_addr;

    if (argc < 2) {
        printf("Usage: %s port_no", argv[0]);
        exit(1);
    }

    printf("Server starting ...\n");
    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) 
        die_with_error("Error: socket() Failed.");

    // Bind to port
    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        die_with_error("Error: bind() Failed.");

    // Listen for connection
    listen(server_sock, 5);
    printf("Server listening to port %d ...\n", port_no);
    printf("Waiting for connection(s) ...\n");

    // Accept connection
    client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock < 0) 
        die_with_error("Error: accept() Failed.");

    printf("Client successfully connected ...\n");

    // Ask user to input the PIN
    char pin[3];
    printf("Enter a 3 digit pin\n");
    for (int i = 0; i < 3; i++) {
        scanf(" %c", &pin[i]); // Added space to consume any whitespace/newline
    }

    // Communication loop
    while (1) {
        // Send message to client
        printf("< ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        n = send(client_sock, buffer, strlen(buffer), 0);
        if (n < 0) die_with_error("Error: send() Failed.");

        // Exit check
        if (strncmp(buffer, "exit", 4) == 0) break;

        // Receive guess from client
        bzero(buffer, 256);
        n = recv(client_sock, buffer, 255, 0);
        if (n < 0) die_with_error("Error: recv() Failed.");

        check(pin, buffer, client_sock);

        if (strncmp(buffer, "exit", 4) == 0) break;
    }

    // Clean up
    printf("Closing connection ...\n");
    close(client_sock);
    close(server_sock);
    return 0; 
}