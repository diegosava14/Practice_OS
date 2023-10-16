# Makefile for compiling Bowman.c and Poole.c

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra

all: Bowman Poole

Bowman: Bowman.c
	$(CC) Bowman.c -o Bowman $(CFLAGS)

Poole: Poole.c
	$(CC) Poole.c -o Poole $(CFLAGS)

clean:
	rm -f Bowman Poole
