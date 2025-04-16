#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include "struct.h"
#include <time.h>
#include <stdbool.h>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800


int main(int argc, char* argv[]) {
    SDL_Surface* screen = NULL;

    camera c;

    int boucle=1;
    int detect1=0;
    int detect2=0;
    int detect3=0;
    int currentscreen=1;
    SDL_Event event;


    texte txt1,txt2,txt3;
    c.walk_right_permission = 1;
    c.walk_left_permission =1;
    c.is_running_right = 0;
    c.is_running_left = 0;
    c.acceleration=1.0f;
    c.start_time=0;

    Uint32 start_time = SDL_GetTicks();
    Uint32 current_time = SDL_GetTicks();
    

    c.shift_pressed = 0;
 
    
   int current_frame = 0;
	
    // initialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    // set up the screen
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
        Uint32 startingtime = SDL_GetTicks();
     Uint32 currentiming = SDL_GetTicks();
    SDL_WM_SetCaption("Background", NULL);

	int tempspb=0,tempsab=0;
	int vb=1;

c.menu1=NULL ;
c.menu1 = IMG_Load("back.png");
c.menu2=NULL ;
c.menu2 = IMG_Load("back2.png");

initcamera (&c);
initialiser_texte1(&txt1);
initialiser_texte2(&txt2);
initialiser_texte3(&txt3);


    while (boucle) {
        // handle events
               Uint32 current_time = SDL_GetTicks();
        Uint32 elapsed_time = current_time - start_time;

	Uint32 currentiming = SDL_GetTicks();  // Update current time
	Uint32 elapsed_timing = currentiming - startingtime; 

    
        if(currentscreen==1)
	{
        SDL_BlitSurface(c.menu1,&c.camera,screen,NULL);
	}

	if (currentscreen==2)
	{
        SDL_BlitSurface(c.menu2,&c.camera,screen,NULL);
	}
        affichertemps(elapsed_timing / 1000);

//guide level 1

 if ( (c.camera.x >= 700) && (c.camera.x <= 1200) && (detect2==0) && (currentscreen==1) ) {
 afficher_texte1(screen,txt1);
    }

 if ( (c.camera.x >= 2700) && (c.camera.x <= 3800) && (detect3==0) && (currentscreen==1) ) {
 afficher_texte2(screen,txt2);
    }

 if ( (c.camera.x >= 4600) && (c.camera.x <= 5100) && (detect2==0) && (currentscreen==1) ) {
 afficher_texte1(screen,txt1);
    }

 if ( (c.camera.x >= 5300) && (c.camera.x <= 5800) && (detect3==0) && (currentscreen==1) ) {
 afficher_texte2(screen,txt2);
    }

 if ( (c.camera.x >= 6100) && (c.camera.x <= 6600) && (detect2==0) && (currentscreen==1) ) {
 afficher_texte1(screen,txt1);
    }

//guide level 2

 if ( (c.camera.x >= 1300) && (c.camera.x <= 2000) && (detect2==0) && (currentscreen==2) ) {
 afficher_texte1(screen,txt1);
    }

 if ( (c.camera.x >= 2700) && (c.camera.x <= 4500) && (detect3==0) && (currentscreen==2) ) {
 afficher_texte2(screen,txt2);
    }

 if ( (c.camera.x >= 5000) && (c.camera.x <= 5600) && (detect2==0) && (currentscreen==2) ) {
 afficher_texte1(screen,txt1);
    }

 if ( (c.camera.x >= 6200) && (c.camera.x <= 7200) && (detect3==0) && (currentscreen==2) ) {
 afficher_texte2(screen,txt2);
    }


 if ( (c.camera.x >= 7500) && (detect1==0) ) {
 afficher_texte3(screen,txt3);
    }

 if ( (c.camera.x >= 7500) && (detect1==1) ) {
currentscreen=2;
c.camera.x=0; 
detect1=3;
    }
	
	SDL_Flip(screen);

        if (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_FreeSurface(c.menu1);
                    SDL_FreeSurface(c.menu2);
                    SDL_Quit();
                    boucle= 0;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        boucle=0;
                        break;

 			case SDLK_x:
			if ( (detect1!=3) && (c.camera.x >= 7500) && (currentscreen==1) ){
                        detect1=1;
			}
                        break;
		
                        case SDLK_RIGHT:
                       if (!c.is_running_left) {
                            
                            c.is_running_right = 1;
                            }
                            
                            
                            
                            break;
                        case SDLK_LEFT:
                	    if (!c.is_running_right) {
                	    
                	    c.is_running_left = 1;
                	    }
                	    
                            break;
                        case SDLK_LSHIFT:
                        	c.shift_pressed = 1;
                        	c.start_time = SDL_GetTicks();
                        	c.acceleration=1.0f;
                            break;
                        case SDLK_DOWN :
                                scrollingdown (&c);
                            break;
                        case SDLK_UP :
                                scrollingup (&c);
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_RIGHT:
                      
                           c.is_running_right = 0;
                            break;
                        case SDLK_LEFT:
                      
                           c.is_running_left = 0;
                            break;
                        case SDLK_LSHIFT:
			    c.shift_pressed = 0;
                            c.acceleration=1.0f;
                            break;   
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        //camera moving
	if(c.walk_right_permission == 1)
	{
 	if (c.is_running_right) {
            scrolling (&c);
        }
        }
        if(c.walk_left_permission == 1)
        {
	if (c.is_running_left) {
	scrollingb (&c);
	}
	}
	//camera acceleration
	if(c.acceleration>=1.0f)
	{
	  c.elapsed_time = SDL_GetTicks() - c.start_time;
	  if (c.elapsed_time < 3000) {
	  float new_K = (c.elapsed_time / 750) * 0.5f + 1.0f;
	  if (new_K <= 3.0f && new_K > c.acceleration) {
	    c.acceleration = new_K;
	    }
	  } else {
	     c.acceleration = 3.0f;
	    }
     
	  }

	

}
    SDL_FreeSurface(screen);
    TTF_Quit();
    SDL_Quit();
return 0;







}









