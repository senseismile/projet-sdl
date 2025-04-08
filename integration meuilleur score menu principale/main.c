#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int saveMenu();

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

    musique = Mix_LoadMUS("Assassins Creed 2 OST _ Jesper Kyd - Ezios Family Track 03.mp3");
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
			else if (event.button.button==SDL_BUTTON_LEFT && (event.motion.y<=485 && event.motion.y>=397 
								   && event.motion.x<=305 && event.motion.x>=58))  
			{
                   			score(ecran);
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
                    saveMenu();
		    return 0;
                }
		 if (event.key.keysym.sym == SDLK_m) {
                   			score(ecran);
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
                    return saveMenu();
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
int optionsMenu() {
    SDL_Surface *ecran = NULL, *volume0 = NULL;
    SDL_Surface *volume32 = NULL, *volume64 = NULL, *volume96 = NULL, *volume128 = NULL;
    SDL_Surface *bouton = NULL, *retour = NULL, *bouton_hover = NULL;
    SDL_Surface *actuel = NULL;
    SDL_Surface *hover_fenetre = NULL, *hover_pe = NULL, *hover_retour = NULL;
    SDL_Event event;
    Mix_Music *musique = NULL;
    Mix_Chunk *clickSound = NULL;
    TTF_Font *font = NULL, *font_retour = NULL;
    int quit = 0;
    int is_fs = 0;
    int mousex, mousey;

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

    font = TTF_OpenFont("greek.ttf", 60);
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    font_retour = TTF_OpenFont("greek.ttf", 45);
    if (!font_retour) {
        printf("Error loading font: %s\n", TTF_GetError());
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

    // Load images
    volume0 = IMG_Load("volume0.png");
    if (!volume0) {
        printf("Error loading volume0.png: %s\n", IMG_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    volume32 = IMG_Load("volume32.png");
    if (!volume32) {
        printf("Error loading volume32.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    volume64 = IMG_Load("volume64.png");
    if (!volume64) {
        printf("Error loading volume64.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    volume96 = IMG_Load("volume96.png");
    if (!volume96) {
        printf("Error loading volume96.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    volume128 = IMG_Load("volume128.png");
    if (!volume128) {
        printf("Error loading volume128.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    bouton = IMG_Load("bouton.png");
    if (!bouton) {
        printf("Error loading bouton.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    retour = IMG_Load("retour.png");
    if (!retour) {
        printf("Error loading retour.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    bouton_hover = IMG_Load("bouton_hover.png");
    if (!bouton_hover) {
        printf("Error loading bouton_hover.png: %s\n", IMG_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    clickSound = Mix_LoadWAV("click.wav");
    if (!clickSound) {
        printf("Error loading click sound: %s\n", Mix_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }

    musique = Mix_LoadMUS("Ezios Family.mp3");
    if (!musique) {
        printf("Error loading music: %s\n", Mix_GetError());
        cleanup(volume0, NULL, NULL, musique, NULL, clickSound, font);
        return 1;
    }
    Mix_PlayMusic(musique, -1);

    // Rescale images to fit the screen
    volume0 = resizeSurface(volume0, SCREEN_WIDTH, SCREEN_HEIGHT);
    volume32 = resizeSurface(volume32, SCREEN_WIDTH, SCREEN_HEIGHT);
    volume64 = resizeSurface(volume64, SCREEN_WIDTH, SCREEN_HEIGHT);
    volume96 = resizeSurface(volume96, SCREEN_WIDTH, SCREEN_HEIGHT);
    volume128 = resizeSurface(volume128, SCREEN_WIDTH, SCREEN_HEIGHT);
    bouton = resizeSurface(bouton, BUTTON_WIDTH, BUTTON_HEIGHT);
    retour = resizeSurface(retour, BUTTON_WIDTH, BUTTON_HEIGHT);
    bouton_hover = resizeSurface(bouton_hover, BUTTON_WIDTH, BUTTON_HEIGHT);

    actuel = volume0;
    hover_fenetre = bouton;
    hover_pe = bouton;
    hover_retour = bouton;

    // Define positions and sizes for buttons and text
    SDL_Rect positionimage = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect bouton_fenetre = {SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.45, BUTTON_WIDTH, BUTTON_HEIGHT};
    SDL_Rect bouton_fs = {SCREEN_WIDTH * 0.55, SCREEN_HEIGHT * 0.45, BUTTON_WIDTH, BUTTON_HEIGHT};
    SDL_Rect position_fenetre = {SCREEN_WIDTH * 0.12, SCREEN_HEIGHT * 0.5, 0, 0};
    SDL_Rect position_plein = {SCREEN_WIDTH * 0.6, SCREEN_HEIGHT * 0.5, 0, 0};
    SDL_Rect position_ecran = {SCREEN_WIDTH * 0.6, SCREEN_HEIGHT * 0.55, 0, 0};
    SDL_Rect bouton_retour = {SCREEN_WIDTH * 0.8, SCREEN_HEIGHT * 0.8, BUTTON_WIDTH, BUTTON_HEIGHT};
    SDL_Rect position_retour = {SCREEN_WIDTH * 0.82, SCREEN_HEIGHT * 0.82, 0, 0};
    SDL_Rect hitbox_retour = {SCREEN_WIDTH * 0.8, SCREEN_HEIGHT * 0.8, BUTTON_WIDTH, BUTTON_HEIGHT};

    while (!quit) {
        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));

        SDL_BlitSurface(actuel, NULL, ecran, &positionimage);
        SDL_BlitSurface(hover_fenetre, NULL, ecran, &bouton_fenetre);
        SDL_BlitSurface(hover_pe, NULL, ecran, &bouton_fs);
        SDL_BlitSurface(hover_retour, NULL, ecran, &bouton_retour);

        SDL_Surface* fenetre = TTF_RenderText_Solid(font, "Fenetre", (SDL_Color){0, 0, 0});
        if (fenetre) {
            SDL_BlitSurface(fenetre, NULL, ecran, &position_fenetre);
            SDL_FreeSurface(fenetre);
        }

        SDL_Surface* plein = TTF_RenderText_Solid(font, "Plein", (SDL_Color){0, 0, 0});
        if (plein) {
            SDL_BlitSurface(plein, NULL, ecran, &position_plein);
            SDL_FreeSurface(plein);
        }

        SDL_Surface* t_ecran = TTF_RenderText_Solid(font, "Ecran", (SDL_Color){0, 0, 0});
        if (t_ecran) {
            SDL_BlitSurface(t_ecran, NULL, ecran, &position_ecran);
            SDL_FreeSurface(t_ecran);
        }

        SDL_Surface* t_retour = TTF_RenderText_Solid(font_retour, "Retour", (SDL_Color){0, 0, 0});
        if (t_retour) {
            SDL_BlitSurface(t_retour, NULL, ecran, &position_retour);
            SDL_FreeSurface(t_retour);
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                    }
                    if (event.key.keysym.sym == SDLK_f) {
                        is_fs = !is_fs;
                        ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | (is_fs ? SDL_FULLSCREEN : 0));
                    }
                    if (event.key.keysym.sym == SDLK_UP) {
                        if (actuel == volume0) {
                            actuel = volume32;
                        } else if (actuel == volume32) {
                            actuel = volume64;
                        } else if (actuel == volume64) {
                            actuel = volume96;
                        } else if (actuel == volume96) {
                            actuel = volume128;
                        }
                    }
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        if (actuel == volume128) {
                            actuel = volume96;
                        } else if (actuel == volume96) {
                            actuel = volume64;
                        } else if (actuel == volume64) {
                            actuel = volume32;
                        } else if (actuel == volume32) {
                            actuel = volume0;
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (event.button.x > bouton_fenetre.x && event.button.x < bouton_fenetre.x + bouton_fenetre.w &&
                            event.button.y > bouton_fenetre.y && event.button.y < bouton_fenetre.y + bouton_fenetre.h) {
                            is_fs = !is_fs;
                            ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | (is_fs ? SDL_FULLSCREEN : 0));
                            Mix_PlayChannel(-1, clickSound, 0);
                        }
                        if (event.button.x > bouton_fs.x && event.button.x < bouton_fs.x + bouton_fs.w &&
                            event.button.y > bouton_fs.y && event.button.y < bouton_fs.y + bouton_fs.h) {
                            is_fs = !is_fs;
                            ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | (is_fs ? SDL_FULLSCREEN : 0));
                            Mix_PlayChannel(-1, clickSound, 0);
                        }

                        if (event.button.x > SCREEN_WIDTH * 0.1 && event.button.x < SCREEN_WIDTH * 0.1 + BUTTON_WIDTH &&
                            event.button.y > SCREEN_HEIGHT * 0.2 && event.button.y < SCREEN_HEIGHT * 0.2 + BUTTON_HEIGHT) {
                            actuel = volume0;
                            Mix_VolumeMusic(0);
                            Mix_PlayChannel(-1, clickSound, 0);
                        }

                        if (event.button.x > SCREEN_WIDTH * 0.3 && event.button.x < SCREEN_WIDTH * 0.3 + BUTTON_WIDTH &&
                            event.button.y > SCREEN_HEIGHT * 0.2 && event.button.y < SCREEN_HEIGHT * 0.2 + BUTTON_HEIGHT) {
                            actuel = volume32;
                            Mix_VolumeMusic(32);
                            Mix_PlayChannel(-1, clickSound, 0);
                        }
                        if (event.button.x > SCREEN_WIDTH * 0.5 && event.button.x < SCREEN_WIDTH * 0.5 + BUTTON_WIDTH &&
                            event.button.y > SCREEN_HEIGHT * 0.2 && event.button.y < SCREEN_HEIGHT * 0.2 + BUTTON_HEIGHT) {
                            actuel = volume64;
                            Mix_VolumeMusic(64);
                            Mix_PlayChannel(-1, clickSound, 0);
                        }
                        if (event.button.x > SCREEN_WIDTH * 0.7 && event.button.x < SCREEN_WIDTH * 0.7 + BUTTON_WIDTH &&
                            event.button.y > SCREEN_HEIGHT * 0.2 && event.button.y < SCREEN_HEIGHT * 0.2 + BUTTON_HEIGHT) {
                            actuel = volume96;
                            Mix_VolumeMusic(96);
                            Mix_PlayChannel(-1, clickSound, 0);
                        }
                        if (event.button.x > SCREEN_WIDTH * 0.9 && event.button.x < SCREEN_WIDTH * 0.9 + BUTTON_WIDTH &&
                            event.button.y > SCREEN_HEIGHT * 0.2 && event.button.y < SCREEN_HEIGHT * 0.2 + BUTTON_HEIGHT) {
                            actuel = volume128;
                            Mix_VolumeMusic(120);
                            Mix_PlayChannel(-1, clickSound, 0);
                        }

                        if (event.button.x >= hitbox_retour.x && event.button.x <= hitbox_retour.x + hitbox_retour.w &&
                            event.button.y >= hitbox_retour.y && event.button.y <= hitbox_retour.y + hitbox_retour.h) {
                            quit = 1;
                        }
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mousex = event.motion.x;
                    mousey = event.motion.y;
                    if (mousex > bouton_fenetre.x && mousex < bouton_fenetre.x + bouton_fenetre.w &&
                        mousey > bouton_fenetre.y && mousey < bouton_fenetre.y + bouton_fenetre.h) {
                        hover_fenetre = bouton_hover;
                    } else {
                        hover_fenetre = bouton;
                    }

                    if (mousex > bouton_fs.x && mousex < bouton_fs.x + bouton_fs.w &&
                        mousey > bouton_fs.y && mousey < bouton_fs.y + bouton_fs.h) {
                        hover_pe = bouton_hover;
                    } else {
                        hover_pe = bouton;
                    }

                    if (mousex > hitbox_retour.x && mousex < hitbox_retour.x + hitbox_retour.w &&
                        mousey > hitbox_retour.y && mousey < hitbox_retour.y + hitbox_retour.h) {
                        hover_retour = bouton_hover;
                    } else {
                        hover_retour = bouton;
                    }
                    break;
            }
        }

        SDL_Flip(ecran);
    }

    // Cleanup
    SDL_FreeSurface(volume0);
    SDL_FreeSurface(volume32);
    SDL_FreeSurface(volume64);
    SDL_FreeSurface(volume96);
    SDL_FreeSurface(volume128);
    SDL_FreeSurface(bouton);
    SDL_FreeSurface(retour);
    SDL_FreeSurface(bouton_hover);
    Mix_FreeMusic(musique);
    Mix_FreeChunk(clickSound);
    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_CloseFont(font_retour);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
int saveMenu() {
    int screenWidth = 983;
int screenHeight = 560;
Mix_Music* guiMusic = NULL;
Mix_Music* bgMusic = NULL;
int lastHovered = 0;

SDL_Surface* resizeImage(SDL_Surface* image) {
    if (!image) {
        fprintf(stderr, "Error: Image is NULL during resizing.\n");
        return NULL;
    }
    SDL_Surface* resized = zoomSurface(image, (float)screenWidth / image->w, (float)screenHeight / image->h, SMOOTHING_ON);
    if (!resized) {
        fprintf(stderr, "Error: Resizing failed.\n");
    }
    return resized;
}

void playHoverSoundOnce(int hoverIndex) {
    if (lastHovered != hoverIndex) {
        Mix_HaltMusic();
        Mix_PlayMusic(guiMusic, 0);
        lastHovered = hoverIndex;
    }
}

void stopHoverSound() {
    if (lastHovered != 0) {
        Mix_HaltMusic();
        lastHovered = 0;
        Mix_PlayMusic(bgMusic, -1);
    }
}
SDL_Surface *ecran = NULL, *image = NULL, *imageB = NULL, *imageD = NULL, *hoverImage1 = NULL, *hoverImage2 = NULL, *hoverImage3 = NULL, *hoverImage4 = NULL;
    int quitter = 1, isOnImageB = 0, wasImageD = 0;
    SDL_Rect poscranimg = {0, 0};
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() == -1) return 1;
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) return 1;

    guiMusic = Mix_LoadMUS("click.mp3");
    if (!guiMusic) {
        fprintf(stderr, "Failed to load hover sound: %s\n", Mix_GetError());
        return 1;
    }

    bgMusic = Mix_LoadMUS("Assassins Creed 2 OST _ Jesper Kyd - Ezios Family Track 03.mp3");
    if (!bgMusic) {
        fprintf(stderr, "Failed to load background music: %s\n", Mix_GetError());
        return 1;
    }

    Mix_PlayMusic(bgMusic, -1);

    ecran = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!ecran) return 1;

    image = resizeImage(IMG_Load("sauv.png"));
    imageB = resizeImage(IMG_Load("r1.png"));
    imageD = resizeImage(IMG_Load("charg.png"));
    hoverImage1 = resizeImage(IMG_Load("4.png"));
    hoverImage2 = resizeImage(IMG_Load("2.png"));
    hoverImage3 = resizeImage(IMG_Load("11.png"));
    hoverImage4 = resizeImage(IMG_Load("3.png"));
    if (!image || !imageB || !imageD || !hoverImage1 || !hoverImage2 || !hoverImage3 || !hoverImage4) return 1;

    int mouseX = 0, mouseY = 0;
    int imageD_x1 = screenWidth * 0.08, imageD_x2 = screenWidth * 0.37;
    int imageD_y1 = screenHeight * 0.68, imageD_y2 = screenHeight * 0.88;
    int imageB_x1 = screenWidth * 0.6, imageB_x2 = screenWidth * 0.85;
    int imageB_y1 = screenHeight * 0.7, imageB_y2 = screenHeight * 0.9;
    int hoverImage3_x1 = 650, hoverImage3_x2 = 897;
    int hoverImage3_y1 = 389, hoverImage3_y2 = 496;
    int hoverImage4_x1 = 543, hoverImage4_x2 = 790;
    int hoverImage4_y1 = 160, hoverImage4_y2 = 263;

    while (quitter) {
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT: quitter = 0; break;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&mouseX, &mouseY);
                if (!isOnImageB) {
                    if (!wasImageD && mouseX >= imageD_x1 && mouseX <= imageD_x2 && mouseY >= imageD_y1 && mouseY <= imageD_y2) {
                        image = hoverImage1;
                        playHoverSoundOnce(1);
                    } else if (wasImageD && mouseX >= imageB_x1 && mouseX <= imageB_x2 && mouseY >= imageB_y1 && mouseY <= imageB_y2) {
                        image = hoverImage2;
                        playHoverSoundOnce(2);
                    } else if (!wasImageD && mouseX >= hoverImage3_x1 && mouseX <= hoverImage3_x2 && mouseY >= hoverImage3_y1 && mouseY <= hoverImage3_y2) {
                        image = hoverImage3;
                        playHoverSoundOnce(3);
                    } else if (wasImageD && mouseX >= hoverImage4_x1 && mouseX <= hoverImage4_x2 && mouseY >= hoverImage4_y1 && mouseY <= hoverImage4_y2) {
                        image = hoverImage4;
                        playHoverSoundOnce(4);
                    } else {
                        image = wasImageD ? imageD : resizeImage(IMG_Load("sauv.png"));
                        stopHoverSound();
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (!wasImageD && mouseX >= imageD_x1 && mouseX <= imageD_x2 && mouseY >= imageD_y1 && mouseY <= imageD_y2) {
                        image = imageD;
                        wasImageD = 1;
                    } else if (wasImageD && mouseX >= imageB_x1 && mouseX <= imageB_x2 && mouseY >= imageB_y1 && mouseY <= imageB_y2) {
                        SDL_Quit();
                        system("./jouer");
                        quitter = 0;
                        isOnImageB = 1;
                        wasImageD = 0;
                    }
                }
                break;
        }
        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
        SDL_BlitSurface(image, NULL, ecran, &poscranimg);
        SDL_Flip(ecran);
    }

    Mix_FreeMusic(guiMusic);
    Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}

