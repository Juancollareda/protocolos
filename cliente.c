#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1" // Cambiar esto a la dirección IP del servidor
#define PORT 8080

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error al crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida o no soportada");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al conectarse al servidor");
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor. Puedes empezar a enviar mensajes.\n");

    while (1) {
        printf("Mensaje: ");
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Saliendo del cliente...\n");
            break;
        }

        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
            perror("Error al enviar el mensaje al servidor");
            exit(EXIT_FAILURE);
        }

        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Respuesta del servidor: %s\n", buffer);
        } else if (bytes_read == 0) {
            printf("El servidor ha cerrado la conexión\n");
            break;
        } else {
            perror("Error al recibir la respuesta del servidor");
            exit(EXIT_FAILURE);
        }
    }

    close(client_socket);

    return 0;
}
