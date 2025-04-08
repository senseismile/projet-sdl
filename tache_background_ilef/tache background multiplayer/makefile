CFLAGS=-w -O2 -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
prog:function.o main.o
	gcc function.o main.o -o prog $(CFLAGS)

main.o:main.c
	gcc -c main.c
function.o:function.c
	gcc -c function.c
clean:
	rm -rf *.o
