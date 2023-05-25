#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #define SERVER_IP "127.0.0.1"
// #define PORT 18000
volatile sig_atomic_t quit = 0;

void sigint_handler(int signum)
{
    quit = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        puts("Please input the port number and IP address");
        exit(EXIT_FAILURE);
    }
    const int PORT = atoi(argv[1]);
    const char *SERVER_IP = argv[2];
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Создание сокета
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    } else {
        puts("Socket created");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Преобразование IP-адреса из строки в формат сетевого порядка байтов
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    } else {
        puts("inet_pton done");
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    } else {
        puts("connected");
    }
    while (!quit)
    {
        char buffer[1024] = {0};
        if (recv(sock, buffer, sizeof(int), 0) > 0) {
            int flower_index = atoi(buffer);
            printf("Received flower index: %d\n", flower_index);
            // Поливка цветка
            printf("Watering flower %d\n", flower_index);
            sleep(1);
            printf("Finished watering flower %d\n", flower_index);
            send(sock, buffer, sizeof(int), 0);
        } else {
            break;
        }
    }
    close(sock);
    return 0;
}
