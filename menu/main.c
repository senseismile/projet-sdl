#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function to resize an SDL surface
SDL_Surface* resizeSurface(SDL_Surface* surface, int width, int height) {
    SDL_Surface* resizedSurface = SDL_CreateRGBSurface(
        surface->flags, width, height, surface->format->BitsPerPixel,
        surface->format->Rmask, surface->format->Gmask,
        surface->format->Bmask, surface->format->Amask
    );
    if (!resizedSurface) {
        printf("Error creating resized surface: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_SoftStretch(surface, NULL, resizedSurface, NULL);
    return resizedSurface;
}

// Function to clean up resources
void cleanup(SDL_Surface* image, SDL_Surface* boutons[], SDL_Surface* boutonsH[], Mix_Music* musique, Mix_Chunk* hoverSound, Mix_Chunk* clickSound, TTF_Font* font) {
    SDL_FreeSurface(image);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        SDL_FreeSurface(boutons[i]);
        SDL_FreeSurface(boutonsH[i]);
    }
    if (musique) {
        Mix_HaltMusic();
        Mix_FreeMusic(musique);
    }
    if (hoverSound) {
        Mix_FreeChunk(hoverSound);
    }
    if (clickSound) {
        Mix_FreeChunk(clickSound);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

// Main menu function
int mainMenu() {
    SDL_Surface *ecran = NULL, *image = NULL;
    SDL_Surface *boutons[NUM_BUTTONS] = {NULL}, *boutonsH[NUM_BUTTONS] = {NULL};
    SDL_Event event;
    Mix_Music *musique = NULL;
    Mix_Chunk *hoverSound = NULL;
    Mix_Chunk *clickSound = NULL;
    TTF_Font *font = NULL;
    int quit = 0, survole = -1;
    int prev_survole = -1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("TTF initialization error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Mix_OpenAudio error: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont("greek.ttf", 48);
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    musique = Mix_LoadMUS("Ezios Family.mp3");
    if (!musique) {
        printf("Error loading music: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    Mix_PlayMusic(musique, -1);

    hoverSound = Mix_LoadWAV("UI Generic Button Move Hover Fall Guys Sound - Sound Effect for editing.wav");
    if (!hoverSound) {
        printf("Error loading hover sound: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    clickSound = Mix_LoadWAV("click.wav");
    if (!clickSound) {
        printf("Error loading click sound: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!ecran) {
        printf("Error creating window: %s\n", SDL_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    image = IMG_Load("backg.png");
    if (!image) {
        printf("Error loading image: %s\n", IMG_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    SDL_Surface* resizedImage = resizeSurface(image, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!resizedImage) {
        cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
        return 1;
    }
    SDL_FreeSurface(image);
    image = resizedImage;

    const char *const fichiers[NUM_BUTTONS] = {"jouer.png", "options.png", "score.png", "histoire.png", "quitter.png"};
    const char *const fichiersH[NUM_BUTTONS] = {"jouera.png", "optionsa.png", "scorea.png", "histoirea.png", "quittera.png"};
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        SDL_Surface* temp = IMG_Load(fichiers[i]);
        SDL_Surface* tempH = IMG_Load(fichiersH[i]);
        if (!temp || !tempH) {
            printf("Error loading button: %s\n", fichiers[i]);
            for (int j = 0; j < i; j++) {
                SDL_FreeSurface(boutons[j]);
                SDL_FreeSurface(boutonsH[j]);
            }
            cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
            return 1;
        }

        boutons[i] = resizeSurface(temp, BUTTON_WIDTH, BUTTON_HEIGHT);
        boutonsH[i] = resizeSurface(tempH, BUTTON_WIDTH, BUTTON_HEIGHT);
        SDL_FreeSurface(temp);
        SDL_FreeSurface(tempH);

        if (!boutons[i] || !boutonsH[i]) {
            printf("Error resizing button: %s\n", fichiers[i]);
            cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
            return 1;
        }
    }

    SDL_Rect pos[NUM_BUTTONS];
    int x = 30, y = 150, ecart = 20;
    for (int i = 0; i < NUM_BUTTONS - 1; i++) {
        pos[i].x = x;
        pos[i].y = y + i * (BUTTON_HEIGHT + ecart);
    }

    pos[NUM_BUTTONS - 1].x = ecran->w - BUTTON_WIDTH - 30;
    pos[NUM_BUTTONS - 1].y = pos[NUM_BUTTONS - 2].y;

    SDL_Color textColor = {255, 255, 255}; // White color
    SDL_Rect textPosition = {20, 20, 0, 0}; // Top-left position

    while (!quit) {
        SDL_BlitSurface(image, NULL, ecran, NULL);

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Welcome To", textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, ecran, &textPosition);
            SDL_FreeSurface(textSurface);
        }

        for (int i = 0; i < NUM_BUTTONS; i++) {
            SDL_BlitSurface((i == survole) ? boutonsH[i] : boutons[i], NULL, ecran, &pos[i]);
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }

            if (event.type == SDL_MOUSEMOTION) {
                prev_survole = survole;
                survole = -1;
                for (int i = 0; i < NUM_BUTTONS; i++) {
                    if (event.motion.x >= pos[i].x && event.motion.x < pos[i].x + BUTTON_WIDTH &&
                        event.motion.y >= pos[i].y && event.motion.y < pos[i].y + BUTTON_HEIGHT) {
                        survole = i;
                        break;
                    }
                }

                if (survole != -1 && survole != prev_survole) {
                    Mix_PlayChannel(-1, hoverSound, 0);
                }
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_j) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
                    if (access("./saver", X_OK) == -1) {
                        printf("Error: 'saver' program does not exist or is not executable.\n");
                        return 1;
                    }
                    system("./saver");
                    return 0;
                }
                if (event.key.keysym.sym == SDLK_o) {
                    Mix_PlayChannel(-1, clickSound, 0);
                    cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
                    return optionsMenu(); // Call optionsMenu() instead of system("./options")
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = 1;
                }
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (event.button.x >= pos[0].x && event.button.x < pos[0].x + BUTTON_WIDTH &&
                    event.button.y >= pos[0].y && event.button.y < pos[0].y + BUTTON_HEIGHT) {
                    
                    Mix_PlayChannel(-1, clickSound, 0);
                    cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);

                    if (access("./saver", X_OK) == -1) {
                        printf("Error: 'saver' program does not exist or is not executable.\n");
                        return 1;
                    }
                    system("./saver");
                    return 0;
                }

                if (event.button.x >= pos[1].x && event.button.x < pos[1].x + BUTTON_WIDTH &&
                    event.button.y >= pos[1].y && event.button.y < pos[1].y + BUTTON_HEIGHT) {
                    
                    Mix_PlayChannel(-1, clickSound, 0);
                    cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
                    return optionsMenu(); // Call optionsMenu() instead of system("./options")
                }

                if (event.button.x >= pos[NUM_BUTTONS - 1].x && event.button.x < pos[NUM_BUTTONS - 1].x + BUTTON_WIDTH &&
                    event.button.y >= pos[NUM_BUTTONS - 1].y && event.button.y < pos[NUM_BUTTONS - 1].y + BUTTON_HEIGHT) {
                    quit = 1;
                }
            }
        }

        SDL_Flip(ecran);
        SDL_Delay(16);
    }

    cleanup(image, boutons, boutonsH, musique, hoverSound, clickSound, font);
    return 0;
}
