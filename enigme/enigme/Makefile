CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lm

quiz: quiz.o
	$(CC) $(CFLAGS) -o quiz quiz.o $(LDFLAGS)

quiz.o: quiz.c
	$(CC) $(CFLAGS) -c quiz.c

clean:
	rm -f quiz quiz.o
