#include "enigme.h"

int main(int argc, char** argv) {
    SDL_Event event;
    SDL_Rect positionimage, positiontexte, positionhint, positionhorloge, positionroto;
    SDL_Rect p_rune1, p_rune2, p_rune3, p_rune4, p_rune5, p_rune6;
    SDL_Surface *ecran = NULL, *image = NULL, *message = NULL, *hint = NULL;
    SDL_Surface *horloge0 = NULL, *horloge1 = NULL, *horloge2 = NULL, *horloge3 = NULL, *horloge4 = NULL, *horloge5 = NULL, *horloge6 = NULL, *horloge7 = NULL, *horloge8 = NULL;
    SDL_Surface *rune1 = NULL, *rune2 = NULL, *rune3 = NULL, *rune4 = NULL, *rune5 = NULL, *rune6 = NULL;
    TTF_Font *font = NULL, *fontm = NULL, *fontdiff = NULL;
    SDL_Surface *saha = NULL, *faux = NULL, *temps = NULL;
    int (*currentVictoire)[6] = NULL;
    char difficultyText[20] = {0};
    SDL_Surface *difficultyMessage = NULL;
    SDL_Rect difficultyPosition = {600, 57, 0, 0};
    SDL_Color difficultyColor = {255, 0, 0};
    SDL_Color textColor = {0, 0, 0};
    SDL_Color hintColor = {255, 165, 0};
    Mix_Chunk *son;
    int mousex, mousey;
    int quitter = 1, nb = 0;
    int victoire[] = {2, 3, 4, 5, 1, 6}; //facile
    int victoire2[] = {1, 1, 2, 2, 3, 3};//moyenne
    int victoire3[] = {5, 4, 6, 3, 2, 2};//difficile 
    int t[6] = {0}; 
    char hintText[100];
    int click1 = 0, click2 = 0, click3 = 0, click4 = 0, click5 = 0, click6 = 0;
    int i, test, nb_fail = 0, fin_jeu = 0;
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

    ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (ecran == NULL) {
        printf("Échec de création de la fenêtre : %s\n", SDL_GetError());
        return 1;
    }

    image = IMG_Load("enigmeback.png");
    if (image == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    saha = IMG_Load("saha.png");
    if (saha == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    faux = IMG_Load("Faux.png");
    if (faux== NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    temps = IMG_Load("temps.png");
    if (temps == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    horloge0 = IMG_Load("horloge1.0.png");
    if (horloge0 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    horloge1 = IMG_Load("horloge1.1.png");
    if (horloge1 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    horloge2 = IMG_Load("horloge1.2.png");
    if (horloge2 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    horloge3 = IMG_Load("horloge1.3.png");
    if (horloge3 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    horloge4 = IMG_Load("horloge1.4.png");
    if (horloge4 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    horloge5 = IMG_Load("horloge1.5.png");
    if (horloge5 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    horloge6 = IMG_Load("horloge1.6.png");
    if (horloge6 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    horloge7 = IMG_Load("horloge1.7.png");
    if (horloge7 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    horloge8 = IMG_Load("horloge1.8.png");
    if (horloge8 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    rune1 = IMG_Load("f.png");
    if (rune1 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    rune2 = IMG_Load("b.png");
    if (rune2 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    rune3 = IMG_Load("v.png");
    if (rune3 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    rune4 = IMG_Load("g.png");
    if (rune4 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    rune5 = IMG_Load("t.png");
    if (rune5 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    
    rune6 = IMG_Load("r.png");
    if (rune6 == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }
    fontdiff = TTF_OpenFont("DalekPinpointBold.ttf", 50);
    if (fontdiff == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }
	
    font = TTF_OpenFont("arial.ttf", 50);
    if (font == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }
    fontm = TTF_OpenFont("arial.ttf", 40);
    if (font == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }
    son= Mix_LoadWAV( "click-8.wav" );
    
    int difficulty = rand() % 3;
    switch(difficulty) {
        case 0:
            currentVictoire = &victoire;
            strcpy(difficultyText, "Facile");
            break;
        case 1:
            currentVictoire = &victoire2;
            strcpy(difficultyText, "Moyen");
            break;
        case 2:
            currentVictoire = &victoire3;
            strcpy(difficultyText, "Difficile");
            break;
    }
    difficultyMessage = TTF_RenderText_Solid(fontdiff, difficultyText, difficultyColor);
    
    positionimage.x = 0;
    positionimage.y = 0;
    positiontexte.x = 500;
    positiontexte.y = 150;
    positionhint.x = 200;
    positionhint.y = 350;
    positionhorloge.x = 200;
    positionhorloge.y = 400;
    positionroto.x = 10;
    positionroto.y = 0;
    p_rune1.x = 270;
    p_rune1.y = 200;
    p_rune2.x = 412;
    p_rune2.y = 205;
    p_rune3.x = 560;
    p_rune3.y = 210;
    p_rune4.x = 700;
    p_rune4.y = 210;
    p_rune5.x = 855;
    p_rune5.y = 210;
    p_rune6.x = 1001;
    p_rune6.y = 215;
    
    SDL_Surface *tabh[9] = {horloge0, horloge1, horloge2, horloge3, horloge4, horloge5, horloge6, horloge7, horloge8};
    int current_clock = 0;
    Uint32 last_change = SDL_GetTicks();
    int time_up = 0;

    while (quitter) {
      
        if (!fin_jeu && SDL_GetTicks() - last_change > 2000) {  
            current_clock = (current_clock + 1) % 9;
            last_change = SDL_GetTicks();
            
        
            if (current_clock == 8) {
                time_up = 1;
            }
        }

        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 255, 255, 255));
        SDL_BlitSurface(image, NULL, ecran, &positionimage);
        SDL_BlitSurface(tabh[current_clock], NULL, ecran, &positionhorloge);
        
        if (difficultyMessage) {
            SDL_BlitSurface(difficultyMessage, NULL, ecran, &difficultyPosition);
        }
        SDL_BlitSurface(rune1, NULL, ecran, &p_rune1);
        SDL_BlitSurface(rune2, NULL, ecran, &p_rune2);
        SDL_BlitSurface(rune3, NULL, ecran, &p_rune3);
        SDL_BlitSurface(rune4, NULL, ecran, &p_rune4);
        SDL_BlitSurface(rune5, NULL, ecran, &p_rune5);
        SDL_BlitSurface(rune6, NULL, ecran, &p_rune6);

        if (hint && nb_fail >= 3) {
            SDL_BlitSurface(hint, NULL, ecran, &positionhint);
        }
        
    
        if (time_up) {
            SDL_BlitSurface(temps, NULL, ecran, &positionroto);
            SDL_BlitSurface(message, NULL, ecran, &positiontexte);
            SDL_Flip(ecran);
            SDL_Delay(2000);
            
        
            resetGame(&nb, t);
            fin_jeu = 0;
            current_clock = 0;
            time_up = 0;
            last_change = SDL_GetTicks();
            
           
            difficulty = rand() % 3;
            switch(difficulty) {
                case 0: currentVictoire = &victoire; strcpy(difficultyText, "  Facile"); break;
                case 1: currentVictoire = &victoire2; strcpy(difficultyText, "Moyen"); break;
                case 2: currentVictoire = &victoire3; strcpy(difficultyText, "Difficile"); break;
            }
            if (difficultyMessage) SDL_FreeSurface(difficultyMessage);
            difficultyMessage = TTF_RenderText_Solid(font, difficultyText, difficultyColor);
            
            if (hint) {
                SDL_FreeSurface(hint);
                hint = NULL;
            }
            
            SDL_FreeSurface(message);
            message = NULL;
            continue;
        }
        
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quitter = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) quitter = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                  if (nb < 6) {
                    mousex = event.button.x;
                    mousey = event.button.y;

                    if (mousex >= 270 && mousex <= 320 && mousey >= 220 && mousey <= 315) {
                        Mix_PlayChannel(-1, son, 0);
                        t[nb] = 1; 
                        nb++;
                    } else if (mousex >= 410 && mousex <= 515 && mousey >= 220 && mousey <= 310) {
                        Mix_PlayChannel(-1, son, 0);
                        t[nb] = 2; 
                        nb++;
                    } else if (mousex >= 550 && mousex <= 615 && mousey >= 220 && mousey <= 320) {
                        Mix_PlayChannel(-1, son, 0);
                        t[nb] = 3; 
                        nb++;
                    } else if (mousex >= 700 && mousex <= 760 && mousey >= 215 && mousey <= 330) {
                        Mix_PlayChannel(-1, son, 0);
                        t[nb] = 4; 
                        nb++;
                    } else if (mousex >= 845 && mousex <= 920 && mousey >= 215 && mousey <= 320) {
                        Mix_PlayChannel(-1, son, 0);
                        t[nb] = 5; 
                        nb++;
                    } else if (mousex >= 980 && mousex <= 1050 && mousey >= 215 && mousey <= 315) {
                        Mix_PlayChannel(-1, son, 0);
                        t[nb] = 6; 
                        nb++;
                    }
                    break;
                }
            }
        }

        if (nb == 6) {
            test = 1;
            for (i = 0; i < 6; i++) {
                if (t[i] != (*currentVictoire)[i]) {
                    test = 0;
                    break;
                }
            }

            if (test) {
            	fin_jeu = 1;
                SDL_BlitSurface(saha, NULL, ecran, &positionroto);
                nb_fail = 0;
                if (hint) {
                    SDL_FreeSurface(hint);
                    hint = NULL;
                }
            } else {
                nb_fail++;
                if (nb_fail >= 3 && !hint) {
                    choicehint(hintText, *currentVictoire, difficulty);
                    hint = TTF_RenderText_Solid(fontm, hintText, hintColor);
                }
                SDL_BlitSurface(faux, NULL, ecran, &positionroto);

                SDL_BlitSurface(message, NULL, ecran, &positiontexte);
                if (hint) {
                    SDL_BlitSurface(hint, NULL, ecran, &positionhint);
                }
                SDL_Flip(ecran);
                SDL_Delay(2000);
                
                resetGame(&nb, t);
                fin_jeu = 0;
                SDL_FreeSurface(message);
                message = NULL;
                continue;
            }
        }
        
        if (message) {
            SDL_BlitSurface(message, NULL, ecran, &positiontexte);
        }
        
        SDL_Flip(ecran);
        SDL_FreeSurface(message);
        message = NULL;
        
    }

    SDL_FreeSurface(ecran);
    SDL_FreeSurface(image);
    SDL_FreeSurface(horloge0);
    SDL_FreeSurface(horloge1);
    SDL_FreeSurface(horloge2);
    SDL_FreeSurface(horloge3);
    SDL_FreeSurface(horloge4);
    SDL_FreeSurface(horloge5);
    SDL_FreeSurface(horloge6);
    SDL_FreeSurface(horloge7);
    SDL_FreeSurface(horloge8);
    SDL_FreeSurface(difficultyMessage);
    SDL_FreeSurface(rune1);
    SDL_FreeSurface(rune2);
    SDL_FreeSurface(rune3);
    SDL_FreeSurface(rune4);
    SDL_FreeSurface(rune5);
    SDL_FreeSurface(rune6);
    SDL_FreeSurface(saha);
    SDL_FreeSurface(faux);
    TTF_CloseFont(font);
    TTF_CloseFont(fontm);
    TTF_CloseFont(fontdiff);
    Mix_FreeChunk( son);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
