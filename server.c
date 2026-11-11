#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"

#define MAX_CONNS 32

char t_buff[MAX_MSG_SIZE];
char r_buff[MAX_MSG_SIZE];

size_t trim_suffix(char *s, size_t l)
{
    if (s[l-1] == '\n') {
        s[--l] = '\0';
    }
    if (s[l-1] == '\r') {
        s[--l] = '\0';
    }
    return l;
}

bool print_client_message(int client)
{
    size_t bytes_read = read(client, r_buff, MAX_MSG_SIZE);
    trim_suffix(r_buff, bytes_read);
    printf("Client message: %s\n", r_buff);

    if (strcmp(r_buff, "quit") == 0) {
        write(client, STATUS_QUIT, strlen(STATUS_QUIT));
        printf("Client closed connection\n");
        return false;
    }

    write(client, STATUS_OK, strlen(STATUS_OK));

    return true;
}

static size_t id;

int main (void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "ERROR: could not create socket\n");
        return 1;
    }

    const struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        {0},
        {0},
    };

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "ERROR: could not bind socket to adress\n");
        return 1;
    };

    if (listen(sock, MAX_CONNS) < 0) {
        fprintf(stderr, "ERROR: could not listen to socket\n");
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

    while (true) {
        int client = accept(sock, 0, 0);
        ++id;
        if (client < 0) {
            fprintf(stderr, "ERROR: could not accept client\n");
            return 1;
        }

        printf("Connection established with client(%zu)\n", id);

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

    if (close(sock) < 0) {
        fprintf(stderr, "ERROR: could not close server socket\n");
        return 1;
    }

    return 0;
}

