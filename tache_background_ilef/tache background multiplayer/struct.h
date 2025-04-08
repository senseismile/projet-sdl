#ifndef STRUCT_H
#define STRUCT_H
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
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
}; 
typedef struct camera camera;














//tache background





void initcamera ( camera* c);
void initcameramultiplayer ( camera* c);
void scrolling (camera* c);
void scrollingb (camera* c);
void scrollingup (camera* c);
void scrollingdown (camera* c);
//void saveScore(ScoreInfo  s,  char *fileName);
//void bestScore(char *filename, ScoreInfo t[]) ;
//void initialisation_background_music(audio *back_music);
//void liberer_background_music(audio *back_music);



#endif
