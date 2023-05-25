#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define NUM_FLOWERS 40

int dead_flowers[NUM_FLOWERS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t quit = 0;

void *client_handler(void *arg)
{
    int client_socket = *((int *)arg);
    char buffer[1024] = {0};
    while (!quit) {
        // Отправляем сообщение о случайной смерти цветка каждую секунду
        sleep(1);
        int flower_index = rand() % NUM_FLOWERS;
        char msg[1024] = {0};

        pthread_mutex_lock(&mutex);
        dead_flowers[flower_index] = 1;
        pthread_mutex_unlock(&mutex);

        sprintf(msg, "%d", flower_index);
        send(client_socket, msg, sizeof(int), 0);
        printf("Flower %d has died\n", flower_index);
        // Получаем сообщение о поливке цветка от клиента
        if (recv(client_socket, buffer, sizeof(int), 0) > 0)
        {
            flower_index = atoi(buffer);
            printf("%d is blossoming again\n", flower_index);
            if (flower_index > 0 && flower_index < NUM_FLOWERS)
            {
                pthread_mutex_lock(&mutex);
                dead_flowers[flower_index] = 0;
                pthread_mutex_unlock(&mutex);
            }
            else
            {
                printf("Received invalid flower index %d\n", flower_index);
            }
        } else {
            puts("recv error");
            break;
        }
    }
    close(client_socket);
    free(arg);
}

void sigint_handler(int signum)
{
    quit = 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        puts("Please input the port number");
        exit(EXIT_FAILURE);
    }
    const int PORT = atoi(argv[1]);
    int server_fd, new_socket[2];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id[2];
    pthread_t sender_id;

    // Инициализация состояний цветков
    for (int i = 0; i < NUM_FLOWERS; i++)
    {
        dead_flowers[i] = 0;
    }

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        puts("Socket created");
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        // exit(EXIT_FAILURE);
    }
    else
    {
        puts("Bind successful");
    }
    if (listen(server_fd, 2) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    else
    {
        puts("Listening");
    }

    // Установка обработчика сигнала SIGINT (Ctrl+C)
    for (int i = 0; i < 2; ++i)
    {
        if ((new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Accepting client %d\n", i);
        }

        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket[i];
        if (pthread_create(&thread_id[i], NULL, client_handler, (void *)client_socket) < 0)
        {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < 2; ++i) {
        pthread_join(thread_id[i], NULL);
    }
    return 0;
}
