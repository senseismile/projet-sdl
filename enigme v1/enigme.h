#ifndef ENIGME_H
#define ENIGME_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 1344
#define SCREEN_HEIGHT 756

void resetGame(int *nb, int t[]);
void choicehint(char* hintText, int pattern[], int difficulty);
int display_screen(SDL_Surface* screen, const char* image_path, Uint32 display_time);

#endif
