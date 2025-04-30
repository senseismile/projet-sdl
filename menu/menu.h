#ifndef MENU_H
#define MENU_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define BUTTON_WIDTH 300
#define BUTTON_HEIGHT 100
#define NUM_BUTTONS 5

// Function declarations
SDL_Surface* resizeSurface(SDL_Surface* surface, int width, int height);
void cleanup(SDL_Surface* image, SDL_Surface* boutons[], SDL_Surface* boutonsH[], Mix_Music* musique, Mix_Chunk* hoverSound, Mix_Chunk* clickSound, TTF_Font* font);
int mainMenu();
int optionsMenu();
void scaleBackgroundToFullscreen(SDL_Surface* image, SDL_Surface** imagefs);

#endif // MENU_H
