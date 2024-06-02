#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <vector>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void broadcastMessage(int senderSocket, const std::string& message, const std::vector<int>& clientSockets) {
    for (int clientSocket : clientSockets) {
        if (clientSocket != senderSocket) {
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }
}

int main() {
    int serverSocket, newSocket, maxClients = MAX_CLIENTS;
    int clientSockets[MAX_CLIENTS] = {0};
    int maxSocket;
    struct sockaddr_in address;
    fd_set readfds;

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to the port
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << PORT << std::endl;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        maxSocket = serverSocket;

        for (int i = 0; i < maxClients; i++) {
            int socket = clientSockets[i];
            if (socket > 0) {
                FD_SET(socket, &readfds);
            }
            if (socket > maxSocket) {
                maxSocket = socket;
            }
        }

        int activity = select(maxSocket + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Select error" << std::endl;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&address)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            std::cout << "New connection, socket fd is " << newSocket << std::endl;

            for (int i = 0; i < maxClients; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    std::cout << "Adding to list of sockets as " << i << std::endl;
                    break;
                }
            }
        }

        for (int i = 0; i < maxClients; i++) {
            int socket = clientSockets[i];
            if (FD_ISSET(socket, &readfds)) {
                char buffer[BUFFER_SIZE];
                int valread = read(socket, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    close(socket);
                    clientSockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    std::string message = std::string(buffer);
                    std::cout << "Message from client: " << message << std::endl;
                    broadcastMessage(socket, message, std::vector<int>(clientSockets, clientSockets + maxClients));
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}
