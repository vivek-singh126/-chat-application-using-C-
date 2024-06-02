#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    std::string message;
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int max_sd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            std::getline(std::cin, message);
            send(sock, message.c_str(), message.length(), 0);
        }

        if (FD_ISSET(sock, &readfds)) {
            int valread = read(sock, buffer, BUFFER_SIZE);
            if (valread > 0) {
                buffer[valread] = '\0';
                std::cout << "Server: " << buffer << std::endl;
            }
        }
    }

    close(sock);
    return 0;
}
