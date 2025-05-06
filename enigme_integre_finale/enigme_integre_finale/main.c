#include "header.h"
#include "enigme.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <unistd.h> 

void runEnigmeGame(SDL_Surface *screen, TTF_Font *font, TTF_Font *fontm, TTF_Font *fontdiff) {
    EnigmeState enigmeState = {0};
    srand(time(NULL));
    enigmeState.quitter = 1;
    enigmeState.difficulty = rand() % 3;
    enigmeState.message = NULL;

    enigmeState.ecran = screen;
    enigmeState.image = IMG_Load("enigmeback.png");
    enigmeState.saha = IMG_Load("saha.png");
    enigmeState.faux = IMG_Load("Faux.png");
    enigmeState.temps = IMG_Load("temps.png");
    if (!enigmeState.image || !enigmeState.saha || !enigmeState.faux || !enigmeState.temps) {
        printf("Failed to load enigme assets\n");
        return;
    }

    char horloge_paths[9][20] = {
        "horloge1.0.png", "horloge1.1.png", "horloge1.2.png", "horloge1.3.png",
        "horloge1.4.png", "horloge1.5.png", "horloge1.6.png", "horloge1.7.png",
        "horloge1.8.png"
    };
    for (int i = 0; i < 9; i++) {
        enigmeState.horloge[i] = IMG_Load(horloge_paths[i]);
        if (!enigmeState.horloge[i]) {
            printf("Failed to load %s\n", horloge_paths[i]);
            return;
        }
    }

    char rune_files[6][10] = {"f.png", "b.png", "v.png", "g.png", "t.png", "r.png"};
    char rune_hover_files[6][15] = {
        "f_hover.png", "b_hover.png", "v_hover.png",
        "g_hover.png", "t_hover.png", "r_hover.png"
    };
    for (int i = 0; i < 6; i++) {
        enigmeState.rune[i] = IMG_Load(rune_files[i]);
        enigmeState.rune_hover[i] = IMG_Load(rune_hover_files[i]);
        if (!enigmeState.rune[i] || !enigmeState.rune_hover[i]) {
            printf("Failed to load rune %d\n", i);
            return;
        }
        enigmeState.r[i] = enigmeState.rune[i];
    }

    enigmeState.facile = IMG_Load("facile.png");
    enigmeState.moyen = IMG_Load("moyen.png");
    enigmeState.difficile = IMG_Load("difficile.png");
    if (!enigmeState.facile || !enigmeState.moyen || !enigmeState.difficile) {
        printf("Failed to load difficulty images\n");
        return;
    }

    enigmeState.son = Mix_LoadWAV("click-8.wav");
    if (!enigmeState.son) {
        printf("Failed to load click sound\n");
        return;
    }

    enigmeState.font = font;
    enigmeState.fontm = fontm;
    enigmeState.fontdiff = fontdiff;

    switch (enigmeState.difficulty) {
        case 0:
            enigmeState.currentVictoire = &enigmeState.victoire;
            strcpy(enigmeState.difficultyText, "Facile");
            enigmeState.current = enigmeState.facile;
            memcpy(enigmeState.victoire, (int[]){2, 3, 4, 5, 1, 6}, sizeof(int) * 6);
            break;
        case 1:
            enigmeState.currentVictoire = &enigmeState.victoire2;
            strcpy(enigmeState.difficultyText, "Moyen");
            enigmeState.current = enigmeState.moyen;
            memcpy(enigmeState.victoire2, (int[]){1, 1, 2, 2, 3, 3}, sizeof(int) * 6);
            break;
        case 2:
            enigmeState.currentVictoire = &enigmeState.victoire3;
            strcpy(enigmeState.difficultyText, "Difficile");
            enigmeState.current = enigmeState.difficile;
            memcpy(enigmeState.victoire3, (int[]){4, 5, 6, 3, 2, 2}, sizeof(int) * 6);
            break;
    }

    enigmeState.difficultyColor = (SDL_Color){255, 0, 0};
    enigmeState.textColor = (SDL_Color){0, 0, 0};
    enigmeState.hintColor = (SDL_Color){255, 165, 0};
    enigmeState.difficultyMessage = TTF_RenderText_Solid(enigmeState.fontdiff, enigmeState.difficultyText, enigmeState.difficultyColor);

    enigmeState.positionimage = (SDL_Rect){0, 0, 0, 0};
    enigmeState.positiontexte = (SDL_Rect){500, 150, 0, 0};
    enigmeState.positionhint = (SDL_Rect){200, 350, 0, 0};
    enigmeState.positionhorloge = (SDL_Rect){200, 400, 0, 0};
    enigmeState.positionroto = (SDL_Rect){(WINDOW_WIDTH - enigmeState.current->w) / 2, 50, 0, 0};
    enigmeState.difficultyPosition = (SDL_Rect){600, 57, 0, 0};
    enigmeState.p_rune[0] = (SDL_Rect){270, 200, 0, 0};
    enigmeState.p_rune[1] = (SDL_Rect){412, 205, 0, 0};
    enigmeState.p_rune[2] = (SDL_Rect){560, 210, 0, 0};
    enigmeState.p_rune[3] = (SDL_Rect){700, 210, 0, 0};
    enigmeState.p_rune[4] = (SDL_Rect){855, 210, 0, 0};
    enigmeState.p_rune[5] = (SDL_Rect){1001, 215, 0, 0};

    enigmeState.gameStartTime = SDL_GetTicks();
    enigmeState.last_change = SDL_GetTicks();

    // Display runes-meaning.png for 10 seconds
    if (display_screen(screen, "runes-meaning.png", 10000) != 0) {
        enigmeState.quitter = 0;
    }

    // Display enigmeback.png with difficulty image for 4 seconds
    if (enigmeState.quitter) {
        Uint32 difficultyStartTime = SDL_GetTicks();
        while (enigmeState.quitter && (SDL_GetTicks() - difficultyStartTime < 4000)) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                    enigmeState.quitter = 0;
                    currentState = MENU_STATE;
                }
            }
            SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
            SDL_BlitSurface(enigmeState.image, NULL, screen, &enigmeState.positionimage);
            SDL_BlitSurface(enigmeState.current, NULL, screen, &enigmeState.positionroto);
            SDL_Flip(screen);
            SDL_Delay(10);
        }
    }

    while (enigmeState.quitter) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsed = currentTime - enigmeState.gameStartTime;

        if (!enigmeState.fin_jeu && SDL_GetTicks() - enigmeState.last_change > 4000) {
            enigmeState.current_clock = (enigmeState.current_clock + 1) % 9;
            enigmeState.last_change = SDL_GetTicks();
            if (enigmeState.current_clock == 8) {
                enigmeState.time_up = 1;
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
        SDL_BlitSurface(enigmeState.image, NULL, screen, &enigmeState.positionimage);
        SDL_BlitSurface(enigmeState.horloge[enigmeState.current_clock], NULL, screen, &enigmeState.positionhorloge);

        if (enigmeState.difficultyMessage) {
            SDL_BlitSurface(enigmeState.difficultyMessage, NULL, screen, &enigmeState.difficultyPosition);
        }
        for (int i = 0; i < 6; i++) {
            SDL_BlitSurface(enigmeState.r[i], NULL, screen, &enigmeState.p_rune[i]);
        }
        if (enigmeState.hint && enigmeState.nb_fail >= 3) {
            SDL_BlitSurface(enigmeState.hint, NULL, screen, &enigmeState.positionhint);
        }

        if (enigmeState.time_up) {
            SDL_BlitSurface(enigmeState.temps, NULL, screen, &enigmeState.positionroto);
            if (enigmeState.message) {
                SDL_BlitSurface(enigmeState.message, NULL, screen, &enigmeState.positiontexte);
            }
            SDL_Flip(screen);
            SDL_Delay(2000);

            resetGame(&enigmeState);
            enigmeState.fin_jeu = 0;
            enigmeState.current_clock = 0;
            enigmeState.time_up = 0;
            enigmeState.last_change = SDL_GetTicks();

            if (enigmeState.difficultyMessage) SDL_FreeSurface(enigmeState.difficultyMessage);
            enigmeState.difficultyMessage = TTF_RenderText_Solid(enigmeState.font, enigmeState.difficultyText, enigmeState.difficultyColor);

            if (enigmeState.hint) {
                SDL_FreeSurface(enigmeState.hint);
                enigmeState.hint = NULL;
            }

            if (enigmeState.message) {
                SDL_FreeSurface(enigmeState.message);
                enigmeState.message = NULL;
            }
            continue;
        }

        SDL_GetMouseState(&enigmeState.mousex, &enigmeState.mousey);
        for (int i = 0; i < 6; i++) {
            enigmeState.r[i] = enigmeState.rune[i];
        }

        if (enigmeState.mousex >= 270 && enigmeState.mousex <= 320 && enigmeState.mousey >= 220 && enigmeState.mousey <= 315) {
            enigmeState.r[0] = enigmeState.rune_hover[0];
        } else if (enigmeState.mousex >= 410 && enigmeState.mousex <= 515 && enigmeState.mousey >= 220 && enigmeState.mousey <= 310) {
            enigmeState.r[1] = enigmeState.rune_hover[1];
        } else if (enigmeState.mousex >= 550 && enigmeState.mousex <= 615 && enigmeState.mousey >= 220 && enigmeState.mousey <= 320) {
            enigmeState.r[2] = enigmeState.rune_hover[2];
        } else if (enigmeState.mousex >= 700 && enigmeState.mousex <= 760 && enigmeState.mousey >= 215 && enigmeState.mousey <= 330) {
            enigmeState.r[3] = enigmeState.rune_hover[3];
        } else if (enigmeState.mousex >= 845 && enigmeState.mousex <= 920 && enigmeState.mousey >= 215 && enigmeState.mousey <= 320) {
            enigmeState.r[4] = enigmeState.rune_hover[4];
        } else if (enigmeState.mousex >= 980 && enigmeState.mousex <= 1050 && enigmeState.mousey >= 215 && enigmeState.mousey <= 315) {
            enigmeState.r[5] = enigmeState.rune_hover[5];
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    enigmeState.quitter = 0;
                    currentState = MENU_STATE;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        enigmeState.quitter = 0;
                        currentState = MENU_STATE;
                    } else if (enigmeState.nb < 6) {
                        if (event.key.keysym.sym == SDLK_a) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 1;
                        } else if (event.key.keysym.sym == SDLK_z) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 2;
                        } else if (event.key.keysym.sym == SDLK_e) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 3;
                        } else if (event.key.keysym.sym == SDLK_r) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 4;
                        } else if (event.key.keysym.sym == SDLK_t) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 5;
                        } else if (event.key.keysym.sym == SDLK_y) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 6;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (enigmeState.nb < 6) {
                        enigmeState.mousex = event.button.x;
                        enigmeState.mousey = event.button.y;
                        if (enigmeState.mousex >= 270 && enigmeState.mousex <= 320 && enigmeState.mousey >= 220 && enigmeState.mousey <= 315) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 1;
                        } else if (enigmeState.mousex >= 410 && enigmeState.mousex <= 515 && enigmeState.mousey >= 220 && enigmeState.mousey <= 310) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 2;
                        } else if (enigmeState.mousex >= 550 && enigmeState.mousex <= 615 && enigmeState.mousey >= 220 && enigmeState.mousey <= 320) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 3;
                        } else if (enigmeState.mousex >= 700 && enigmeState.mousex <= 760 && enigmeState.mousey >= 215 && enigmeState.mousey <= 330) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 4;
                        } else if (enigmeState.mousex >= 845 && enigmeState.mousex <= 920 && enigmeState.mousey >= 215 && enigmeState.mousey <= 320) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 5;
                        } else if (enigmeState.mousex >= 980 && enigmeState.mousex <= 1050 && enigmeState.mousey >= 215 && enigmeState.mousey <= 315) {
                            Mix_PlayChannel(-1, enigmeState.son, 0);
                            enigmeState.t[enigmeState.nb++] = 6;
                        }
                    }
                    break;
            }
        }

        if (enigmeState.nb == 6) {
            int test = 1;
            for (int i = 0; i < 6; i++) {
                if (enigmeState.t[i] != (*enigmeState.currentVictoire)[i]) {
                    test = 0;
                    break;
                }
            }

            if (test) {
                enigmeState.fin_jeu = 1;
                SDL_BlitSurface(enigmeState.saha, NULL, screen, &enigmeState.positionroto);
                enigmeState.nb_fail = 0;
                if (enigmeState.hint) {
                    SDL_FreeSurface(enigmeState.hint);
                    enigmeState.hint = NULL;
                }
                SDL_Flip(screen);
                SDL_Delay(2000);
                currentState = MENU_STATE;
                enigmeState.quitter = 0;
            } else {
                enigmeState.nb_fail++;
                if (enigmeState.nb_fail >= 3 && !enigmeState.hint) {
                    choicehint(&enigmeState);
                    enigmeState.hint = TTF_RenderText_Solid(enigmeState.fontm, enigmeState.hintText, enigmeState.hintColor);
                }
                SDL_BlitSurface(enigmeState.faux, NULL, screen, &enigmeState.positionroto);
                if (enigmeState.message) {
                    SDL_BlitSurface(enigmeState.message, NULL, screen, &enigmeState.positiontexte);
                }
                if (enigmeState.hint) {
                    SDL_BlitSurface(enigmeState.hint, NULL, screen, &enigmeState.positionhint);
                }
                SDL_Flip(screen);
                SDL_Delay(2000);

                resetGame(&enigmeState);
                enigmeState.fin_jeu = 0;
                if (enigmeState.message) {
                    SDL_FreeSurface(enigmeState.message);
                    enigmeState.message = NULL;
                }
                continue;
            }
        }

        if (enigmeState.message) {
            SDL_BlitSurface(enigmeState.message, NULL, screen, &enigmeState.positiontexte);
        }

        SDL_Flip(screen);
        if (enigmeState.message) {
            SDL_FreeSurface(enigmeState.message);
            enigmeState.message = NULL;
        }
    }

    SDL_FreeSurface(enigmeState.image);
    SDL_FreeSurface(enigmeState.saha);
    SDL_FreeSurface(enigmeState.faux);
    SDL_FreeSurface(enigmeState.temps);
    for (int i = 0; i < 9; i++) SDL_FreeSurface(enigmeState.horloge[i]);
    for (int i = 0; i < 6; i++) {
        SDL_FreeSurface(enigmeState.rune[i]);
        SDL_FreeSurface(enigmeState.rune_hover[i]);
    }
    SDL_FreeSurface(enigmeState.facile);
    SDL_FreeSurface(enigmeState.moyen);
    SDL_FreeSurface(enigmeState.difficile);
    if (enigmeState.difficultyMessage) SDL_FreeSurface(enigmeState.difficultyMessage);
    if (enigmeState.hint) SDL_FreeSurface(enigmeState.hint);
    Mix_FreeChunk(enigmeState.son);
}

