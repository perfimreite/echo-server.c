#include <stdio.h>
#include <stdbool.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include "common.h"

static void read_line(const char *prompt, char *buff)
{
    printf("%s", prompt);
    if (!fgets(buff, MAX_MSG_SIZE, stdin)) {
        fprintf(stderr, "ERROR: `fgets()` failed\n");
    }
}

char t_buff[MAX_MSG_SIZE];
char r_buff[MAX_MSG_SIZE];

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // char *username = NULL;

    const struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        {0},
        {0},
    };

    if (connect(sock, (const struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("ERROR: connect failed");
        return 1;
    }

    struct pollfd p = {
        .fd = sock,
        .events = POLLIN,
        .revents = 0,
    };

    while (true) {
        poll(&p, 1, POLL_TIMEOUT);
        if (p.revents & POLLIN) {
            if (!read_from_socket(sock, r_buff)) break;

            printf("[Server] %s", r_buff);
        }

        read_line("> ", t_buff);
        if (strlen(t_buff) > 0 && strcmp(t_buff, "\n") != 0) {
            write(sock, t_buff, strlen(t_buff));
        }
    }

    if (close(sock) < 0) {
        perror("ERROR: server close failed\n");
        return 1;
    }
    printf("Server closed connection successfully\n");

    return 0;
}

