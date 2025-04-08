# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer

# Source files (assuming they're in the same directory as Makefile)
SRC = main.c source.c
OBJ = $(SRC:.c=.o)
HEADERS = header.h

# Executable name
TARGET = quiz_game

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
