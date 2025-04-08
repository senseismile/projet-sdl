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



//tache background
//initialisation camera
void initcamera ( camera* c)
{
    c->camera.x=0;
    c->camera.y=0;
    c->camera.h=1080;
    c->camera.w=1920;
    
}
void initcameramultiplayer ( camera* c)
{
    c->camera.x=0;
    c->camera.y=0;
    c->camera.h=1080;
    c->camera.w=960;
    
}
//scrolling to right
void scrolling (camera* c)
{
	//character not accelerating
	if(!c->shift_pressed)
        {
        c->camera.x+=5;
        }
        
        //character accelerating
        if(c->shift_pressed)
        {
        c->camera.x=c->camera.x+5*(int)c->acceleration;
        }
        
         if (c->camera.x < 0) {
        c->camera.x = 0;
    }
    if (c->camera.x >=7500)
        {
            c->camera.x=7500;
        }
    }


//scrolling to left
void scrollingb (camera* c)
{
	//character not accelerating
    	if(!c->shift_pressed)
        {
        c->camera.x-=5;
        }
        //character accelerating
        if(c->shift_pressed)
        {
        c->camera.x=c->camera.x-5*(int)c->acceleration;
        }
        
    if (c->camera.x <= 10)
    {
        c->camera.x = 0;
    }
    
}
//scrolling up and down , didnt use in integration
void scrollingdown (camera* c)
{
        c->camera.y+=40;
        if (c->camera.y >=600)
        {
            c->camera.y=600;
        }
}
void scrollingup (camera* c)
{
        c->camera.y-=40;
        if (c->camera.y <=10)
        {
            c->camera.y=10;
        }
}













































