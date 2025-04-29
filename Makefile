CC = gcc
CFLAGS = -g -Wall -I. -pthread
LDFLAGS = 

all: server client

server: server.o csapp.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: client.o csapp.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c csapp.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f server client *.o