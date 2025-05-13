CFLAGS=-w -O2 -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
prog:fichier.o main.o
	gcc fichier.o main.o -o prog  $(CFLAGS)
main.o:main.c
	gcc -c main.c 
fichier.o:fichier.c
	gcc -c fichier.c 
clean:
	rm -rf *.o

