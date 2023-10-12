# Makefile for compiling Bowman.c with specific flags

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra

# Target executable
TARGET = Bowman

# Default rule
all: $(TARGET)

# Rule to compile Bowman.c
$(TARGET): Bowman.c
	$(CC) $< -o $@ $(CFLAGS)

# Clean rule to remove the executable
clean:
	rm -f $(TARGET)
