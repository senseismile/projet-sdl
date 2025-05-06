CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -std=c99 -I. $(shell sdl-config --cflags) -DSAVE_PATH="../menu/savegame.dat"
LDFLAGS = $(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lSDL_mixer -lm
OBJS = game.o main.o
TARGET = game
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

rebuild: clean all

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run

-include $(DEPS)
