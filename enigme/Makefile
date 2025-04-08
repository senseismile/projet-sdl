prog: main.o enigme.o
	gcc main.o enigme.o -o prog -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

main.o: main.c enigme.h
	gcc -c main.c -g

enigme.o: enigme.c enigme.h
	gcc -c enigme.c -g


