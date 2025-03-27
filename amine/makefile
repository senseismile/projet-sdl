CC = gcc
CFLAGS = -Wall -Wextra -g `sdl-config --cflags` -I.
LDFLAGS = `sdl-config --libs` -lSDL_image -lSDL_ttf

OBJS = game.o main.o

all: game

game: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f game $(OBJS)

.PHONY: all clean
