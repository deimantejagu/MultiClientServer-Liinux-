#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX 80
#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 2025

void startGame(int sock);

int main() {
    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        printf("Error: Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server_address.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Error: Failed to connect to server %s %d\n", SERVER_ADDR, SERVER_PORT);
        close(client_socket);
        exit(EXIT_FAILURE);
    } else {
        printf("Connected to server %s %d\n", SERVER_ADDR, SERVER_PORT);
    }

    startGame(client_socket);

    // Close socket
    close(client_socket);

    return 0;
}

void startGame(int sock)
{
    int num = 0, response = 0, lower, upper;

    printf("Enter the lower bound: ");
    scanf("%d", &lower);
    send(sock, &lower, sizeof(lower), 0);

    printf("Enter the upper bound: ");
    scanf("%d", &upper);
    send(sock, &upper, sizeof(upper), 0);

    for (;;) {
        printf("Enter the number from %d to %d: ", lower, upper);

        scanf("%d", &num);
        if (send(sock, &num, sizeof(num), 0) == -1) {
            printf("Error: Failed to send data to server.\n");
            break;
        }

        if (recv(sock, &response, MAX - 1, 0) <= 0) {
            printf("Error: Failed to receive data from server.\n");
            break;
        }

        if(response == 1){
            printf("You win!\n");
            break;
        } else if(response == 2){
            printf("The number is bigger\n");
        } else if(response == 3){
            printf("The number is smaller\n");
        } else if (response == 5) {
            printf("Error: Server is full. Disconnecting...\n");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }
}
