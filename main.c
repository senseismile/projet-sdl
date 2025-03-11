#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH 1344
#define SCREEN_HEIGHT 756

int main(int argc, char** argv) {
    SDL_Event event;
    SDL_Rect positionimage;
    SDL_Rect positiontexte;
    SDL_Surface *ecran = NULL;
    SDL_Surface *image = NULL;
    SDL_Surface *lose = NULL;
    TTF_Font *font = NULL;
    SDL_Surface *saha = NULL;
    SDL_Color textColor = {0, 0, 0};
    int mousex, mousey;
    int quitter = 1, nb = 0;
    int victoire[] = {2, 3, 4, 5, 1, 6};
    int t[5];
    int click1 = 0, click2 = 0, click3 = 0, click4 = 0, click5 = 0, click6 = 0;
    int i, test = 0;
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

    // Create window
    ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (ecran == NULL) {
        printf("Échec de création de la fenêtre : %s\n", SDL_GetError());
        return 1;
    }

    image = IMG_Load("asba.png");
    if (image == NULL) {
        printf("Erreur de chargement de l'image : %s\n", IMG_GetError());
        return 1;
    }

    font = TTF_OpenFont("arial.ttf", 50);
    if (font == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return 1;
    }

    positionimage.x = 0;
    positionimage.y = 0;
    positiontexte.x = 672;
    positiontexte.y = 478;

    while (quitter) {
        
        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 255, 255, 255));

        
        SDL_BlitSurface(image, NULL, ecran, &positionimage);

        
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
                case SDL_MOUSEBUTTONDOWN:
                    mousex = event.button.x;
                    mousey = event.button.y;
                     
		            if (mousex >= 320 && mousex <= 390 && mousey >= 330 && mousey <= 430) {
		                click1 = 1;
		                nb++;
		            }	
		            else {
		            	click1 = 0;
		            	if (mousex >= 440 && mousex <= 515 && mousey >= 330 && mousey <= 430) {
		                   click2 = 1;
		                   nb++;		               
		            	}
		            
				else {
				   click2 = 0;
				   if (mousex >= 565 && mousex <= 640 && mousey >= 330 && mousey <= 430) {
				      click3 = 1;
				      nb++;
				   }
		            	
		            	   else {
		            	       click3 = 0;
		            		if (mousex >= 690 && mousex <= 760 && mousey >= 330 && mousey <= 430) {
		               	    click4 = 1;
		               	    nb++;		                
		            		} 
		            		else {		            	
		            		     click4 = 0;
		            		     if (mousex >= 810 && mousex <= 880 && mousey >= 330 && mousey <= 430) {
		                	        click5 = 1;
		                	        nb++;		                
		            		     }
		            
		            		     else {
		            			click5 = 0;
		            			if (mousex >= 930 && mousex <= 1000 && mousey >= 330 && mousey <= 430) {
		                   		   click6 = 1;
		                   		   nb++;		                
		            			}
		            		     }
		            		}
		            	   }
		            	 }
		              }
		            break;
            	}
            
           }
	for (i = 0; i < 5; i++) {
       	if (click1 = 1) {
       		t[i] == 1;
       	}
       	if (click2 = 1) {
       		t[i] == 2;
       	}
       	if (click3 = 1) {
       		t[i] == 3;
       	}
       	if (click4 = 1) {
       		t[i] == 4;
       	}
       	if (click5 = 1) {
       		t[i] == 5;
       	}
       	if (click6 = 1) {
       		t[i] == 6;
       	}
       }
	for (i = 0; i < 5; i++) {
		if (t[i] == victoire[i]) {
			test = 1;
		}
		else test = 0;
       }
       
       	
       if (test = 1 && nb == 6) {
       	saha = TTF_RenderText_Solid(font, "saha", textColor);
            	if (saha) {
                	SDL_BlitSurface(saha, NULL, ecran, &positiontexte);
		}
	}
	
        SDL_Flip(ecran);
    }

    
    SDL_FreeSurface(ecran);
    SDL_FreeSurface(image);
    SDL_FreeSurface(lose);
    SDL_FreeSurface(saha);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
