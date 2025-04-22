#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

// Error handler
void die_with_error(char *error_msg){
    printf("%s", error_msg);
    exit(-1);
}

// Send a clue message
void sendclue(char *clue_msg,char *buffer, int client_sock ){
    strcpy(buffer, clue_msg);
    int n = send(client_sock, buffer, strlen(buffer), 0);
}

// Check the guess and send hints to server
void check(char pin[3], char buffer[3], int client_sock){
    int rdigit=0, rplace=0;

    for (int i = 0; i < 3; i++) {
        for (int o = 0; o < 3; o++) {
            if (pin[o] == buffer[i]){
                rdigit++;
                break;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        if (pin[i] == buffer[i]){
            rplace++;
        }
    }

    bzero(buffer, 256);

    if (rdigit == 0){
        sendclue("Nothing is correct.\n", buffer, client_sock);
    } else if (rdigit == 1){
        sendclue("1 digit is correct.\n", buffer, client_sock);
    } else {
        sprintf(buffer, "%d digits are correct.\n", rdigit);
        sendclue(buffer, buffer, client_sock);
    }

    bzero(buffer, 256);

    if (rplace == 0){
        sendclue("Nothing is in the right place.\n", buffer, client_sock);
    } else if (rplace == 1){
        sendclue("1 digit is in the right place.\n", buffer, client_sock);
    } else {
        sprintf(buffer, "%d digits are in the right place.\n", rplace);
        sendclue(buffer, buffer, client_sock);
    }
}

int main(int argc,  char *argv[]){
    int client_sock, port_no, n;
    struct sockaddr_in server_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
        printf("Usage: %s hostname port_no",  argv[0]);
        exit(1);
    }

    printf("Client starting ...\n");

    // Create socket
    client_sock = socket(AF_INET,  SOCK_STREAM,  0);
    if (client_sock < 0) 
        die_with_error("Error: socket() Failed.");

    // Resolve host
    printf("Looking for host '%s'...\n", argv[1]);
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        die_with_error("Error: No such host.");
    }
    printf("Host found!\n");

    // Connect to server
    port_no = atoi(argv[2]);
    bzero((char *) &server_addr,  sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,  (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_no);

    printf("Connecting to server at port %d...\n", port_no);
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        die_with_error("Error: connect() Failed.");

    printf("Connection successful!\n");

    // Ask user to input the PIN
    char pin[3];
    printf("Enter a 3 digit pin\n");
    for (int i = 0; i < 3; i++) {
        scanf(" %c", &pin[i]); // Added space to consume whitespace
    }

    // Communication loop
    while (1) {
        // Receive message from server
        bzero(buffer, 256);
        n = recv(client_sock, buffer, 255, 0);
        if (n < 0) die_with_error("Error: recv() Failed.");

        check(pin, buffer, client_sock);

        if (strncmp(buffer, "exit", 4) == 0) break;

        // Send guess to server
        printf("< ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        n = send(client_sock, buffer, strlen(buffer), 0);
        if (n < 0) die_with_error("Error: send() Failed.");

        if (strncmp(buffer, "exit", 4) == 0) break;
    }

    // Clean up
    printf("Closing connection ...\n");
    close(client_sock);
    return 0;
}
