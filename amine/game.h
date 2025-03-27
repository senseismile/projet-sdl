#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

// Screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Physics
#define PLAYER_SPEED 5
#define GRAVITY 0.5
#define JUMP_FORCE -12
#define MAX_PLATFORMS 100

// Platform generation
#define MIN_PLATFORM_SPACING 150
#define MAX_PLATFORM_SPACING 300
#define MOVING_PLATFORM_SPEED 2
#define MOVING_PLATFORM_CHANCE 20 // 20%

// Background
#define BG_SCROLL_RATIO 0.7f

// Game duration (90 seconds)
#define GAME_DURATION_MS 90000

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
    float scrollX;
    int width;
} Background;

typedef struct {
    SDL_Rect rect;
    SDL_Surface* image;
    bool isDestructible;
    bool isActive;
    bool isMoving;
    bool isEndless;
    int timer;
    int moveDirection;
    int moveDistance;
    int originalX;
} Platform;

// Function prototypes
void initSDL();
void loadResources();
void initPlayer(Player* p);
void initBackground(Background* bg);
void initPlatforms(Platform platforms[], int* count);
void handleEvents(Player* p, bool* running);
void updateGame(Player* p, Platform platforms[], int* count, Background* bg);
void renderGame(Player* p, Background* bg, Platform platforms[], int count);
void cleanUp(Player* p, Background* bg);

#endif
