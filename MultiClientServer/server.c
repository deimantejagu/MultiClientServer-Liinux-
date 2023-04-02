#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define MAX 80
#define PORT 2025

int client_sockets[MAX_CLIENTS];
pthread_t client_threads[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void startGame(int conn);
int randomNumber(int lower, int upper);

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;

    startGame(client_socket);

    // Client disconnected, close socket and exit thread
    close(client_socket);
    printf("Socket closed. %d \n", client_socket);


    // Remove client socket from list
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == client_socket) {
            client_sockets[i] = -1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket, client_index;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Error creating server socket.\n");
        exit(EXIT_FAILURE);
    }

    // Bind server socket to port
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Error binding server socket to port %d.\n", PORT);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        printf("Error listening for incoming connections.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Listening for incoming connections...\n");
    }

    // Initialize client sockets to -1 (empty)
    memset(client_sockets, -1, sizeof(client_sockets));

    // Accept incoming connections and spawn new thread to handle each client
    while (1) {
        // Wait for incoming connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1) {
            printf("Error accepting incoming connection.\n");
            continue;
        } else {
            printf("Accepted incoming connection.\n");
        }

        // Add client socket to list
        pthread_mutex_lock(&mutex);
        client_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == -1) {
                client_sockets[i] = client_socket;
                client_index = i;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        // Reject client if max number of clients already connected
        if (client_index == -1) {
            int response = 5;
            send(client_socket, &response, sizeof(response), 0);
            printf("Client rejected due to maximum number of clients reached. Socket: %d\n", client_socket);
            close(client_socket);
            continue;
        }

        // Spawn new thread to handle client
        if (pthread_create(&client_threads[client_index], NULL, handle_client, &client_socket) != 0) {
            printf("Failed to create client thread. Socket: %d\n", client_socket);
            close(client_socket);
            continue;
        } else {
            printf("New client thread created. Socket: %d\n", client_socket);
        }
    }

    // Wait for all client threads to exit
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {
            pthread_join(client_threads[i], NULL);
        }
    }

    return 0;
}

void startGame(int socket)
{
    int lower = 0, upper = 0, response = 0, recvNum = 0;

    if (recv(socket, &lower, MAX - 1, 0) <= 0){
        close(socket);
        printf("Socket closed. %d \n", socket);
        return;
    }

    if (recv(socket, &upper, MAX - 1, 0) <= 0){
        close(socket);
        printf("Socket closed. %d \n", socket);
        return;
    }

    srand(time(0));
    int randomNum = randomNumber(lower, upper);

    for (;;) {
        if (recv(socket, &recvNum, MAX - 1, 0) <= 0){
            close(socket);
            break;
        }

        if(recvNum == randomNum){
            response = 1;
            send(socket, &response, sizeof(response), 0);
            // Close server socket
            close(socket);
            printf("Socket closed. %d \n", socket);
            break;
        } else if(recvNum < randomNum){
            response = 2;
            send(socket, &response, sizeof(response), 0);
        } else if (recvNum > randomNum) {
            response = 3;
            send(socket, &response, sizeof(response), 0);
        }
    }
}

int randomNumber(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}