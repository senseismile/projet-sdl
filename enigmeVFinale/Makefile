CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer
SOURCES = main.c source.c enigme.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = game
HEADERS = header.h enigme.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
