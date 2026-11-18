#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define PORT 6969
#define MAX_MSG_SIZE 512
#define POLL_TIMEOUT 10

typedef struct {
    char *name;
    char *password;
    bool logged_in;
} User;

bool read_from_socket(int sock, char *buff);

#endif // COMMON_H