int main() {
    SDL_Surface *screen = NULL;
    TTF_Font *font = NULL, *fontm = NULL, *fontdiff = NULL;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        printf("Failed to get current working directory\n");
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return 1;
    }

    Mix_AllocateChannels(16);

    font = TTF_OpenFont("arial.ttf", 24);
    fontm = TTF_OpenFont("arial.ttf", 40);
    fontdiff = TTF_OpenFont("DalekPinpointBold.ttf", 50);
    if (!font || !fontm || !fontdiff) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        cleanupGame();
        return 1;
    }

    beepSound = Mix_LoadWAV("beep.wav");
    hoverSound = Mix_LoadWAV("click.wav");
    enigmeMusic = Mix_LoadMUS("enigme.wav");
    winMusic = Mix_LoadMUS("win.wav");
    if (!beepSound || !hoverSound || !enigmeMusic || !winMusic) {
        printf("Warning: Could not load sounds\n");
    }

    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_SWSURFACE);
    if (!screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        cleanupGame();
        return 1;
    }

    SDL_Surface *menuBg = loadImage("2D.png");
    SDL_Surface *quizButton = loadImage("quiz.png");
    SDL_Surface *quizButtonHover = loadImage("quiz2.png");
    SDL_Surface *puzzleButton = loadImage("puzzle.png");
    SDL_Surface *puzzleButtonHover = loadImage("puzzle2.png");
    if (!menuBg || !quizButton || !quizButtonHover || !puzzleButton || !puzzleButtonHover) {
        printf("Failed to load menu assets\n");
        cleanupGame();
        return 1;
    }

    SDL_Rect quizButtonRect = {
        (WINDOW_WIDTH - quizButton->w) / 2,
        (WINDOW_HEIGHT/2 - quizButton->h - BUTTON_SPACING/2),
        quizButton->w,
        quizButton->h
    };
    SDL_Rect puzzleButtonRect = {
        (WINDOW_WIDTH - puzzleButton->w) / 2,
        (WINDOW_HEIGHT/2 + BUTTON_SPACING/2),
        puzzleButton->w,
        puzzleButton->h
    };

    QuizQuestion questions[MAX_QUESTIONS];
    int questionCount = loadQuestions("fichier.txt", questions);
    if (questionCount == 0) {
        printf("Failed to load questions\n");
        cleanupGame();
        return 1;
    }

    if (enigmeMusic) Mix_PlayMusic(enigmeMusic, -1);

    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && currentState == MENU_STATE) {
                Point mouse = {event.button.x, event.button.y};
                if (pointInRect(mouse, quizButtonRect)) {
                    Mix_PlayChannel(-1, hoverSound, 0);
                    if (enigmeMusic) Mix_HaltMusic();
                    currentState = GAME_STATE;
                    player = (PlayerStats){0, 3, 1, 0, 0};
                } else if (pointInRect(mouse, puzzleButtonRect)) {
                    Mix_PlayChannel(-1, hoverSound, 0);
                    currentState = PUZZLE_STATE;
                }
            }
        }

        if (currentState == MENU_STATE) {
            Point mouse;
            SDL_GetMouseState(&mouse.x, &mouse.y);
            int isHoveringQuiz = pointInRect(mouse, quizButtonRect);
            int isHoveringPuzzle = pointInRect(mouse, puzzleButtonRect);

            SDL_BlitSurface(menuBg, NULL, screen, NULL);
            SDL_BlitSurface(isHoveringQuiz ? quizButtonHover : quizButton, NULL, screen, &quizButtonRect);
            SDL_BlitSurface(isHoveringPuzzle ? puzzleButtonHover : puzzleButton, NULL, screen, &puzzleButtonRect);
            SDL_Flip(screen);
        } else if (currentState == GAME_STATE) {
            runQuizGame(screen, font, questions, questionCount);
            if (currentState == GAMEOVER_STATE) {
                showGameOver(screen, font);
                currentState = MENU_STATE;
                if (enigmeMusic) Mix_PlayMusic(enigmeMusic, -1);
            } else if (currentState == WIN_STATE) {
                showWinScreen(screen, font);
                currentState = MENU_STATE;
                if (enigmeMusic) Mix_PlayMusic(enigmeMusic, -1);
            }
        } else if (currentState == PUZZLE_STATE) {
            runEnigmeGame(screen, font, fontm, fontdiff);
            if (enigmeMusic) Mix_PlayMusic(enigmeMusic, -1);
        }

        SDL_Delay(10);
    }

    SDL_FreeSurface(menuBg);
    SDL_FreeSurface(quizButton);
    SDL_FreeSurface(quizButtonHover);
    SDL_FreeSurface(puzzleButton);
    SDL_FreeSurface(puzzleButtonHover);
    TTF_CloseFont(font);
    TTF_CloseFont(fontm);
    TTF_CloseFont(fontdiff);
    cleanupGame();

    return 0;
}
