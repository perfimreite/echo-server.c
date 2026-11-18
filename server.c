#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "common.h"

static char t_buff[MAX_MSG_SIZE];
static char r_buff[MAX_MSG_SIZE];

#define MAX_USERNAME_SIZE 32
#define MAX_PASSWORD_SIZE 32

#define MAX_CONNS 32
pthread_t threads[MAX_CONNS];

#define MAX_USERS 256
static User users[MAX_USERS];
static size_t user_count = 0;

#define MAX_SESSIONS 32

typedef struct {
    int fd;
    User *user;
} Session;

static void print_users()
{
    printf("--- USER TABLE ---\n");
    for (size_t i = 0; i < user_count; i++) {
        printf("%zu %s %s\n", i, users[i].name, users[i].logged_in ? "logged_in" : "logged_out");
    }
    printf("------------------\n");
}

static void trim_suffix(char *s)
{
    if (s[strlen(s)-1] == '\n') {
        s[strlen(s)-1] = '\0';
    }
    if (s[strlen(s)-1] == '\r') {
        s[strlen(s)-1] = '\0';
    }
}

static User *get_user_by_name(const char *name)
{
    for (User *u = users; u < &users[user_count]; u++) {
        if (strcmp(name, u->name) == 0) {
            return u;
        }
    }
    return NULL;
}

static void log_out(Session *s)
{
    if (s->user) {
        s->user->logged_in = false;
        s->user = NULL;

        sprintf(t_buff, "You were logged out.\n");
    } else {
        sprintf(t_buff, "You are not currently logged in.\n");
    }

    write(s->fd, t_buff, strlen(t_buff));
}

static void create_account(Session *s)
{
    User user = {0};
    user.name = malloc(MAX_USERNAME_SIZE + 1);
    user.password = malloc(MAX_PASSWORD_SIZE + 1);

    while (true) {
        sprintf(t_buff, "name: \n");
        write(s->fd, t_buff, strlen(t_buff));
        read_from_socket(s->fd, r_buff);
        trim_suffix(r_buff);

        if (!get_user_by_name(r_buff)) {
            strcpy(user.name, r_buff);
            break;
        }

        sprintf(t_buff, "Name is already taken!\n");
        write(s->fd, t_buff, strlen(t_buff));
    }

    sprintf(t_buff, "password: \n");
    write(s->fd, t_buff, strlen(t_buff));
    read_from_socket(s->fd, r_buff);
    trim_suffix(r_buff);
    strcpy(user.password, r_buff);

    user.logged_in = true;
    users[user_count++] = user;

    printf("INFO: created new user '%s'.\n", user.name);

    sprintf(t_buff, "Account was created successfully! You are now logged in as '%s'.\n", user.name);
    write(s->fd, t_buff, strlen(t_buff));

    s->user = &users[user_count - 1];
}

static void log_in(Session *s)
{
    User *user = NULL;

    while (true) {
        sprintf(t_buff, "name: \n");
        write(s->fd, t_buff, strlen(t_buff));
        read_from_socket(s->fd, r_buff);
        trim_suffix(r_buff);

        user = get_user_by_name(r_buff);
        if (user) {
            break;
        }

        sprintf(t_buff, "No such username exists.\n");
        write(s->fd, t_buff, strlen(t_buff));
    }

    while (true) {
        sprintf(t_buff, "password: \n");
        write(s->fd, t_buff, strlen(t_buff));
        read_from_socket(s->fd, r_buff);
        trim_suffix(r_buff);

        if (strcmp(r_buff, user->password) == 0) {
            user->logged_in = true;

            s->user = user;

            sprintf(t_buff, "You are now logged in as '%s'.\n", user->name);
            write(s->fd, t_buff, strlen(t_buff));
            return;
        }

        sprintf(t_buff, "Password is incorrect!\n");
        write(s->fd, t_buff, strlen(t_buff));
    }
}

const char *commands = "\n"
    "---------------------------------------------------\n"
    "/commands       - List all existing commands\n"
    "/create-account - Create new user account\n"
    "/login          - Login in to existing user account\n"
    "/logout         - Log out of current account\n"
    "---------------------------------------------------\n";

static bool print_client_message(Session *s)
{
    memset(r_buff, 0, MAX_MSG_SIZE);

    if (!read_from_socket(s->fd, r_buff)) {
        printf("INFO: Client closed connection abnormaly\n");
        return false;
    }

    trim_suffix(r_buff);

    printf("[#%s] %s\n", s->user ? s->user->name : "", r_buff);

    if (strcmp(r_buff, "/quit") == 0) {
        printf("INFO: Client closed connection via command: '%s'\n", r_buff);
        return false;
    }

    if (strcmp(r_buff, "/commands") == 0) {
        write(s->fd, commands, strlen(commands));
    } else if (strcmp(r_buff, "/create-account") == 0) {
        if (!s->user) {
            create_account(s);
        } else {
            sprintf(t_buff, "You are already logged in as '%s'", s->user->name);
            write(s->fd, t_buff, strlen(t_buff));
        }
    } else if (strcmp(r_buff, "/login") == 0) {
        if (!s->user) {
            log_in(s);
        } else {
            sprintf(t_buff, "You are already logged in as '%s'", s->user->name);
            write(s->fd, t_buff, strlen(t_buff));
        }
    } else if (strcmp(r_buff, "/logout") == 0) {
        log_out(s);
    }

    print_users();

    return true;
}

static void *handle_connection(void *arg)
{
    int sock = *(int *)arg;

    Session s = {0};

    s.fd = accept(sock, NULL, NULL);
    if (s.fd < 0) {
        perror("ERROR: accept failed");
        return NULL;
    }

    printf("INFO: Connection established with client: %d\n", s.fd);

    const char *message = "Connection established ('/commands' for info)\n";
    write(s.fd, message, strlen(message));

    while (true) {
        if (!print_client_message(&s)) break;
    }

    if (close(s.fd) < 0) {
        perror("ERROR: client close failed\n");
    }

    return NULL;
}

int main (void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR: socket failed");
        return 1;
    }
    printf("INFO: Created socket\n");

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
    printf("INFO: Bound socket with address\n");

    if (listen(sock, MAX_CONNS) < 0) {
        perror("ERROR: listen failed");
        return 1;
    }
    printf("INFO: Server is listening on port %d...\n", PORT);

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

    if (close(sock) < 0) {
        perror("ERROR: server close failed");
    }

    return 0;
}

