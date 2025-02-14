#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
// #include <string.h>

#define PORT 42069

int main ()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        AF_INET,
        htons(PORT),
        0,
    };

    int b = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (b < 0) {
        printf("ERROR: could not start server...\n");
        return 1;
    };
    listen(socket_fd, 32);
    printf("Server is listening on port %d\n", PORT);
    
    int client_id = 1;
    while (1) {
        int client_fd = accept(socket_fd, 0, 0);
        printf("Connection established with user %d.\n", client_id++);

        while (1) {
            char client_buffer[1024] = {0};
            read(client_fd, client_buffer, sizeof(client_buffer));
            printf("Client message: %s", client_buffer);
        }

        close(client_fd);
    }

    close(socket_fd);
    return 0;
}
