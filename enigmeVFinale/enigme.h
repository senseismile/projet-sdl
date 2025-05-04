#ifndef ENIGME_H
#define ENIGME_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 1297
#define SCREEN_HEIGHT 749

typedef struct {
    SDL_Surface *ecran;
    SDL_Surface *image;
    SDL_Surface *message;
    SDL_Surface *hint;
    SDL_Surface *facile;
    SDL_Surface *moyen;
    SDL_Surface *difficile;
    SDL_Surface *current;
    SDL_Surface *horloge[9]; 
    SDL_Surface *rune[6];   
    SDL_Surface *r[6];      
    SDL_Surface *rune_hover[6]; 
    SDL_Surface *saha;
    SDL_Surface *faux;
    SDL_Surface *temps;
    SDL_Surface *difficultyMessage;
    TTF_Font *font;
    TTF_Font *fontm;
    TTF_Font *fontdiff;
    Mix_Chunk *son;
    SDL_Rect positionimage;
    SDL_Rect positiontexte;
    SDL_Rect positionhint;
    SDL_Rect positionhorloge;
    SDL_Rect positionroto;
    SDL_Rect p_rune[6]; 
    SDL_Rect difficultyPosition;
    int mousex, mousey;
    int quitter;
    int nb;
    int nb_fail;
    int fin_jeu;
    int in_transition;
    int time_up;
    int difficulty;
    int t[6];
    int victoire[6];  // Facile
    int victoire2[6]; // Moyen
    int victoire3[6]; // Difficile
    int (*currentVictoire)[6];
    char difficultyText[20];
    char hintText[100];
    int current_clock;
    SDL_Color difficultyColor;
    SDL_Color textColor;
    SDL_Color hintColor;
    Uint32 gameStartTime;
    Uint32 last_change;
} EnigmeState;

void resetGame(EnigmeState *state);
void choicehint(EnigmeState *state);
int display_screen(SDL_Surface* screen, const char* image_path, Uint32 display_time);

#endif
