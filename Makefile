all: server client

server: server.c
	gcc -Wall -Wextra -o server server.c -lpthread

client: client.c
	gcc -Wall -Wextra -o client client.c
