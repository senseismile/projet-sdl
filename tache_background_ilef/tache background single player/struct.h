#ifndef STRUCT_H
#define STRUCT_H
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <time.h>
#define SCREEN_H 1080
#define SCREEN_W 1920

//tache personnage


//structure personnage



//structure camera

struct camera
{
	SDL_Rect camera;
	int shift_pressed;
	SDL_Surface *menu1;
	SDL_Surface *menu2;
	int walk_right_permission;
	int walk_left_permission;
	int is_running_right;
	int is_running_left;
	float acceleration;
	Uint32 start_time ;
	Uint32 elapsed_time ;
	SDL_Rect birdpos;
	int birddirection;
	
}; 
typedef struct camera camera;

typedef struct
{
	SDL_Surface *txt; //pointeur sur text
	SDL_Rect pos_txt; //position de text sur l'ecran
	SDL_Color color_txt; //coleur de text
	TTF_Font *police; //pointeur sur la police de text
}texte;

void initcamera ( camera* c);
void scrolling (camera* c);
void scrollingb (camera* c);
void scrollingup (camera* c);
void scrollingdown (camera* c);

void affichertemps(int temps);


//text guide
void initialiser_texte1(texte *text1);
void afficher_texte1(SDL_Surface *screen,texte text1);
void initialiser_texte2(texte *text2);
void afficher_texte2(SDL_Surface *screen,texte text2);
void initialiser_texte3(texte *text3);
void afficher_texte3(SDL_Surface *screen,texte text3);





















#endif
