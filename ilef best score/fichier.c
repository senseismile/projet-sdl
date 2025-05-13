#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <string.h>
#include "fichier.h"

//traitement des image

	///////////////////////////// Background //////////////////////////////////////
void initialiser_imageBackground(image *imge)
{
	imge->url="background.png";
	imge->img=IMG_Load(imge->url);
	if (imge->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imge->pos_img_ecran.x=0;
	imge->pos_img_ecran.y=0;
	imge->pos_img_affiche.x=0;
	imge->pos_img_affiche.y=0;
	imge->pos_img_affiche.h=SCREEN_H;
	imge->pos_img_affiche.w=SCREEN_W;
}

void afficher_imageBackground(SDL_Surface *screen,image imge)
{
	SDL_BlitSurface(imge.img,NULL, screen,&imge.pos_img_ecran);
}
	///////////////////////////// second background //////////////////////////////////////
void initialiser_image2(image *imgsecond)
{
	imgsecond->url="background2.png";
	imgsecond->img=IMG_Load(imgsecond->url);
	if (imgsecond->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imgsecond->pos_img_affiche.x=0;
	imgsecond->pos_img_affiche.y=0;

}

void afficher_image2(SDL_Surface *screen,image imgsecond)
{
	SDL_BlitSurface(imgsecond.img,NULL,screen,&imgsecond.pos_img_ecran);
}



	///////////////////////////// third picture //////////////////////////////////////
void initialiser_image3(image *imgthird)
{
	imgthird->url="valid.png";
	imgthird->img=IMG_Load(imgthird->url);
	if (imgthird->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imgthird->pos_img_affiche.x=0;
	imgthird->pos_img_affiche.y=0;
	imgthird->pos_img_affiche.h=150;
	imgthird->pos_img_affiche.w=150;
	imgthird->pos_img_ecran.x=570;
	imgthird->pos_img_ecran.y=180;
}

void afficher_image3(SDL_Surface *screen,image imgthird)
{
	SDL_BlitSurface(imgthird.img,NULL,screen,&imgthird.pos_img_ecran);
}
	
	
	///////////////////////////// fourth picture //////////////////////////////////////
void initialiser_image4(image *imgfourth)
{
	imgfourth->url="validpressed.png";
	imgfourth->img=IMG_Load(imgfourth->url);
	if (imgfourth->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}

	imgfourth->pos_img_affiche.x=0;
	imgfourth->pos_img_affiche.y=0;
	imgfourth->pos_img_affiche.h=150;
	imgfourth->pos_img_affiche.w=150;
	imgfourth->pos_img_ecran.x=570;
	imgfourth->pos_img_ecran.y=180;
}

void afficher_image4(SDL_Surface *screen,image imgfourth)
{
	SDL_BlitSurface(imgfourth.img,NULL,screen,&imgfourth.pos_img_ecran);
}	
		///////////////////////////// fifth image //////////////////////////////////////
void initialiser_image5(image *imgfifth)
{
	imgfifth->url="exit.png";
	imgfifth->img=IMG_Load(imgfifth->url);
	if (imgfifth->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imgfifth->pos_img_affiche.x=0;
	imgfifth->pos_img_affiche.y=0;
	imgfifth->pos_img_affiche.h=150;
	imgfifth->pos_img_affiche.w=150;
	imgfifth->pos_img_ecran.x=350;
	imgfifth->pos_img_ecran.y=300;
}

void afficher_image5(SDL_Surface *screen,image imgfifth)
{
	SDL_BlitSurface(imgfifth.img,NULL,screen,&imgfifth.pos_img_ecran);
}

		///////////////////////////// sixth image //////////////////////////////////////
void initialiser_image6(image *imgsixith)
{
	imgsixith->url="exitpressed.png";
	imgsixith->img=IMG_Load(imgsixith->url);
	if (imgsixith->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imgsixith->pos_img_affiche.x=0;
	imgsixith->pos_img_affiche.y=0;
	imgsixith->pos_img_affiche.h=150;
	imgsixith->pos_img_affiche.w=150;
	imgsixith->pos_img_ecran.x=350;
	imgsixith->pos_img_ecran.y=300;
}

void afficher_image6(SDL_Surface *screen,image imgsixith)
{
	SDL_BlitSurface(imgsixith.img,NULL,screen,&imgsixith.pos_img_ecran);
}

		///////////////////////////// seventh image //////////////////////////////////////
void initialiser_image7(image *imgseventh)
{
	imgseventh->url="retour.png";
	imgseventh->img=IMG_Load(imgseventh->url);
	if (imgseventh->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imgseventh->pos_img_affiche.x=0;
	imgseventh->pos_img_affiche.y=0;
	imgseventh->pos_img_affiche.h=150;
	imgseventh->pos_img_affiche.w=150;
	imgseventh->pos_img_ecran.x=650;
	imgseventh->pos_img_ecran.y=300;
}

void afficher_image7(SDL_Surface *screen,image imgseventh)
{
	SDL_BlitSurface(imgseventh.img,NULL,screen,&imgseventh.pos_img_ecran);
}
		///////////////////////////// eighth image //////////////////////////////////////
void initialiser_image8(image *imgeighth)
{
	imgeighth->url="retourpressed.png";
	imgeighth->img=IMG_Load(imgeighth->url);
	if (imgeighth->img == NULL)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return;
	}
	imgeighth->pos_img_affiche.x=0;
	imgeighth->pos_img_affiche.y=0;
	imgeighth->pos_img_affiche.h=150;
	imgeighth->pos_img_affiche.w=150;
	imgeighth->pos_img_ecran.x=650;
	imgeighth->pos_img_ecran.y=300;
}

void afficher_image8(SDL_Surface *screen,image imgeighth)
{
	SDL_BlitSurface(imgeighth.img,NULL,screen,&imgeighth.pos_img_ecran);
}


      ///////////////////////////// liberer les images //////////////////////////////////////
void libere_image(image imge)
{
	SDL_FreeSurface(imge.img);
}


//traitement musique
      ///////////////////////////// Background Music //////////////////////////////////////
void initialiser_audio(Mix_Music *music)
{
	if (Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,1024)==-1)
	{
		printf("%s",SDL_GetError());
	}
	music=Mix_LoadMUS("Yehezkel Raz - Ballerina.mp3");
	Mix_PlayMusic(music,-1);
	Mix_VolumeMusic(MIX_MAX_VOLUME/3.5);
}

void liberer_musique(Mix_Music *music)
{
	Mix_FreeMusic(music);
}
	////////////////////////// Click Sound Effect //////////////////////////////
void initialiser_audiobref(Mix_Chunk *music)
{
	Mix_OpenAudio(40100,MIX_DEFAULT_FORMAT,2,1048);
	music=Mix_LoadWAV("BEEP_Bip de caisse (ID 1417)_LS.wav");
	Mix_PlayChannel(-1,music,0);
	if(music==NULL) 
		printf("%s",SDL_GetError());
}

void liberer_musiquebref(Mix_Chunk *music)
{
	Mix_FreeChunk(music);
}

//traitement texte

      ///////////////////////////// liberer les textes //////////////////////////////////////
void liberer_texte(texte txte)
{
	TTF_CloseFont(txte.police);
	TTF_Quit();
}
	//////////////////////////  Text //////////////////////////////


void renderText(SDL_Surface *screen, const char *text, TTF_Font *font, SDL_Color color, int x, int y) {
    // Create surface with text
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    if (textSurface == NULL) {
       // fprintf(stderr, "Unable to render text: %s\n", TTF_GetError());
        return;
    }

    // Create a rectangle to define the position of the text on the screen
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};

    // Blit the text surface onto the main screen at the specified position
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);

    // Update the screen
    SDL_Flip(screen);

    // Free the text surface
    SDL_FreeSurface(textSurface);
}

// Global variable to store the last key pressed
SDLKey lastKeyPressed = SDLK_UNKNOWN;

void handleTextInput(SDL_Event *event, char *userInput, size_t maxLength) {
    if (event->type == SDL_KEYDOWN) {
        SDLKey key = event->key.keysym.sym;

        // Ignore repeated key presses by checking if the same key was just pressed
        if (key == lastKeyPressed) {
            return; // Do nothing if the key is the same as the last one
        }

        size_t len = strlen(userInput);

        // Handle backspace (delete last character)
        if (key == SDLK_BACKSPACE && len > 0) {
            userInput[len - 1] = '\0';
        }
        // Handle Enter key (submit or process the input)
        else if (key == SDLK_RETURN) {
            printf("User pressed Enter: %s\n", userInput); // You can process the input here
        }
        // Handle other printable characters (avoid overflow)
        else if (len < maxLength - 1) {
            if (key >= SDLK_SPACE && key <= SDLK_z) { // Handle printable characters only
                userInput[len] = (char)key;
                userInput[len + 1] = '\0'; // Null-terminate the string
            }
        }

        // Update the last key pressed
        lastKeyPressed = key;
    }
}

