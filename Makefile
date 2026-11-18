all: server client

server: server.c common.c
	gcc -Wall -Wextra -o server server.c common.c -lpthread

client: client.c common.c
	gcc -Wall -Wextra -o client client.c common.c

clean:
	rm -rf server
	rm -rf client
