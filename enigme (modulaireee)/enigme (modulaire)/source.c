#include "header.h"

int pointInRect(Point p, SDL_Rect r) {
    return (p.x >= r.x) && (p.x < r.x + r.w) && (p.y >= r.y) && (p.y < r.y + r.h);
}

SDL_Surface* loadImage(const char *path) {
    SDL_Surface *img = IMG_Load(path);
    if (!img) printf("Error loading %s: %s\n", path, IMG_GetError());
    return img;
}

void renderCenteredText(SDL_Surface *screen, TTF_Font *font, const char *text, 
                       SDL_Rect rect, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, color);
    if (textSurface) {
        SDL_Rect dstRect = {
            rect.x + (rect.w - textSurface->w)/2,
            rect.y + (rect.h - textSurface->h)/2,
            textSurface->w,
            textSurface->h
        };
        SDL_BlitSurface(textSurface, NULL, screen, &dstRect);
        SDL_FreeSurface(textSurface);
    }
}

// [Rest of the function implementations...]
// (Include all the other functions from the original code here)

int initGame(SDL_Surface **screen, TTF_Font **font) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }
    
    *screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_SWSURFACE);
    if (!*screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        return 0;
    }
    
    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return 0;
    }
    
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return 0;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return 0;
    }
    
    Mix_AllocateChannels(16);
    beepSound = Mix_LoadWAV("beep.wav");
    hoverSound = Mix_LoadWAV("hover.wav");
    enigmeMusic = Mix_LoadMUS("enigme.wav");
    winMusic = Mix_LoadMUS("win.wav");
    
    if (!beepSound || !hoverSound) {
        printf("Warning: Could not load sound effects\n");
    }
    if (!enigmeMusic) {
        printf("Warning: Could not load enigme music\n");
    }
    if (!winMusic) {
        printf("Warning: Could not load win music\n");
    }
    
    *font = TTF_OpenFont("arial.ttf", 24);
    if (!*font) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        return 0;
    }

    return 1;
}

void cleanupGame(SDL_Surface *menuBg, SDL_Surface *quizButton, SDL_Surface *quizButtonHover,
                SDL_Surface *puzzleButton, SDL_Surface *puzzleButtonHover, TTF_Font *font) {
    if (beepSound) Mix_FreeChunk(beepSound);
    if (hoverSound) Mix_FreeChunk(hoverSound);
    if (enigmeMusic) Mix_FreeMusic(enigmeMusic);
    if (winMusic) Mix_FreeMusic(winMusic);
    Mix_CloseAudio();
    SDL_FreeSurface(menuBg);
    SDL_FreeSurface(quizButton);
    SDL_FreeSurface(quizButtonHover);
    SDL_FreeSurface(puzzleButton);
    SDL_FreeSurface(puzzleButtonHover);
    TTF_CloseFont(font);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}
