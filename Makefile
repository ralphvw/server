CC = gcc
CFLAGS = -Wall -Wextra

all: server

server: sock.c
	$(CC) $(CFLAGS) sock.c -o server

clean:
	rm -f server
