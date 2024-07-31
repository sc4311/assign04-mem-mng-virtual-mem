# Makefile for the memory management assignment

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lm

# Targets
TARGETS = part1 part2 part3

# Source files
SRC1 = part1.c
SRC2 = part2.c
SRC3 = part3.c

# Object files
OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)
OBJ3 = $(SRC3:.c=.o)

# Default target
all: $(TARGETS)

# Build part1
part1: $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $^

# Build part2
part2: $(OBJ2)
	$(CC) $(CFLAGS) -o $@ $^

# Build part3
part3: $(OBJ3)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(TARGETS) $(OBJ1) $(OBJ2) $(OBJ3)

# Phony targets
.PHONY: all clean