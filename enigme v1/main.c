#include "enigme.h"

int main(int argc, char** argv) {
    SDL_Event event;
    GameState state = {0}; 
    int i, test;
    srand(time(NULL));

   
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Échec d'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Échec d'initialisation de SDL_ttf : %s\n", TTF_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Erreur lors de l'initialisation de l'audio : %s\n", Mix_GetError());
        return 1;
    }

   
    state.ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (state.ecran == NULL) {
        printf("Échec de création de la fenêtre : %s\n", SDL_GetError());
        return 1;
    }
    
    
    if (display_screen(state.ecran, "runes-meaning.png", 10000) != 0) return 1;

    
    state.image = IMG_Load("enigmeback.png");
    if (state.image == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    state.saha = IMG_Load("saha.png");
    if (state.saha == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    state.faux = IMG_Load("Faux.png");
    if (state.faux == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    state.temps = IMG_Load("temps.png");
    if (state.temps == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }

    char horloge_paths[9][20] = {
        "horloge1.0.png", "horloge1.1.png", "horloge1.2.png", "horloge1.3.png",
        "horloge1.4.png", "horloge1.5.png", "horloge1.6.png", "horloge1.7.png",
        "horloge1.8.png"
    };
    for (i = 0; i < 9; i++) {
        state.horloge[i] = IMG_Load(horloge_paths[i]);
        if (state.horloge[i] == NULL) {
            printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
            return 1;
        }
    }

   
    char rune_files[6][10] = {"f.png", "b.png", "v.png", "g.png", "t.png", "r.png"};
    char rune_hover_files[6][15] = {
        "f_hover.png", "b_hover.png", "v_hover.png",
        "g_hover.png", "t_hover.png", "r_hover.png"
    };
    for (i = 0; i < 6; i++) {
        state.rune[i] = IMG_Load(rune_files[i]);
        if (state.rune[i] == NULL) {
            printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
            return 1;
        }
        state.rune_hover[i] = IMG_Load(rune_hover_files[i]);
        if (state.rune_hover[i] == NULL) {
            printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
            return 1;
        }
        state.r[i] = state.rune[i]; 
    }

    state.fontdiff = TTF_OpenFont("DalekPinpointBold.ttf", 50);
    if (state.fontdiff == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }
    state.font = TTF_OpenFont("arial.ttf", 50);
    if (state.font == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }
    state.fontm = TTF_OpenFont("arial.ttf", 40);
    if (state.fontm == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }

    state.facile = IMG_Load("facile.png");
    if (state.facile == NULL) {
        printf("Error loading facile: %s\n", IMG_GetError());
        return 1;
    }
    state.moyen = IMG_Load("moyen.png");
    if (state.moyen == NULL) {
        printf("Error loading moyen: %s\n", IMG_GetError());
        return 1;
    }
    state.difficile = IMG_Load("difficile.png");
    if (state.difficile == NULL) {
        printf("Error loading difficile: %s\n", IMG_GetError());
        return 1;
    }

    state.son = Mix_LoadWAV("click-8.wav");
    if (state.son == NULL) {
        printf("Erreur de chargement du son : %s\n", Mix_GetError());
        return 1;
    }


    state.quitter = 1;
    state.difficulty = rand() % 3;
    switch (state.difficulty) {
        case 0:
            state.currentVictoire = &state.victoire;
            strcpy(state.difficultyText, "Facile");
            state.current = state.facile;
            memcpy(state.victoire, (int[]){2, 3, 4, 5, 1, 6}, sizeof(int) * 6);
            break;
        case 1:
            state.currentVictoire = &state.victoire2;
            strcpy(state.difficultyText, "Moyen");
            state.current = state.moyen;
            memcpy(state.victoire2, (int[]){1, 1, 2, 2, 3, 3}, sizeof(int) * 6);
            break;
        case 2:
            state.currentVictoire = &state.victoire3;
            strcpy(state.difficultyText, "Difficile");
            state.current = state.difficile;
            memcpy(state.victoire3, (int[]){4, 5, 6, 3, 2, 2}, sizeof(int) * 6);
            break;
    }


    state.difficultyColor.r = 255;
    state.difficultyColor.g = 0;
    state.difficultyColor.b = 0;
    state.textColor.r = 0;
    state.textColor.g = 0;
    state.textColor.b = 0;
    state.hintColor.r = 255;
    state.hintColor.g = 165;
    state.hintColor.b = 0;
    state.difficultyMessage = TTF_RenderText_Solid(state.fontdiff, state.difficultyText, state.difficultyColor);

    state.positionimage.x = 0;
    state.positionimage.y = 0;
    state.positiontexte.x = 500;
    state.positiontexte.y = 150;
    state.positionhint.x = 200;
    state.positionhint.y = 350;
    state.positionhorloge.x = 200;
    state.positionhorloge.y = 400;
    state.positionroto.x = 10;
    state.positionroto.y = 0;
    state.difficultyPosition.x = 600;
    state.difficultyPosition.y = 57;

    state.p_rune[0].x = 270;
    state.p_rune[0].y = 200;
    state.p_rune[1].x = 412;
    state.p_rune[1].y = 205;
    state.p_rune[2].x = 560;
    state.p_rune[2].y = 210;
    state.p_rune[3].x = 700;
    state.p_rune[3].y = 210;
    state.p_rune[4].x = 855;
    state.p_rune[4].y = 210;
    state.p_rune[5].x = 1001;
    state.p_rune[5].y = 215;


    state.gameStartTime = SDL_GetTicks();
    state.last_change = SDL_GetTicks();


    while (state.quitter) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsed = currentTime - state.gameStartTime;

        if (!state.fin_jeu && SDL_GetTicks() - state.last_change > 4000) {
            state.current_clock = (state.current_clock + 1) % 9;
            state.last_change = SDL_GetTicks();
            if (state.current_clock == 8) {
                state.time_up = 1;
            }
        }

        SDL_FillRect(state.ecran, NULL, SDL_MapRGB(state.ecran->format, 255, 255, 255));
        SDL_BlitSurface(state.image, NULL, state.ecran, &state.positionimage);
        SDL_BlitSurface(state.horloge[state.current_clock], NULL, state.ecran, &state.positionhorloge);
        
        if (elapsed < 4000) {
            SDL_BlitSurface(state.current, NULL, state.ecran, &state.positionroto);
        } else {
            if (state.difficultyMessage) {
                SDL_BlitSurface(state.difficultyMessage, NULL, state.ecran, &state.difficultyPosition);
            }
            for (i = 0; i < 6; i++) {
                SDL_BlitSurface(state.r[i], NULL, state.ecran, &state.p_rune[i]);
            }
            if (state.hint && state.nb_fail >= 3) {
                SDL_BlitSurface(state.hint, NULL, state.ecran, &state.positionhint);
            }

            if (state.time_up) {
                SDL_BlitSurface(state.temps, NULL, state.ecran, &state.positionroto);
                SDL_BlitSurface(state.message, NULL, state.ecran, &state.positiontexte);
                SDL_Flip(state.ecran);
                SDL_Delay(2000);

                resetGame(&state);
                state.fin_jeu = 0;
                state.current_clock = 0;
                state.time_up = 0;
                state.last_change = SDL_GetTicks();

                if (state.difficultyMessage) SDL_FreeSurface(state.difficultyMessage);
                state.difficultyMessage = TTF_RenderText_Solid(state.font, state.difficultyText, state.difficultyColor);

                if (state.hint) {
                    SDL_FreeSurface(state.hint);
                    state.hint = NULL;
                }

                if (state.message) SDL_FreeSurface(state.message);
                state.message = NULL;
                continue;
            }
        }

        SDL_GetMouseState(&state.mousex, &state.mousey);
        for (i = 0; i < 6; i++) {
            state.r[i] = state.rune[i];
        }

        if (state.mousex >= 270 && state.mousex <= 320 && state.mousey >= 220 && state.mousey <= 315) {
            state.r[0] = state.rune_hover[0];
        }
        if (state.mousex >= 410 && state.mousex <= 515 && state.mousey >= 220 && state.mousey <= 310) {
            state.r[1] = state.rune_hover[1];
        }
        if (state.mousex >= 550 && state.mousex <= 615 && state.mousey >= 220 && state.mousey <= 320) {
            state.r[2] = state.rune_hover[2];
        }
        if (state.mousex >= 700 && state.mousex <= 760 && state.mousey >= 215 && state.mousey <= 330) {
            state.r[3] = state.rune_hover[3];
        }
        if (state.mousex >= 845 && state.mousex <= 920 && state.mousey >= 215 && state.mousey <= 320) {
            state.r[4] = state.rune_hover[4];
        }
        if (state.mousex >= 980 && state.mousex <= 1050 && state.mousey >= 215 && state.mousey <= 315) {
            state.r[5] = state.rune_hover[5];
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state.quitter = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) state.quitter = 0;
                     if (elapsed >= 4000 && state.nb < 6) {
                        state.mousex = event.button.x;
                        state.mousey = event.button.y;
                        if (event.key.keysym.sym == SDLK_a){
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 1;
                            state.nb++;
                        }
                        if (event.key.keysym.sym == SDLK_z){
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 2;
                            state.nb++;
                        }
                        if (event.key.keysym.sym == SDLK_e){  
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 3;
                            state.nb++;
                        }
                        if (event.key.keysym.sym == SDLK_r){  
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 4;
                            state.nb++;
                        }
                        if (event.key.keysym.sym == SDLK_t){  
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 5;
                            state.nb++;
                        }  
                        if (event.key.keysym.sym == SDLK_y){  
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 6;
                            state.nb++;
                        } 
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (elapsed >= 4000 && state.nb < 6) {
                        state.mousex = event.button.x;
                        state.mousey = event.button.y;

                        if (state.mousex >= 270 && state.mousex <= 320 && state.mousey >= 220 && state.mousey <= 315) {
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 1;
                            state.nb++;
                        } else if (state.mousex >= 410 && state.mousex <= 515 && state.mousey >= 220 && state.mousey <= 310) {
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 2;
                            state.nb++;
                        } else if (state.mousex >= 550 && state.mousex <= 615 && state.mousey >= 220 && state.mousey <= 320) {
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 3;
                            state.nb++;
                        } else if (state.mousex >= 700 && state.mousex <= 760 && state.mousey >= 215 && state.mousey <= 330) {
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 4;
                            state.nb++;
                        } else if (state.mousex >= 845 && state.mousex <= 920 && state.mousey >= 215 && state.mousey <= 320) {
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 5;
                            state.nb++;
                        } else if (state.mousex >= 980 && state.mousex <= 1050 && state.mousey >= 215 && state.mousey <= 315) {
                            Mix_PlayChannel(-1, state.son, 0);
                            state.t[state.nb] = 6;
                            state.nb++;
                        }
                    }
                    break;
            }
        }

        if (state.nb == 6) {
            test = 1;
            for (i = 0; i < 6; i++) {
                if (state.t[i] != (*state.currentVictoire)[i]) {
                    test = 0;
                    break;
                }
            }

            if (test) {
                state.fin_jeu = 1;
                SDL_BlitSurface(state.saha, NULL, state.ecran, &state.positionroto);
                state.nb_fail = 0;
                if (state.hint) {
                    SDL_FreeSurface(state.hint);
                    state.hint = NULL;
                }
            } else {
                state.nb_fail++;
                if (state.nb_fail >= 3 && !state.hint) {
                    choicehint(&state);
                    state.hint = TTF_RenderText_Solid(state.fontm, state.hintText, state.hintColor);
                }
                SDL_BlitSurface(state.faux, NULL, state.ecran, &state.positionroto);
                SDL_BlitSurface(state.message, NULL, state.ecran, &state.positiontexte);
                if (state.hint) {
                    SDL_BlitSurface(state.hint, NULL, state.ecran, &state.positionhint);
                }
                SDL_Flip(state.ecran);
                SDL_Delay(2000);

                resetGame(&state);
                state.fin_jeu = 0;
                if (state.message) SDL_FreeSurface(state.message);
                state.message = NULL;
                continue;
            }
        }

        if (state.message) {
            SDL_BlitSurface(state.message, NULL, state.ecran, &state.positiontexte);
        }

        SDL_Flip(state.ecran);
        if (state.message) SDL_FreeSurface(state.message);
        state.message = NULL;
    }


    SDL_FreeSurface(state.ecran);
    SDL_FreeSurface(state.facile);
    SDL_FreeSurface(state.moyen);
    SDL_FreeSurface(state.difficile);
    SDL_FreeSurface(state.image);
    for (i = 0; i < 9; i++) {
        SDL_FreeSurface(state.horloge[i]);
    }
    SDL_FreeSurface(state.difficultyMessage);
    for (i = 0; i < 6; i++) {
        SDL_FreeSurface(state.rune[i]);
        SDL_FreeSurface(state.rune_hover[i]);
    }
    SDL_FreeSurface(state.saha);
    SDL_FreeSurface(state.faux);
    SDL_FreeSurface(state.temps);
    TTF_CloseFont(state.font);
    TTF_CloseFont(state.fontm);
    TTF_CloseFont(state.fontdiff);
    Mix_FreeChunk(state.son);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
