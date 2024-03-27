CC = gcc
CFLAGS = -Wall -Wextra
CFLAGS_THREADS = -Wall -Wextra -lpthread

all: Bowman Poole Discovery	
	rm -Rf .git
	rm -Rf Floyd

Bowman: Bowman.c common.c common.h
	$(CC) Bowman.c common.c -o Bowman $(CFLAGS_THREADS)

Poole: Poole.c common.c common.h
	$(CC) Poole.c common.c -o Poole $(CFLAGS_THREADS)

Discovery: Discovery.c common.c common.h
	$(CC) Discovery.c common.c -o Discovery $(CFLAGS_THREADS)

clean:
	rm -f Bowman Poole Discovery