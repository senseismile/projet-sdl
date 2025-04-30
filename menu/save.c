#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int screenWidth = 983;
int screenHeight = 560;
int isFullScreen = 0;
Mix_Music* guiMusic = NULL;
Mix_Music* bgMusic = NULL;
int lastHovered = 0;

SDL_Surface* resizeImage(SDL_Surface* image) {
    if (!image) return NULL;
    SDL_Surface* resized = zoomSurface(image, (float)screenWidth / image->w, (float)screenHeight / image->h, SMOOTHING_ON);
    return resized;
}

void playHoverSoundOnce(int hoverIndex) {
    if (lastHovered != hoverIndex) {
        Mix_HaltMusic();
        Mix_PlayMusic(guiMusic, 0);
        lastHovered = hoverIndex;
    }
}

void stopHoverSound() {
    if (lastHovered != 0) {
        Mix_HaltMusic();
        lastHovered = 0;
        Mix_PlayMusic(bgMusic, -1);
    }
}

int loadAndRunGame() {
    if (access("savegame.dat", F_OK) != 0) return 0;
    
    Mix_FreeMusic(guiMusic);
    Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    SDL_Quit();
    
    chdir("../final");
    execl("./game", "./game", "--load", NULL);
    
    perror("Failed to launch game");
    return 1;
}

int main(int argc, char** argv) {
    SDL_Surface *ecran = NULL, *image = NULL, *imageB = NULL, *imageD = NULL, *hoverImage1 = NULL, *hoverImage2 = NULL, *hoverImage3 = NULL, *hoverImage4 = NULL;
    int quitter = 1, isOnImageB = 0, wasImageD = 0;
    SDL_Rect poscranimg = {0, 0};
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() == -1) return 1;
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) return 1;

    guiMusic = Mix_LoadMUS("click.mp3");
    bgMusic = Mix_LoadMUS("Assassins Creed 2 OST _ Jesper Kyd - Ezios Family Track 03.mp3");
    if (!guiMusic || !bgMusic) return 1;

    Mix_PlayMusic(bgMusic, -1);

    ecran = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!ecran) return 1;

    image = resizeImage(IMG_Load("sauv.png"));
    imageB = resizeImage(IMG_Load("r1.png"));
    imageD = resizeImage(IMG_Load("charg.png"));
    hoverImage1 = resizeImage(IMG_Load("4.png"));
    hoverImage2 = resizeImage(IMG_Load("2.png"));
    hoverImage3 = resizeImage(IMG_Load("11.png"));
    hoverImage4 = resizeImage(IMG_Load("loadsave.png"));
    if (!image || !imageB || !imageD || !hoverImage1 || !hoverImage2 || !hoverImage3 || !hoverImage4) return 1;

    int mouseX = 0, mouseY = 0;
    int imageD_x1 = screenWidth * 0.08, imageD_x2 = screenWidth * 0.37;
    int imageD_y1 = screenHeight * 0.68, imageD_y2 = screenHeight * 0.88;
    int imageB_x1 = screenWidth * 0.6, imageB_x2 = screenWidth * 0.85;
    int imageB_y1 = screenHeight * 0.7, imageB_y2 = screenHeight * 0.9;
    int hoverImage3_x1 = 650, hoverImage3_x2 = 897;
    int hoverImage3_y1 = 389, hoverImage3_y2 = 496;
    int hoverImage4_x1 = 543, hoverImage4_x2 = 790;
    int hoverImage4_y1 = 160, hoverImage4_y2 = 263;

    while (quitter) {
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT: quitter = 0; break;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&mouseX, &mouseY);
                if (!isOnImageB) {
                    if (!wasImageD && mouseX >= imageD_x1 && mouseX <= imageD_x2 && mouseY >= imageD_y1 && mouseY <= imageD_y2) {
                        image = hoverImage1;
                        playHoverSoundOnce(1);
                    } else if (wasImageD && mouseX >= imageB_x1 && mouseX <= imageB_x2 && mouseY >= imageB_y1 && mouseY <= imageB_y2) {
                        image = hoverImage2;
                        playHoverSoundOnce(2);
                    } else if (!wasImageD && mouseX >= hoverImage3_x1 && mouseX <= hoverImage3_x2 && mouseY >= hoverImage3_y1 && mouseY <= hoverImage3_y2) {
                        image = hoverImage3;
                        playHoverSoundOnce(3);
                    } else if (wasImageD && mouseX >= hoverImage4_x1 && mouseX <= hoverImage4_x2 && mouseY >= hoverImage4_y1 && mouseY <= hoverImage4_y2) {
                        image = hoverImage4;
                        playHoverSoundOnce(4);
                    } else {
                        image = wasImageD ? imageD : resizeImage(IMG_Load("sauv.png"));
                        stopHoverSound();
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (!wasImageD && mouseX >= imageD_x1 && mouseX <= imageD_x2 && mouseY >= imageD_y1 && mouseY <= imageD_y2) {
                        image = imageD;
                        wasImageD = 1;
                    } else if (wasImageD && mouseX >= imageB_x1 && mouseX <= imageB_x2 && mouseY >= imageB_y1 && mouseY <= imageB_y2) {
                        SDL_Quit();
                        system("./jouer");
                        quitter = 0;
                        isOnImageB = 1;
                        wasImageD = 0;
                    } else if (wasImageD && mouseX >= hoverImage4_x1 && mouseX <= hoverImage4_x2 && mouseY >= hoverImage4_y1 && mouseY <= hoverImage4_y2) {
                        if (loadAndRunGame() == 0) {
                            quitter = 0;
                        }
                    }
                }
                break;
        }
        SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
        SDL_BlitSurface(image, NULL, ecran, &poscranimg);
        SDL_Flip(ecran);
    }

    Mix_FreeMusic(guiMusic);
    Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}
