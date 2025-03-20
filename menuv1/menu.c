#include "menu.h"

int main() {
    return mainMenu();
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
