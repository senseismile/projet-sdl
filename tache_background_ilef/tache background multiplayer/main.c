#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include "struct.h"
#include <stdbool.h>



#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define CHARACTER_WIDTH 224
#define CHARACTER_HEIGHT 245
#define GRAVITY 0.5
#define JUMP_VELOCITY -16
#define RUN_SPEED 5.0

#define FRAMES_PER_SECOND 15
#define NUM_FRAMES 3
#define FRAME_DELAY (1000 / FRAMES_PER_SECOND)

int main(int argc, char* argv[]) {
    SDL_Surface* screen = NULL;

    camera c,c2;

    int boucle=1;

    SDL_Event event;



	//camera 1 variables
    c.walk_right_permission = 1;
    c.walk_left_permission =1;
    c.is_running_right = 0;
    c.is_running_left = 0;
    c.acceleration=1.0f;
    c.start_time=0;
    c.elapsed_time=0;
    Uint32 start_time = SDL_GetTicks();
    Uint32 current_time = SDL_GetTicks();
    c.shift_pressed = 0;
    
    //camera2 variables
    c2.walk_right_permission = 1;
    c2.walk_left_permission =1;
    c2.is_running_right = 0;
    c2.is_running_left = 0;
    c2.acceleration=1.0f;
    c2.start_time=0;
    c2.elapsed_time=0;
    //Uint32 start_time = SDL_GetTicks();
    //Uint32 current_time = SDL_GetTicks();
    c2.shift_pressed = 0;
 
    
   
	
    // initialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    // set up the screen
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    SDL_WM_SetCaption("Character Movement", NULL);




c.menu1=NULL ;
c.menu1 = IMG_Load("back.png");
c2.menu2=NULL ;
c2.menu2 = IMG_Load("back.png");
//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

initcamera (&c);
initcameramultiplayer (&c2);

SDL_Rect control;
control.x=960;
control.y=0;
control.h=1080;
control.w=1920;

   
    while (boucle) {
        // handle events
        SDL_BlitSurface(c.menu1,&c.camera,screen,&control);
        SDL_BlitSurface(c2.menu2,&c2.camera,screen,NULL);
        Uint32 current_time = SDL_GetTicks();
        //Uint32 current_time2 = SDL_GetTicks();
	





	

	SDL_Flip(screen);
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_FreeSurface(c.menu1);
                     SDL_FreeSurface(c.menu2);
                    //liberer_background_music(&back_music);
                    SDL_Quit();
                    boucle= 0;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        boucle=0;
                        break;
                        case SDLK_RIGHT:
                       if (!c.is_running_left) {
                            
                            c.is_running_right = 1;
                            }
                            break;
                            case SDLK_d:
                	    if (!c2.is_running_right) {
                	    
                	    c2.is_running_right = 1;
                	    }
                	    
                            break;
                        case SDLK_LEFT:
                	    if (!c.is_running_right) {
                	    
                	    c.is_running_left = 1;
                	    }
                	    
                            break;
                            case SDLK_q:
                	    if (!c2.is_running_right) {
                	    
                	    c2.is_running_left = 1;
                	    }
                	    
                            break;
                        case SDLK_RSHIFT:
                        	c.shift_pressed = 1;
                        	c.start_time = SDL_GetTicks();
                        	c.acceleration=1.0f;
                            break;
                            case SDLK_LSHIFT:
                        	c2.shift_pressed = 1;
                        	c2.start_time = SDL_GetTicks();
                        	c2.acceleration=1.0f;
                            break;
                        case SDLK_DOWN :
                                scrollingdown (&c);
                            break;
                        case SDLK_s :
                                scrollingdown (&c2);
                            break;
                        case SDLK_UP :
                                scrollingup (&c);
                            break;
                        case SDLK_z :
                                scrollingup (&c2);
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
                        case SDLK_d:                      
                           c2.is_running_right = 0;
                            break;
                        case SDLK_LEFT:                      
                           c.is_running_left = 0;
                            break;
                         case SDLK_q:                      
                           c2.is_running_left = 0;
                            break;
                        case SDLK_RSHIFT:
			    c.shift_pressed = 0;
                            c.acceleration=1.0f;
                            break;
                        case SDLK_LSHIFT:
			    c2.shift_pressed = 0;
                            c2.acceleration=1.0f;
                            break;      
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        //camera 1 moving
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
	//camera 2 moving
	
	if(c2.walk_right_permission == 1)
	{
 	if (c2.is_running_right) {
            scrolling (&c2);
        }
        }
        if(c2.walk_left_permission == 1)
        {
	if (c2.is_running_left) {
	scrollingb (&c2);
	}
	}
	
	//camera 1 acceleration
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
	  //camera 2 acceleration
	if(c2.acceleration>=1.0f)
	{
	  c2.elapsed_time = SDL_GetTicks() - c2.start_time;
	  if (c2.elapsed_time < 3000) {
	  float new_K = (c2.elapsed_time / 750) * 0.5f + 1.0f;
	  if (new_K <= 3.0f && new_K > c2.acceleration) {
	    c2.acceleration = new_K;
	    }
	  } else {
	     c2.acceleration = 3.0f;
	    }
	  }
	

	

}

return 0;







}









