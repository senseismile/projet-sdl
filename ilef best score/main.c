#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include "fichier.h"
#define MAX_INPUT_LENGTH 256 // Define the maximum input length

int main()
{
	SDL_Surface *screen;
	image IMAGE_8;
	image IMAGE_7;
	image IMAGE_6;
	image IMAGE_5;
	image IMAGE,IMAGE_2,IMAGE_3,IMAGE_4;
	Mix_Music *music;
	Mix_Chunk *mus;
	texte txte;
	SDL_Event event;
	int boucle=1,detect=1,detect2=0,detect3=0;
	int pressed1=0;
	int pressed2=0;
	int pressed3=0;
	
	
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)==-1)
	{
		printf("unable to load background image %s\n",SDL_GetError());
		return -1;
	}

    if (TTF_Init() == -1) {
        fprintf(stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }


	screen=SDL_SetVideoMode(SCREEN_W,SCREEN_H,32,SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE); 
	initialiser_image7(&IMAGE_5);
	initialiser_image6(&IMAGE_6);
	initialiser_image5(&IMAGE_7);
	initialiser_image8(&IMAGE_8);
	initialiser_imageBackground(&IMAGE);
	initialiser_image2(&IMAGE_2); 
	initialiser_image3(&IMAGE_3);
	initialiser_image4(&IMAGE_4);

 char userInput[MAX_INPUT_LENGTH] = ""; // Initialize user input
    size_t maxLength = MAX_INPUT_LENGTH; // Set maxLength

 // Load font
    TTF_Font *font = TTF_OpenFont("ShortBaby-Mg2w.ttf", 24);
    if (font == NULL) {
        fprintf(stderr, "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    // Define text input and color

    SDL_Color textColor = {255, 255, 255}; // White

    // Define the position of the input box (X, Y) and width and height
    int inputX = 240, inputY = 160;
    int inputWidth = 300, inputHeight = 30;


	while(boucle)
	{





		if (detect==1 && detect3==0) 
		{
		afficher_imageBackground(screen,IMAGE);
		afficher_image3(screen,IMAGE_3);
		if(pressed1==1)
{
				afficher_image4(screen,IMAGE_4); 
}
            handleTextInput(&event, userInput, sizeof(userInput));
        renderText(screen, userInput, font, textColor, 240, 160);  // Text appears at (100,100)
  // Render user input text inside the defined input box
            if (strlen(userInput) > 0) {
                renderText(screen, userInput, font, textColor, inputX, inputY);
            }

            // Optionally render a box around the input area (for visual feedback)
            SDL_Rect inputBox = {inputX, inputY, inputWidth, inputHeight};


		}

		else if (detect3==1 && detect==0)
		{

		 afficher_image2(screen,IMAGE_2); 
		 afficher_image5(screen,IMAGE_5); 
		 afficher_image7(screen,IMAGE_7); 
if (pressed2==1)
		{
		 afficher_image6(screen,IMAGE_6); 
		}
if (pressed3==1)
{
		 afficher_image8(screen,IMAGE_8); 
}
	
		}

			
		SDL_Flip(screen); //update screen
		
	 while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
			boucle=0;
			break;
			case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_ESCAPE) //click on echap to exit
                	  boucle =0; //whenever boucle==0 the game will quit
	          
			else if(event.key.keysym.sym == SDLK_RETURN) 
			{
			initialiser_audio(music); // plays the music in the background 
			detect=0;
			detect3=1;

                	 				}
			else if(event.key.keysym.sym == SDLK_b) 
			{
			detect=1;
			detect3=0;
			}


    printf("Key pressed: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                handleTextInput(&event, userInput, maxLength); // Now this works
		break;
		
			case SDL_MOUSEBUTTONDOWN:
			
			if (detect==1 && detect3==0 && event.button.button==SDL_BUTTON_LEFT && (event.motion.y<=289 && event.motion.y>=179 
								   && event.motion.x<=802 && event.motion.x>=571))  
			{
			initialiser_audio(music); // plays the music in the background 
			detect=0;
			detect3=1;		
			}
			else if (detect==0 && detect3==1 && event.button.button==SDL_BUTTON_LEFT && (event.motion.y<=406 && event.motion.y>=298 
								   && event.motion.x<=587 && event.motion.x>=350))  
			{
			boucle=0;
			}
			break;
			case SDL_MOUSEMOTION:
    // Check if the mouse is within the specified region
   		 if (detect==1 && detect3==0 && event.motion.y <= 289 && event.motion.y >= 179 &&
       		 event.motion.x <= 802 && event.motion.x >= 571) {
		pressed1=1;
                initialiser_audiobref(mus); 
        	
    }
else
{
		pressed1=0;
}

		 if (detect3==1 && detect==0 && event.motion.y <= 406 && event.motion.y >= 298 &&
       		 event.motion.x <= 587 && event.motion.x >= 350) {
		pressed2=1;
		initialiser_audiobref(mus); 
        	
    }
else
{
		pressed2=0;
}
		 if (detect3==1 && detect==0 && event.motion.y <= 406 && event.motion.y >= 298 &&
       		 event.motion.x <= 887 && event.motion.x >= 650) {
		pressed3=1;
        	initialiser_audiobref(mus); 
    }
else
{
		pressed3=0;
}
    break;
			
	
	SDL_Flip(screen); //update screen
	}
	}
	}
////////////////////// Free all ////////////////////////////////	
 TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
	libere_image(IMAGE);
	libere_image(IMAGE_2);
	libere_image(IMAGE_3);
	libere_image(IMAGE_4);
	liberer_musique(music);
	liberer_musiquebref(mus);

	SDL_Quit();
	return 0;
}
