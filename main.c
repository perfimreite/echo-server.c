#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define PORT 6969
#define MAX_CONNS 32

char buffer[10];

bool print_client_message(int client)
{
    size_t bytes_read = read(client, buffer, sizeof(buffer));

    assert(buffer[bytes_read-2] == '\r' && buffer[bytes_read-1] == '\n');
    buffer[bytes_read-1] = '\0';
    buffer[bytes_read-2] = '\0';

    if (strcmp(buffer, "quit") == 0) {
        return false;
    }
    printf("Client message: %s\n", buffer);
    return true;
}

int main (void)
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        fprintf(stderr, "ERROR: could not create socket\n");
        return 1;
    }

    struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        0,
    };

    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "ERROR: could not bind socket to adress\n");
        return 1;
    };

    if (listen(server, MAX_CONNS) < 0) {
        fprintf(stderr, "ERROR: could not listen to socket\n");
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

    while (true) {
        int client = accept(server, 0, 0);
        if (client < 0) {
            fprintf(stderr, "ERROR: could not accept client\n");
            return 1;
        }

        printf("Connection established with client\n");

        const char *message = "Connection established with server\n";
        write(client, message, strlen(message));

        while (true) {
            if (!print_client_message(client)) break;
        }

        if (close(client) < 0) {
            fprintf(stderr, "ERROR: could not close client socket\n");
            return 1;
        }
    }

    if (close(server) < 0) {
        fprintf(stderr, "ERROR: could not close client socket\n");
        return 1;
    }

    return 0;
}

