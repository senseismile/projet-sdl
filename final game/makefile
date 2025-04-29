CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -std=c99 -I. $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs) -lSDL_image -lSDL_ttf -lSDL_mixer -lm
OBJS = game.o main.o
TARGET = game
DEPS = $(OBJS:.o=.d)

all: $(TARGET)
	@echo "Build complete: $(TARGET)"

$(TARGET): $(OBJS)
	@echo "Linking $@"
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	@echo "Cleaning up"
	rm -f $(TARGET) $(OBJS) $(DEPS)

rebuild: clean all

run: $(TARGET)
	@echo "Running $(TARGET)"
	./$(TARGET)

.PHONY: all clean rebuild run

-include $(DEPS)
