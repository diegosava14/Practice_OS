CC = gcc

CFLAGS = -Wall -Wextra
CFLAGS_THREADS  = -Wall -Wextra -pthread

all: Bowman Poole Discovery

Bowman: Bowman.c
	$(CC) Bowman.c -o Bowman $(CFLAGS)

Poole: Poole.c
	$(CC) Poole.c -o Poole $(CFLAGS)

Discovery: Discovery.c
	$(CC) Discovery.c -o Discovery $(CFLAGS_THREADS)

clean:
	rm -f Bowman Poole Discovery
