#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <unistd.h>

SDL_Surface* resize_surface(SDL_Surface* surface, int width, int height) {
    SDL_Surface* resized_surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        width,
        height,
        surface->format->BitsPerPixel,
        surface->format->Rmask,
        surface->format->Gmask,
        surface->format->Bmask,
        surface->format->Amask
    );

    if (resized_surface == NULL) {
        printf("Erreur lors de la création de la surface redimensionnée : %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SoftStretch(surface, NULL, resized_surface, NULL);
    return resized_surface;
}

int main(int argc, char** argv) {
    SDL_Surface *ecran = NULL;
    SDL_Surface *background = NULL;
    SDL_Surface *button_char1 = NULL;
    SDL_Surface *button_char1_hover = NULL;
    SDL_Surface *button_char2 = NULL;
    SDL_Surface *button_char2_hover = NULL;
    SDL_Surface *button_input1 = NULL;
    SDL_Surface *button_input1_hover = NULL;
    SDL_Surface *button_input2 = NULL;
    SDL_Surface *button_input2_hover = NULL;
    SDL_Surface *button_enter = NULL;
    SDL_Surface *button_enter_hover = NULL;
    SDL_Surface *button_return = NULL;
    SDL_Surface *button_return_hover = NULL;
    SDL_Surface *resized_background = NULL;
    SDL_Surface *resized_button_char1 = NULL;
    SDL_Surface *resized_button_char1_hover = NULL;
    SDL_Surface *resized_button_char2 = NULL;
    SDL_Surface *resized_button_char2_hover = NULL;
    SDL_Surface *resized_button_input1 = NULL;
    SDL_Surface *resized_button_input1_hover = NULL;
    SDL_Surface *resized_button_input2 = NULL;
    SDL_Surface *resized_button_input2_hover = NULL;
    SDL_Surface *resized_button_enter = NULL;
    SDL_Surface *resized_button_enter_hover = NULL;
    SDL_Surface *resized_button_return = NULL;
    SDL_Surface *resized_button_return_hover = NULL;
    SDL_Event event;
    int quitter = 1;

    int selected_character = 0;
    int selected_input = 0;

    Mix_Music *musique = NULL;
    Mix_Chunk *hoverSound = NULL;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Échec d'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    hoverSound = Mix_LoadWAV("UI Generic Button Move Hover Fall Guys Sound - Sound Effect for editing.wav");
    if (!hoverSound) {
        printf("Failed to load hover sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    ecran = SDL_SetVideoMode(1920, 1080, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (ecran == NULL) {
        printf("Échec de création de la fenêtre : %s\n", SDL_GetError());
        return 1;
    }

    musique = Mix_LoadMUS("Assassins Creed 2 OST _ Jesper Kyd - Ezios Family Track 03.mp3");
    if (!musique) {
        printf("Erreur musique: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(musique, -1);
    }

    background = IMG_Load("background.png");
    button_char1 = IMG_Load("character1.png");
    button_char1_hover = IMG_Load("character1 hover.png");
    button_char2 = IMG_Load("character2.png");
    button_char2_hover = IMG_Load("character 2 hover.png");
    button_input1 = IMG_Load("input 1.png");
    button_input1_hover = IMG_Load("input 1 hover.png");
    button_input2 = IMG_Load("input 2.png");
    button_input2_hover = IMG_Load("input 2 hover.png");
    button_enter = IMG_Load("entree.png");
    button_enter_hover = IMG_Load("entree hover.png");
    button_return = IMG_Load("retour.png");
    button_return_hover = IMG_Load("retour hover.png");

    if (!background || !button_char1 || !button_char1_hover || !button_char2 || !button_char2_hover || 
        !button_input1 || !button_input1_hover || !button_input2 || !button_input2_hover || 
        !button_enter || !button_enter_hover || !button_return || !button_return_hover) {
        printf("Erreur de chargement des images : %s\n", IMG_GetError());
        return 1;
    }

    resized_background = resize_surface(background, 1920, 1080);
    int button_width = 400;
    int button_height = 200;
    resized_button_char1 = resize_surface(button_char1, button_width, button_height);
    resized_button_char1_hover = resize_surface(button_char1_hover, button_width, button_height);
    resized_button_char2 = resize_surface(button_char2, button_width, button_height);
    resized_button_char2_hover = resize_surface(button_char2_hover, button_width, button_height);
    resized_button_input1 = resize_surface(button_input1, button_width, button_height);
    resized_button_input1_hover = resize_surface(button_input1_hover, button_width, button_height);
    resized_button_input2 = resize_surface(button_input2, button_width, button_height);
    resized_button_input2_hover = resize_surface(button_input2_hover, button_width, button_height);
    resized_button_enter = resize_surface(button_enter, button_width, button_height);
    resized_button_enter_hover = resize_surface(button_enter_hover, button_width, button_height);
    resized_button_return = resize_surface(button_return, button_width, button_height);
    resized_button_return_hover = resize_surface(button_return_hover, button_width, button_height);

    if (!resized_background || !resized_button_char1 || !resized_button_char1_hover || 
        !resized_button_char2 || !resized_button_char2_hover || !resized_button_input1 || 
        !resized_button_input1_hover || !resized_button_input2 || !resized_button_input2_hover || 
        !resized_button_enter || !resized_button_enter_hover || !resized_button_return || !resized_button_return_hover) {
        printf("Erreur lors du redimensionnement des images : %s\n", SDL_GetError());
        return 1;
    }

    SDL_FreeSurface(background);
    SDL_FreeSurface(button_char1);
    SDL_FreeSurface(button_char1_hover);
    SDL_FreeSurface(button_char2);
    SDL_FreeSurface(button_char2_hover);
    SDL_FreeSurface(button_input1);
    SDL_FreeSurface(button_input1_hover);
    SDL_FreeSurface(button_input2);
    SDL_FreeSurface(button_input2_hover);
    SDL_FreeSurface(button_enter);
    SDL_FreeSurface(button_enter_hover);
    SDL_FreeSurface(button_return);
    SDL_FreeSurface(button_return_hover);

    SDL_Rect position_background = {0, 0};
    int gap = 20;
    int total_width = button_width * 2 + gap;
    int start_x = (1920 - total_width) / 2;
    int start_y = (1080 - button_height * 2 - gap) / 2;

    SDL_Rect position_char1 = {start_x, start_y};
    SDL_Rect position_char2 = {start_x + button_width + gap, start_y};
    SDL_Rect position_input1 = {start_x, start_y + button_height + gap};
    SDL_Rect position_input2 = {start_x + button_width + gap, start_y + button_height + gap};
    SDL_Rect position_enter = {(1920 - button_width) / 2, 1080 - button_height - 20};
    SDL_Rect position_return = {20, 1080 - button_height - 20};

    SDL_Rect hitbox_char1 = position_char1;
    SDL_Rect hitbox_char2 = position_char2;
    SDL_Rect hitbox_input1 = position_input1;
    SDL_Rect hitbox_input2 = position_input2;
    SDL_Rect hitbox_enter = position_enter;
    SDL_Rect hitbox_return = position_return;

    hitbox_char1.w = button_width;
    hitbox_char1.h = button_height;
    hitbox_char2.w = button_width;
    hitbox_char2.h = button_height;
    hitbox_input1.w = button_width;
    hitbox_input1.h = button_height;
    hitbox_input2.w = button_width;
    hitbox_input2.h = button_height;
    hitbox_enter.w = button_width;
    hitbox_enter.h = button_height;
    hitbox_return.w = button_width;
    hitbox_return.h = button_height;

    SDL_Surface* current_button_char1 = resized_button_char1;
    SDL_Surface* current_button_char2 = resized_button_char2;
    SDL_Surface* current_button_input1 = resized_button_input1;
    SDL_Surface* current_button_input2 = resized_button_input2;
    SDL_Surface* current_button_enter = resized_button_enter;
    SDL_Surface* current_button_return = resized_button_return;

    bool is_hovering_char1 = false;
    bool is_hovering_char2 = false;
    bool is_hovering_input1 = false;
    bool is_hovering_input2 = false;
    bool is_hovering_enter = false;
    bool is_hovering_return = false;

    while (quitter) {
        SDL_BlitSurface(resized_background, NULL, ecran, &position_background);
        SDL_BlitSurface(current_button_char1, NULL, ecran, &position_char1);
        SDL_BlitSurface(current_button_char2, NULL, ecran, &position_char2);
        SDL_BlitSurface(current_button_input1, NULL, ecran, &position_input1);
        SDL_BlitSurface(current_button_input2, NULL, ecran, &position_input2);
        SDL_BlitSurface(current_button_enter, NULL, ecran, &position_enter);
        SDL_BlitSurface(current_button_return, NULL, ecran, &position_return);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quitter = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quitter = 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (event.motion.x >= hitbox_char1.x && event.motion.x <= hitbox_char1.x + hitbox_char1.w &&
                        event.motion.y >= hitbox_char1.y && event.motion.y <= hitbox_char1.y + hitbox_char1.h) {
                        if (!is_hovering_char1) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                            is_hovering_char1 = true;
                        }
                        current_button_char1 = resized_button_char1_hover;
                    } else {
                        if (is_hovering_char1) {
                            is_hovering_char1 = false;
                        }
                        if (selected_character != 1) {
                            current_button_char1 = resized_button_char1;
                        }
                    }

                    if (event.motion.x >= hitbox_char2.x && event.motion.x <= hitbox_char2.x + hitbox_char2.w &&
                        event.motion.y >= hitbox_char2.y && event.motion.y <= hitbox_char2.y + hitbox_char2.h) {
                        if (!is_hovering_char2) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                            is_hovering_char2 = true;
                        }
                        current_button_char2 = resized_button_char2_hover;
                    } else {
                        if (is_hovering_char2) {
                            is_hovering_char2 = false;
                        }
                        if (selected_character != 2) {
                            current_button_char2 = resized_button_char2;
                        }
                    }

                    if (event.motion.x >= hitbox_input1.x && event.motion.x <= hitbox_input1.x + hitbox_input1.w &&
                        event.motion.y >= hitbox_input1.y && event.motion.y <= hitbox_input1.y + hitbox_input1.h) {
                        if (!is_hovering_input1) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                            is_hovering_input1 = true;
                        }
                        current_button_input1 = resized_button_input1_hover;
                    } else {
                        if (is_hovering_input1) {
                            is_hovering_input1 = false;
                        }
                        if (selected_input != 1) {
                            current_button_input1 = resized_button_input1;
                        }
                    }

                    if (event.motion.x >= hitbox_input2.x && event.motion.x <= hitbox_input2.x + hitbox_input2.w &&
                        event.motion.y >= hitbox_input2.y && event.motion.y <= hitbox_input2.y + hitbox_input2.h) {
                        if (!is_hovering_input2) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                            is_hovering_input2 = true;
                        }
                        current_button_input2 = resized_button_input2_hover;
                    } else {
                        if (is_hovering_input2) {
                            is_hovering_input2 = false;
                        }
                        if (selected_input != 2) {
                            current_button_input2 = resized_button_input2;
                        }
                    }

                    if (event.motion.x >= hitbox_enter.x && event.motion.x <= hitbox_enter.x + hitbox_enter.w &&
                        event.motion.y >= hitbox_enter.y && event.motion.y <= hitbox_enter.y + hitbox_enter.h) {
                        if (!is_hovering_enter) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                            is_hovering_enter = true;
                        }
                        current_button_enter = resized_button_enter_hover;
                    } else {
                        if (is_hovering_enter) {
                            is_hovering_enter = false;
                        }
                        current_button_enter = resized_button_enter;
                    }

                    if (event.motion.x >= hitbox_return.x && event.motion.x <= hitbox_return.x + hitbox_return.w &&
                        event.motion.y >= hitbox_return.y && event.motion.y <= hitbox_return.y + hitbox_return.h) {
                        if (!is_hovering_return) {
                            Mix_PlayChannel(-1, hoverSound, 0);
                            is_hovering_return = true;
                        }
                        current_button_return = resized_button_return_hover;
                    } else {
                        if (is_hovering_return) {
                            is_hovering_return = false;
                        }
                        current_button_return = resized_button_return;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int x = event.button.x;
                        int y = event.button.y;

                        if (x >= hitbox_char1.x && x <= hitbox_char1.x + hitbox_char1.w &&
                            y >= hitbox_char1.y && y <= hitbox_char1.y + hitbox_char1.h) {
                            printf("Character 1 selected\n");
                            selected_character = 1;
                            current_button_char1 = resized_button_char1_hover;
                            current_button_char2 = resized_button_char2;
                        }
                        if (x >= hitbox_char2.x && x <= hitbox_char2.x + hitbox_char2.w &&
                            y >= hitbox_char2.y && y <= hitbox_char2.y + hitbox_char2.h) {
                            printf("Character 2 selected\n");
                            selected_character = 2;
                            current_button_char2 = resized_button_char2_hover;
                            current_button_char1 = resized_button_char1;
                        }
                        if (x >= hitbox_input1.x && x <= hitbox_input1.x + hitbox_input1.w &&
                            y >= hitbox_input1.y && y <= hitbox_input1.y + hitbox_input1.h) {
                            printf("Input 1 selected\n");
                            selected_input = 1;
                            current_button_input1 = resized_button_input1_hover;
                            current_button_input2 = resized_button_input2;
                        }
                        if (x >= hitbox_input2.x && x <= hitbox_input2.x + hitbox_input2.w &&
                            y >= hitbox_input2.y && y <= hitbox_input2.y + hitbox_input2.h) {
                            printf("Input 2 selected\n");
                            selected_input = 2;
                            current_button_input2 = resized_button_input2_hover;
                            current_button_input1 = resized_button_input1;
                        }
                        if (x >= hitbox_enter.x && x <= hitbox_enter.x + hitbox_enter.w &&
    				y >= hitbox_enter.y && y <= hitbox_enter.y + hitbox_enter.h) {
    				printf("Enter button pressed\n");
    				if (selected_character != 0 && selected_input != 0) {
        				printf("Selected Character: %d, Selected Input: %d\n", selected_character, selected_input);
        				FILE *file = fopen("selection.dat", "wb");
        				if (file) {
            					fwrite(&selected_character, sizeof(int), 1, file);
            					fwrite(&selected_input, sizeof(int), 1, file);
            					fclose(file);
        				}
                                SDL_FreeSurface(resized_background);
                                SDL_FreeSurface(resized_button_char1);
                                SDL_FreeSurface(resized_button_char1_hover);
                                SDL_FreeSurface(resized_button_char2);
                                SDL_FreeSurface(resized_button_char2_hover);
                                SDL_FreeSurface(resized_button_input1);
                                SDL_FreeSurface(resized_button_input1_hover);
                                SDL_FreeSurface(resized_button_input2);
                                SDL_FreeSurface(resized_button_input2_hover);
                                SDL_FreeSurface(resized_button_enter);
                                SDL_FreeSurface(resized_button_enter_hover);
                                SDL_FreeSurface(resized_button_return);
                                SDL_FreeSurface(resized_button_return_hover);
                                Mix_FreeMusic(musique);
                                Mix_FreeChunk(hoverSound);
                                Mix_CloseAudio();
                                SDL_Quit();
                                chdir("../final2");
                                system("./game");
                                quitter = 0;
                            } else {
                                printf("Please select both a character and an input.\n");
                            }
                        }
                        if (x >= hitbox_return.x && x <= hitbox_return.x + hitbox_return.w &&
                            y >= hitbox_return.y && y <= hitbox_return.y + hitbox_return.h) {
                            printf("Return button pressed\n");
                            SDL_FreeSurface(resized_background);
                            SDL_FreeSurface(resized_button_char1);
                            SDL_FreeSurface(resized_button_char1_hover);
                            SDL_FreeSurface(resized_button_char2);
                            SDL_FreeSurface(resized_button_char2_hover);
                            SDL_FreeSurface(resized_button_input1);
                            SDL_FreeSurface(resized_button_input1_hover);
                            SDL_FreeSurface(resized_button_input2);
                            SDL_FreeSurface(resized_button_input2_hover);
                            SDL_FreeSurface(resized_button_enter);
                            SDL_FreeSurface(resized_button_enter_hover);
                            SDL_FreeSurface(resized_button_return);
                            SDL_FreeSurface(resized_button_return_hover);
                            Mix_FreeMusic(musique);
                            Mix_FreeChunk(hoverSound);
                            Mix_CloseAudio();
                            SDL_Quit();
                            system("./jouer");
                            quitter = 0;
                        }
                    }
                    break;
            }
        }

        SDL_Flip(ecran);
    }

    SDL_FreeSurface(resized_background);
    SDL_FreeSurface(resized_button_char1);
    SDL_FreeSurface(resized_button_char1_hover);
    SDL_FreeSurface(resized_button_char2);
    SDL_FreeSurface(resized_button_char2_hover);
    SDL_FreeSurface(resized_button_input1);
    SDL_FreeSurface(resized_button_input1_hover);
    SDL_FreeSurface(resized_button_input2);
    SDL_FreeSurface(resized_button_input2_hover);
    SDL_FreeSurface(resized_button_enter);
    SDL_FreeSurface(resized_button_enter_hover);
    SDL_FreeSurface(resized_button_return);
    SDL_FreeSurface(resized_button_return_hover);

    Mix_FreeMusic(musique);
    Mix_FreeChunk(hoverSound);
    Mix_CloseAudio();

    SDL_Quit();

    return 0;
}
