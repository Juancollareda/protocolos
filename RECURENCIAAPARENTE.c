#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#define BUFFER_SIZE 1024
#define PORT 8080
#define MAX_CLIENTS 10

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    fd_set readfds;
    int max_sd, activity, i, sd;
    int client_sockets[MAX_CLIENTS];
    char buffer[BUFFER_SIZE];

    // Inicializar los clientes sockets a 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al vincular el socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Error al escuchar en el socket");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("Error al utilizar select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_socket, &readfds)) {
            client_addr_len = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("Error al aceptar la conexión del cliente");
                exit(EXIT_FAILURE);
            }

            printf("Nuevo cliente conectado\n");

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                if (read(sd, buffer, sizeof(buffer)) == 0) {
                    getpeername(sd, (struct sockaddr *)&client_addr, &client_addr_len);
                    printf("Cliente desconectado, IP: %s, Puerto: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    printf("Cliente dice: %s", buffer);
                    write(sd, "¡Hola, cliente!\n", strlen("¡Hola, cliente!\n"));
                }
            }
        }
    }

    close(server_socket);

    return 0;
}
