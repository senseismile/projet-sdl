#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define CARD_WIDTH 236
#define CARD_HEIGHT 236
#define NUM_CARDS 6
#define TIME_LIMIT 30
#define CLOCK_IMAGE_COUNT 12

typedef struct {
    SDL_Surface *screen;
    SDL_Surface *background;
    SDL_Surface *cardBack;
    SDL_Surface *cardImages[3];
    SDL_Surface *winImage;
    SDL_Surface *loseImage;
    SDL_Surface *clockImages[CLOCK_IMAGE_COUNT];
    SDL_Rect cardPositions[NUM_CARDS];
    SDL_Rect clockPos;
    SDL_Rect resPos;
    int cardOrder[NUM_CARDS];
    int revealed[NUM_CARDS];
    int selected[2];
    int pairsFound;
    int running;
    time_t startTime;
} Game;

void shuffleCards(int cardOrder[]) {
    int indices[] = {0, 0, 1, 1, 2, 2};
    int i, j, temp;

    for (i = 0; i < NUM_CARDS; i++) {
        j = rand() % NUM_CARDS;
        temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }
    for (i = 0; i < NUM_CARDS; i++) {
        cardOrder[i] = indices[i];
    }
}

void drawCards(Game *game) {
    int i;
    SDL_Surface *img;
    SDL_BlitSurface(game->background, NULL, game->screen, NULL);
    for (i = 0; i < NUM_CARDS; i++) {
        if (game->revealed[i] || i == game->selected[0] || i == game->selected[1])
            img = game->cardImages[game->cardOrder[i]];
        else
            img = game->cardBack;
        SDL_BlitSurface(img, NULL, game->screen, &game->cardPositions[i]);
    }
}

int getCardIndex(int x, int y, SDL_Rect cardPositions[]) {
    for (int i = 0; i < NUM_CARDS; i++) {
        if (x >= cardPositions[i].x && x <= cardPositions[i].x + CARD_WIDTH &&
            y >= cardPositions[i].y && y <= cardPositions[i].y + CARD_HEIGHT)
            return i;
    }
    return -1;
}

void drawClock(Game *game, int elapsedTime) {
    int remaining = TIME_LIMIT - elapsedTime;
    if (remaining < 0) remaining = 0;
    int index = (remaining * (CLOCK_IMAGE_COUNT - 1)) / TIME_LIMIT;
    if (index < 0 || index >= CLOCK_IMAGE_COUNT) {
        printf("Erreur d'index pour l'horloge : %d\n", index);
        return;
    }
    SDL_BlitSurface(game->clockImages[index], NULL, game->screen, &game->clockPos);
}

int main(int argc, char *argv[]) {
    Game game = {0};
    SDL_Event event;
    int i;
    char filename[50];
    int isWaiting = 0;

    SDL_Init(SDL_INIT_VIDEO);
    game.screen = SDL_SetVideoMode(1344, 756, 32, SDL_SWSURFACE);
    game.background = IMG_Load("back22.png");
    game.cardBack = IMG_Load("back.png");
    game.cardImages[0] = IMG_Load("knight.png");
    game.cardImages[1] = IMG_Load("poseidon.png");
    game.cardImages[2] = IMG_Load("zeus.png");
    game.winImage = IMG_Load("saha.png");
    game.loseImage = IMG_Load("Faux.png");

    if (!game.background || !game.cardBack || !game.cardImages[0] || !game.cardImages[1] || !game.cardImages[2] || !game.winImage || !game.loseImage) {
        printf("Erreur de chargement des images : %s\n", IMG_GetError());
        return 1;
    }

    for (i = 0; i < CLOCK_IMAGE_COUNT; i++) {
        sprintf(filename, "clock/clock %d.png", i);
        game.clockImages[i] = IMG_Load(filename);
        if (!game.clockImages[i]) {
            fprintf(stderr, "Erreur de chargement de %s : %s\n", filename, IMG_GetError());
            return 1;
        }
    }

    srand(time(NULL));
    shuffleCards(game.cardOrder);
    for (i = 0; i < NUM_CARDS; i++) {
        game.cardPositions[i].x = (i % 3) * (CARD_WIDTH + 100) + 186;
        game.cardPositions[i].y = (i / 3) * (CARD_HEIGHT + 100) + 72;
    }

    game.selected[0] = -1;
    game.selected[1] = -1;
    game.startTime = time(NULL);
    game.clockPos.x = 10;
    game.clockPos.y = 10;
    game.resPos.x = 5;
    game.resPos.y = 24;
    game.running = 1;

    while (game.running) {
        time_t currentTime = time(NULL);
        int elapsedTime = (int)(currentTime - game.startTime);

        if (elapsedTime > TIME_LIMIT) {
            SDL_BlitSurface(game.loseImage, NULL, game.screen, &game.resPos);
            SDL_Flip(game.screen);
            SDL_Delay(3000);
            break;
        }

        drawCards(&game);
        drawClock(&game, elapsedTime);
        SDL_Flip(game.screen);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game.running = 0;
            } else if (!isWaiting && event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int cardIndex = getCardIndex(event.button.x, event.button.y, game.cardPositions);
                    if (cardIndex != -1 && !game.revealed[cardIndex]) {
                        if (game.selected[0] == -1) {
                            game.selected[0] = cardIndex;
                        } else if (game.selected[1] == -1 && cardIndex != game.selected[0]) {
                            game.selected[1] = cardIndex;
                            drawCards(&game);
                            drawClock(&game, elapsedTime);
                            SDL_Flip(game.screen);
                            isWaiting = 1;
                            SDL_Delay(500);
                            if (game.cardOrder[game.selected[0]] == game.cardOrder[game.selected[1]]) {
                                game.revealed[game.selected[0]] = 1;
                                game.revealed[game.selected[1]] = 1;
                                game.pairsFound++;
                            }
                            game.selected[0] = game.selected[1] = -1;
                            isWaiting = 0;
                        }
                    }
                }
            }
        }

        if (game.pairsFound == 3) {
            SDL_BlitSurface(game.winImage, NULL, game.screen, &game.resPos);
            SDL_Flip(game.screen);
            SDL_Delay(3000);
            break;
        }
    }

    // Libération sécurisée
    if (game.background) SDL_FreeSurface(game.background);
    if (game.cardBack) SDL_FreeSurface(game.cardBack);
    if (game.winImage) SDL_FreeSurface(game.winImage);
    if (game.loseImage) SDL_FreeSurface(game.loseImage);
    for (i = 0; i < 3; i++) {
        if (game.cardImages[i]) SDL_FreeSurface(game.cardImages[i]);
    }
    for (i = 0; i < CLOCK_IMAGE_COUNT; i++) {
        if (game.clockImages[i]) SDL_FreeSurface(game.clockImages[i]);
    }

    SDL_Quit();
    return 0;
}

