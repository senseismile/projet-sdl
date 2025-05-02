CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer
SOURCES = main.c game.c
OBJECTS = $(SOURCES:.c=.o)
EXEC = game
MENU_SRC = menu.c
MENU_EXEC = menu

all: $(EXEC) $(MENU_EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

$(MENU_EXEC): $(MENU_SRC)
	$(CC) $(MENU_SRC) -o $(MENU_EXEC) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(EXEC) $(MENU_EXEC)

.PHONY: all clean
