# Compiler and flags
CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx

# Targets
all: prog saver jouer chars charmult

# Main program
prog: menu.o main.o
	$(CC) menu.o main.o -o prog $(LDFLAGS)

menu.o: menu.c menu.h
	$(CC) $(CFLAGS) -c menu.c

main.o: main.c menu.h
	$(CC) $(CFLAGS) -c main.c

# Saver program
saver: save.o
	$(CC) save.o -o saver $(LDFLAGS)

save.o: save.c
	$(CC) $(CFLAGS) -c save.c

# Jouer program
jouer: joueur.o
	$(CC) joueur.o -o jouer $(LDFLAGS)

joueur.o: joueur.c
	$(CC) $(CFLAGS) -c joueur.c

# Chars program
chars: char_mono.o
	$(CC) char_mono.o -o chars $(LDFLAGS)

char_mono.o: char_mono.c
	$(CC) $(CFLAGS) -c char_mono.c

# Charmult program
charmult: char_multi.o
	$(CC) char_multi.o -o charmult $(LDFLAGS)

char_multi.o: char_multi.c
	$(CC) $(CFLAGS) -c char_multi.c

# Clean up
clean:
	rm -f *.o prog saver jouer chars charmult
