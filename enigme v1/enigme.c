#include "enigme.h"

void resetGame(GameState *state) {
    state->nb = 0;
    memset(state->t, 0, sizeof(int) * 6);
}

void choicehint(GameState *state) {
    if (state->difficulty == 0) { // Facile 
        strcpy(state->hintText, "les 4 premiers clicks sont croissants");
    }
    else if (state->difficulty == 1) { // Moyen
        strcpy(state->hintText, "Indice: 3 clicks successifs se repetent");
    }
    else if (state->difficulty == 2) { // Difficile
        strcpy(state->hintText, "Indice: 5 -> 4 -> 6 et 2 se repete");
    }
}

int display_screen(SDL_Surface* screen, const char* image_path, Uint32 display_time) {
    SDL_Surface* image = IMG_Load(image_path);
    if (!image) {
        printf("Error loading %s: %s\n", image_path, IMG_GetError());
        return -1;
    }

    Uint32 start = SDL_GetTicks();
    SDL_Event event;
    int quit = 0;
    
    while (!quit && (SDL_GetTicks() - start < display_time)) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = 1;
            }
        }
        
        SDL_BlitSurface(image, NULL, screen, NULL);
        SDL_Flip(screen);
        SDL_Delay(10);
    }
    
    SDL_FreeSurface(image);
    return quit ? 1 : 0;
}
