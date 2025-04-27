CC = gcc
CFLAGS = -g -Wall -I.
LDFLAGS = -lpthread

all: server client

server: server.c csapp.o
	$(CC) $(CFLAGS) -o server server.c csapp.o $(LDFLAGS)

client: client.c csapp.o
	$(CC) $(CFLAGS) -o client client.c csapp.o $(LDFLAGS)

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f server client *.o 