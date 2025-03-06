#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 42069
#define MAX_CONNECTIONS 32

void get_client_input(int client_fd)
{
    char client_buffer[512] = {0};
    read(client_fd, client_buffer, sizeof(client_buffer));
    printf("Client message: %s", client_buffer);
}

int main (void)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        0,
    };

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "ERROR: could not bind socket to adress\n");
        return 1;
    };

    listen(socket_fd, MAX_CONNECTIONS);
    printf("Server is listening on port %d...\n", PORT);
    
    while (1) {
        int client_fd = accept(socket_fd, 0, 0);
        if (client_fd < 0) {
            fprintf(stderr, "ERROR: could not accept client\n");
            return 1;
        }

        printf("Connection established with client\n");
        char message[] = "Connection established with server\n";
        write(client_fd, message, sizeof(message));

        while (1) {
            get_client_input(client_fd);
        }

        close(client_fd);
    }

    close(socket_fd);
    return 0;
}
