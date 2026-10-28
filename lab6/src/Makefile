CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: server client

multmod.o: multmod.c multmod.h
	$(CC) $(CFLAGS) -c multmod.c

libcalc.a: multmod.o
	ar rcs libcalc.a multmod.o

server: server.c libcalc.a
	$(CC) $(CFLAGS) server.c -L. -lcalc -o server

client: client.c libcalc.a
	$(CC) $(CFLAGS) client.c -L. -lcalc -o client

clean:
	rm -f server client multmod.o libcalc.a