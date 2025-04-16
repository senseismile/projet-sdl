#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include "struct.h"
#include <stdbool.h>
#include <time.h>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800


void initcamera ( camera* c)
{
    c->camera.x=0;
    c->camera.y=0;
    c->camera.h=1080;
    c->camera.w=1920;
    
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



void affichertemps(int temps)
{
    SDL_Color color_temp = {255, 255, 255};
    TTF_Font* police_time = NULL;
    police_time = TTF_OpenFont("04B_08__.TTF",24);
    char temp[100];
    int heures = temps / 3600;
    int minutes = (temps % 3600) / 60;
    int secondes = temps % 60;
    sprintf(temp, "%02d:%02d:%02d", heures, minutes, secondes);
    SDL_Surface* temps_surface = TTF_RenderText_Solid(police_time, temp, color_temp);
    SDL_Rect pos_temp;
    pos_temp.x = 10;
    pos_temp.y = 10;
    SDL_BlitSurface(temps_surface, NULL, SDL_GetVideoSurface(), &pos_temp);
    TTF_CloseFont(police_time);
    SDL_FreeSurface(temps_surface);
}  


//text guide

void initialiser_texte1(texte *text1)
{
	TTF_Init();
	text1->police=TTF_OpenFont("04B_08__.TTF",35);
	text1->color_txt.r=0;
	text1->color_txt.g=0;
	text1->color_txt.b=0;
	text1->pos_txt.x=100;
	text1->pos_txt.y=80;
	text1->pos_txt.w=150;
	text1->pos_txt.h=150;
}

void afficher_texte1(SDL_Surface *screen,texte text1)
{
	text1.txt=TTF_RenderText_Blended(text1.police,"You must jump over to avoid falling in the hole !",text1.color_txt);
	SDL_BlitSurface(text1.txt,NULL,screen,&text1.pos_txt);
}

void initialiser_texte2(texte *text2)
{
	TTF_Init();
	text2->police=TTF_OpenFont("04B_08__.TTF",35);
	text2->color_txt.r=0;
	text2->color_txt.g=0;
	text2->color_txt.b=0;
	text2->pos_txt.x=150;
	text2->pos_txt.y=80;
	text2->pos_txt.w=150;
	text2->pos_txt.h=150;
}

void afficher_texte2(SDL_Surface *screen,texte text2)
{
	text2.txt=TTF_RenderText_Blended(text2.police,"You have to fight this enemy to avoid death !",text2.color_txt);
	SDL_BlitSurface(text2.txt,NULL,screen,&text2.pos_txt);
}


void initialiser_texte3(texte *text3)
{
	TTF_Init();
	text3->police=TTF_OpenFont("04B_08__.TTF",23);
	text3->color_txt.r=255;
	text3->color_txt.g=255;
	text3->color_txt.b=255;
	text3->pos_txt.x=40;
	text3->pos_txt.y=80;
	text3->pos_txt.w=150;
	text3->pos_txt.h=150;
}

void afficher_texte3(SDL_Surface *screen,texte text3)
{
	text3.txt=TTF_RenderText_Blended(text3.police,"Congratulations on reaching the gate, Press X to proceed to the next level !",text3.color_txt);
	SDL_BlitSurface(text3.txt,NULL,screen,&text3.pos_txt);
}





































