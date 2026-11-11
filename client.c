#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"

char t_buff[MAX_MSG_SIZE];
char r_buff[MAX_MSG_SIZE];

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    const struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        {0},
        {0},
    };

    if (connect(sock, (const struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "ERROR: Could not connect to server\n");
        return 1;
    }

    while (true) {
        int bytes_read = read(sock, r_buff, MAX_MSG_SIZE);
        if (bytes_read < 0) {
            perror("ERROR: read failed");
            break;
        }

        r_buff[bytes_read] = '\0';
        if (strcmp(r_buff, STATUS_QUIT) == 0) {
            break;
        } else if (strcmp(r_buff, STATUS_OK) == 0) {
            // ok
        } else {
            printf("%s", r_buff);
        }

        fgets(t_buff, MAX_MSG_SIZE, stdin);
        write(sock, t_buff, strlen(t_buff));
    }

    // shutdown(sock, SHUT_WR);
    close(sock);
    printf("closed connection\n");

    return 0;
}

