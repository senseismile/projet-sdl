#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SPEED 6
#define GRAVITY 0.5
#define JUMP_FORCE -12
#define MAX_PLATFORMS 100
#define PLATFORM_WIDTH 100
#define PLATFORM_HEIGHT 20
#define MIN_PLATFORM_SPACING 150
#define MAX_PLATFORM_SPACING 300
#define MOVING_PLATFORM_SPEED 2
#define MOVING_PLATFORM_CHANCE 20
#define PLAYER_X_POSITION 200
#define GROUND_PLATFORM_HEIGHT 550
#define END_MARGIN 50
#define TIMER_POS_X 10
#define TIMER_POS_Y 10

extern SDL_Surface* screen;
extern TTF_Font* font;
extern SDL_Surface* static_platform_img;
extern SDL_Surface* moving_platform_img;

typedef struct {
    SDL_Surface* sprite;
    SDL_Rect pos;
    double vx, vy;
    double ax, ay;
    bool isJumping;
    bool isOnMovingPlatform;
    int movingPlatformId;
} Player;

typedef struct {
    SDL_Surface* img;
    float cameraX;
    int width;
    Uint32 startTime;
} Background;

typedef struct {
    SDL_Rect rect;
    SDL_Surface* image;
    bool isDestructible;
    bool isActive;
    bool isMoving;
    int moveDirection;
    int moveDistance;
    int originalX;
} Platform;

void initSDL();
void loadResources();
void initPlayer(Player* p);
void initBackground(Background* bg);
void initPlatforms(Platform platforms[], int* count, int worldWidth);
void handleEvents(Player* p, bool* running);
void updateGame(Player* p, Platform platforms[], int* count, Background* bg);
void renderGame(Player* p, Background* bg, Platform platforms[], int count);
void renderTimer(Background* bg);
void cleanUp(Player* p, Background* bg);

#endif
