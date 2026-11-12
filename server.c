#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "common.h"

#define MAX_CONNS 32
pthread_t threads[MAX_CONNS];

char t_buff[MAX_MSG_SIZE];
char r_buff[MAX_MSG_SIZE];

static size_t trim_suffix(char *s, size_t l)
{
    if (l == 0) return 0;
    if (s[l-1] == '\n') {
        s[--l] = '\0';
    }
    if (l == 0) return 0;
    if (s[l-1] == '\r') {
        s[--l] = '\0';
    }
    return l;
}

static bool print_client_message(int client)
{
    memset(r_buff, 0, MAX_MSG_SIZE);

    int bytes_read = read(client, r_buff, MAX_MSG_SIZE);
    if (bytes_read < 0) {
        perror("ERROR: read failed");
        return false;
    }
    if (bytes_read == 0) {
        printf("Client closed connection abnormaly\n");
        return false;
    }

    trim_suffix(r_buff, bytes_read);

    if (strcmp(r_buff, "/quit") == 0) {
        write(client, STATUS_QUIT, strlen(STATUS_QUIT));
        printf("Client closed connection via command: %s\n", r_buff);
        return false;
    }

    printf("[Client] %s\n", r_buff);

    write(client, STATUS_OK, strlen(STATUS_OK));

    return true;
}

static size_t id;

static void *handle_connection(void *arg)
{
    printf("thread is here\n");
    int sock = *(int *)arg;

    while (true) {
        int client = accept(sock, 0, 0);
        ++id;
        if (client < 0) {
            perror("ERROR: accept failed");
            return NULL;
        }

        printf("Connection established with client(%zu)\n", id);

        const char *message = "Connection established\n";
        write(client, message, strlen(message));

        while (true) {
            if (!print_client_message(client)) break;
        }

        if (close(client) < 0) {
            perror("ERROR: client close failed\n");
            return NULL;
        }
    }

    if (close(sock) < 0) {
        perror("ERROR: server close failed\n");
        return NULL;
    }
}

int main (void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR: socket failed");
        return 1;
    }

    const struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        {0},
        {0},
    };

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("ERROR: bind failed");
        return 1;
    };

    if (listen(sock, MAX_CONNS) < 0) {
        perror("ERROR: listen failed");
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

    // NOTE: code for creating multiple fork()'s.
    //       each branch has a copy of the same memory
    //       and is therefore unaware of the state of
    //       other branches.
    //
    // pid_t root_pid = getpid();
    // printf("*root_pid=%d*\n", root_pid);
    //
    // for (int i = 0; i < 4; i++) {
    //     pid_t pid = fork();
    //     pid_t self_pid = getpid();
    //     pid_t parent_pid = getppid();
    //     printf("idx=%d, parent_pid=%d, pid=%d ", i, parent_pid, self_pid);
    //     if (pid < 0) {
    //         perror("ERROR: fork failed");
    //         return 1;
    //     }
    //     if (parent_pid != root_pid && self_pid != root_pid){
    //         printf("killed\n");
    //         return 0;
    //     }
    //     printf("\n");
    //
    //     if (pid == 0) {
    //         printf("im new here\n");
    //     }
    // }

    for (int i = 0; i < MAX_CONNS; i++) {
        pthread_create(&threads[i], NULL, handle_connection, &sock);
    }

    for (int i = 0; i < MAX_CONNS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

