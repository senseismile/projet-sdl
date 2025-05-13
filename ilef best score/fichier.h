#ifndef FONCTION_H
#define FONCTION_H
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#define SCREEN_H 527
#define SCREEN_W 936
typedef struct
{
	char *url; //url variable de type char pour stocker les images avant l'utilisation de IMG_Load sur cette variable
	SDL_Rect pos_img_affiche; // la partie de l'image a afficher
	SDL_Rect pos_img_ecran; // position de l'affichage de l'image sur l'ecran
	SDL_Surface *img; //pointeur sur image
}image;

typedef struct
{
	SDL_Surface *txt; //pointeur sur text
	SDL_Rect pos_txt; //position de text sur l'ecran
	SDL_Color color_txt; //coleur de text
	TTF_Font *police; //pointeur sur la police de text
}texte;



void renderText(SDL_Surface *screen, const char *text, TTF_Font *font, SDL_Color color, int x, int y);
void handleTextInput(SDL_Event *event, char *userInput, size_t maxLength);


//image
void initialiser_imageBackground(image *imge);
void initialiser_image2(image *imgsecond);
void initialiser_image3(image *imgthird);
void initialiser_image4(image *imgfourth);
void initialiser_image5(image *imgfifth);
void initialiser_image6(image *imgsixith);
void initialiser_image7(image *imgseventh);
void initialiser_image8(image *imgeighth);
//void initialiser_image9(image *imgnineth);

void afficher_imageBackground(SDL_Surface *screen,image imge);
void afficher_image2(SDL_Surface *screen,image imge);
void afficher_image3(SDL_Surface *screen,image imgthird);
void afficher_image4(SDL_Surface *screen,image imgfourth);
void afficher_image5(SDL_Surface *screen,image imgfifth);
void afficher_image6(SDL_Surface *screen,image imgsixith);
void afficher_image7(SDL_Surface *screen,image imgseventh);
void afficher_image8(SDL_Surface *screen,image imgeighth);
//void afficher_image9(SDL_Surface *screen,image imgnineth);

void libere_image(image imge);


//audio
void initialiser_audio(Mix_Music *music);
void liberer_musique(Mix_Music *music);

void initialiser_audiobref(Mix_Chunk *music);
void liberer_musiquebref(Mix_Chunk *music);

//texte
void initialiser_texte(texte *txte);
void initialiser_texte2(texte *gazelle);
void initialiser_texte3(texte *hunter);

void afficher_texte(SDL_Surface *screen,texte txte);


void liberer_texte(texte txte);

#endif
