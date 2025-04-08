CC = gcc
CFLAGS = -Wall -Wextra -O2 -g $(shell sdl-config --cflags) -I.
LDFLAGS = $(shell sdl-config --libs) -lSDL_image -lSDL_ttf
OBJS = game.o main.o
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

game.o: game.c game.h
	$(CC) $(CFLAGS) -c game.c

main.o: main.c game.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(TARGET) $(OBJS)

rebuild: clean all

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
