#include "common.h"

bool read_from_socket(int sock, char *buff)
{
    int bytes_read = read(sock, buff, MAX_MSG_SIZE);
    if (bytes_read < 0) {
        perror("ERROR: read failed");
        return false;
    }
    if (bytes_read == 0) {
        return false;
    }

    buff[bytes_read] = '\0';
    return true;
}

