#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <stdlib.h>

SDL_Surface* resize_surface(SDL_Surface* surface, int width, int height) {
    SDL_Surface* resized_surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        width,
        height,
        surface->format->BitsPerPixel,
        surface->format->Rmask,
        surface->format->Gmask,
        surface->format->Bmask,
        surface->format->Amask
    );

    if (resized_surface == NULL) {
        printf("Erreur lors de la création de la surface redimensionnée : %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SoftStretch(surface, NULL, resized_surface, NULL);
    return resized_surface;
}

int main(int argc, char** argv) {
    Mix_Music *musique = NULL;
    Mix_Chunk *hoverSound = NULL;
    SDL_Surface *ecran = NULL;
    SDL_Surface *background = NULL;
    SDL_Surface *button_mono = NULL;
    SDL_Surface *button_mono_hover = NULL;
    SDL_Surface *button_multi = NULL;
    SDL_Surface *button_multi_hover = NULL;
    SDL_Surface *button_back = NULL;
    SDL_Surface *button_back_hover = NULL;
    SDL_Surface *resized_background = NULL;
    SDL_Surface *resized_button_mono = NULL;
    SDL_Surface *resized_button_mono_hover = NULL;
    SDL_Surface *resized_button_multi = NULL;
    SDL_Surface *resized_button_multi_hover = NULL;
    SDL_Surface *resized_button_back = NULL;
    SDL_Surface *resized_button_back_hover = NULL;
    SDL_Event event;
    int quitter = 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Échec d'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    hoverSound = Mix_LoadWAV("UI Generic Button Move Hover Fall Guys Sound - Sound Effect for editing.wav");
    if (!hoverSound) {
        printf("Failed to load hover sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    ecran = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (ecran == NULL) {
        printf("Échec de création de la fenêtre : %s\n", SDL_GetError());
        return 1;
    }

    musique = Mix_LoadMUS("Assassins Creed 2 OST _ Jesper Kyd - Ezios Family Track 03.mp3");
    if (!musique) {
        printf("Erreur musique: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(musique, -1);
    }

    background = IMG_Load("background.png");
    button_mono = IMG_Load("button mono.png");
    button_mono_hover = IMG_Load("hover mono.png");
    button_multi = IMG_Load("multi.png");
    button_multi_hover = IMG_Load("multi hove.png");
    button_back = IMG_Load("retour.png");
    button_back_hover = IMG_Load("retour hover.png");

    if (!background || !button_mono || !button_mono_hover || !button_multi || !button_multi_hover || !button_back || !button_back_hover) {
        printf("Erreur de chargement des images : %s\n", IMG_GetError());
        return 1;
    }

    resized_background = resize_surface(background, 1920, 1080);
    int button_width = 400;
    int button_height = 200;
    resized_button_mono = resize_surface(button_mono, button_width, button_height);
    resized_button_mono_hover = resize_surface(button_mono_hover, button_width, button_height);
    resized_button_multi = resize_surface(button_multi, button_width, button_height);
    resized_button_multi_hover = resize_surface(button_multi_hover, button_width, button_height);
    resized_button_back = resize_surface(button_back, button_width, button_height);
    resized_button_back_hover = resize_surface(button_back_hover, button_width, button_height);

    if (!resized_background || !resized_button_mono || !resized_button_mono_hover || !resized_button_multi || !resized_button_multi_hover || !resized_button_back || !resized_button_back_hover) {
        printf("Erreur lors du redimensionnement des images : %s\n", SDL_GetError());
        return 1;
    }

    SDL_FreeSurface(background);
    SDL_FreeSurface(button_mono);
    SDL_FreeSurface(button_mono_hover);
    SDL_FreeSurface(button_multi);
    SDL_FreeSurface(button_multi_hover);
    SDL_FreeSurface(button_back);
    SDL_FreeSurface(button_back_hover);

    SDL_Rect position_background = {0, 0};
    int gap = 20;
    int total_width = button_width * 2 + gap;
    int start_x = (1920 - total_width) / 2;
    int start_y = (1080 - button_height) / 2;
    SDL_Rect position_mono = {start_x, start_y};
    SDL_Rect position_multi = {start_x + button_width + gap, start_y};
    SDL_Rect position_back = {1500, 1080 - button_height - 20};

    SDL_Rect hitbox_mono = position_mono;
    SDL_Rect hitbox_multi = position_multi;
    SDL_Rect hitbox_back = position_back;

    hitbox_mono.w = button_width;
    hitbox_mono.h = button_height;
    hitbox_multi.w = button_width;
    hitbox_multi.h = button_height;
    hitbox_back.w = button_width;
    hitbox_back.h = button_height;

    SDL_Surface* current_button_mono = resized_button_mono;
    SDL_Surface* current_button_multi = resized_button_multi;
    SDL_Surface* current_button_back = resized_button_back;

    while (quitter) {
        SDL_BlitSurface(resized_background, NULL, ecran, &position_background);
        SDL_BlitSurface(current_button_mono, NULL, ecran, &position_mono);
        SDL_BlitSurface(current_button_multi, NULL, ecran, &position_multi);
        SDL_BlitSurface(current_button_back, NULL, ecran, &position_back);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quitter = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quitter = 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (event.motion.x >= hitbox_mono.x && event.motion.x <= hitbox_mono.x + hitbox_mono.w &&
                        event.motion.y >= hitbox_mono.y && event.motion.y <= hitbox_mono.y + hitbox_mono.h) {
                        if (current_button_mono != resized_button_mono_hover) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                        }
                        current_button_mono = resized_button_mono_hover;
                    } else {
                        current_button_mono = resized_button_mono;
                    }

                    if (event.motion.x >= hitbox_multi.x && event.motion.x <= hitbox_multi.x + hitbox_multi.w &&
                        event.motion.y >= hitbox_multi.y && event.motion.y <= hitbox_multi.y + hitbox_multi.h) {
                        if (current_button_multi != resized_button_multi_hover) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                        }
                        current_button_multi = resized_button_multi_hover;
                    } else {
                        current_button_multi = resized_button_multi;
                    }

                    if (event.motion.x >= hitbox_back.x && event.motion.x <= hitbox_back.x + hitbox_back.w &&
                        event.motion.y >= hitbox_back.y && event.motion.y <= hitbox_back.y + hitbox_back.h) {
                        if (current_button_back != resized_button_back_hover) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                        }
                        current_button_back = resized_button_back_hover;
                    } else {
                        current_button_back = resized_button_back;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int x = event.button.x;
                        int y = event.button.y;

                        if (x >= hitbox_mono.x && x <= hitbox_mono.x + hitbox_mono.w &&
                            y >= hitbox_mono.y && y <= hitbox_mono.y + hitbox_mono.h) {
                            printf("Mono Player selected\n");
                            SDL_FreeSurface(resized_background);
                            SDL_FreeSurface(resized_button_mono);
                            SDL_FreeSurface(resized_button_mono_hover);
                            SDL_FreeSurface(resized_button_multi);
                            SDL_FreeSurface(resized_button_multi_hover);
                            SDL_FreeSurface(resized_button_back);
                            SDL_FreeSurface(resized_button_back_hover);
                            SDL_Quit();
                            system("./chars");
                            quitter = 0;
                        }
                        if (x >= hitbox_multi.x && x <= hitbox_multi.x + hitbox_multi.w &&
                            y >= hitbox_multi.y && y <= hitbox_multi.y + hitbox_multi.h) {
                            printf("Multiplayer selected\n");
                            SDL_FreeSurface(resized_background);
                            SDL_FreeSurface(resized_button_mono);
                            SDL_FreeSurface(resized_button_mono_hover);
                            SDL_FreeSurface(resized_button_multi);
                            SDL_FreeSurface(resized_button_multi_hover);
                            SDL_FreeSurface(resized_button_back);
                            SDL_FreeSurface(resized_button_back_hover);
                            SDL_Quit();
                            system("./charmult");
                            quitter = 0;
                        }
                        if (x >= hitbox_back.x && x <= hitbox_back.x + hitbox_back.w &&
                            y >= hitbox_back.y && y <= hitbox_back.y + hitbox_back.h) {
                            printf("Back button pressed\n");
			    SDL_FreeSurface(resized_background);
                            SDL_FreeSurface(resized_button_mono);
                            SDL_FreeSurface(resized_button_mono_hover);
                            SDL_FreeSurface(resized_button_multi);
                            SDL_FreeSurface(resized_button_multi_hover);
                            SDL_FreeSurface(resized_button_back);
                            SDL_FreeSurface(resized_button_back_hover);
                            SDL_Quit();
                            system("./prog");
                            quitter = 0;
                        }
                    }
                    break;
            }
        }

        SDL_Flip(ecran);
    }

    SDL_FreeSurface(resized_background);
    SDL_FreeSurface(resized_button_mono);
    SDL_FreeSurface(resized_button_mono_hover);
    SDL_FreeSurface(resized_button_multi);
    SDL_FreeSurface(resized_button_multi_hover);
    SDL_FreeSurface(resized_button_back);
    SDL_FreeSurface(resized_button_back_hover);
    Mix_FreeChunk(hoverSound);
    Mix_FreeMusic(musique);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}
