#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60
#define FRAME_DELAY (1000/FPS)

#define PLAYER_SPEED 4.0f
#define PLAYER_SPRINT_SPEED 6.0f
#define PLAYER_ACCEL 0.5f
#define FRICTION 0.85f
#define GRAVITY 0.5f
#define JUMP_FORCE -12.0f
#define PLAYER_X_POSITION 200
#define GROUND_PLATFORM_HEIGHT 550
#define PLAYER_HEALTH 50
#define PLAYER_LIVES 3
#define PLAYER_ATTACK_DAMAGE 5
#define PLAYER_ATTACK_DURATION 500
#define PLAYER_ATTACK_RANGE 80
#define INVINCIBILITY_DURATION 1000

#define MAX_PLATFORMS 100
#define PLATFORM_WIDTH 100
#define PLATFORM_HEIGHT 20
#define MIN_PLATFORM_SPACING 150
#define MAX_PLATFORM_SPACING 300
#define MOVING_PLATFORM_SPEED 2.0f
#define MOVING_PLATFORM_CHANCE 20
#define END_MARGIN 50

#define BOSS_WIDTH 50
#define BOSS_HEIGHT 50
#define BOSS_X_OFFSET 100
#define BOSS_HEALTH 1000
#define BOSS_ATTACK_COOLDOWN 2000
#define BOSS_ATTACK_DURATION 500
#define BOSS_ATTACK_DAMAGE 10
#define BOSS_MOVE_SPEED 2.5f
#define BOSS_AGGRO_RANGE 400
#define BOSS_ATTACK_RANGE 150

#define MAX_ENEMIES 20
#define ENEMY_HEALTH 50
#define ENEMY_AGGRO_RANGE 250
#define ENEMY_ROAM_SPEED 1.0f
#define ENEMY_CHASE_SPEED 2.0f
#define ENEMY_ATTACK_DURATION 500
#define ENEMY_ATTACK_COOLDOWN 1500
#define ENEMY_IDLE_TIME 2000
#define ENEMY_MOVE_TIME 1000

#define ENEMY_KILL_SCORE 10
#define BOSS_KILL_SCORE 50
#define HIT_PENALTY 5

#define TIMER_POS_X 10
#define TIMER_POS_Y 10
#define SCORE_POS_X SCREEN_WIDTH/2 - 50
#define SCORE_POS_Y 10
#define MINIMAP_WIDTH 150
#define MINIMAP_HEIGHT 113
#define MINIMAP_SCALE 0.2f
#define LEVEL_END_MARGIN 100

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER,
    GAME_STATE_VICTORY
} GameState;

extern SDL_Surface* screen;
extern TTF_Font* font;
extern SDL_Surface* static_platform_img;
extern SDL_Surface* moving_platform_img;
extern SDL_Surface* boss_img;
extern SDL_Surface* boss_attack_img;
extern SDL_Surface* player_img;
extern SDL_Surface* player2_img;
extern SDL_Surface* player_attack_img;
extern SDL_Surface* background_img;

typedef struct {
    SDL_Surface* sprite;
    SDL_Surface* attack_sprite;
    SDL_Rect pos;
    float vx, vy;
    float ax, ay;
    bool isJumping;
    bool isOnMovingPlatform;
    int movingPlatformId;
    int lastGroundY;
    Uint32 lastJumpTime;
    int health;
    int lives;
    bool isAttacking;
    Uint32 lastAttackTime;
    Uint32 attackStartTime;
    SDL_Rect attackHitbox;
    bool facingRight;
    Uint32 lastHitTime;
    bool isInvincible;
    bool isPlayer2;
    bool isSprinting;
} Player;

typedef struct {
    SDL_Surface* img;
    float cameraX;
    int width;
    Uint32 startTime;
    GameState state;
    bool needsReset;
    int score;
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

typedef struct {
    SDL_Surface* sprite;
    SDL_Surface* attack_sprite;
    SDL_Rect pos;
    int health;
    bool isAttacking;
    Uint32 lastAttackTime;
    Uint32 attackStartTime;
    SDL_Rect attackHitbox;
    bool facingLeft;
    float vx;
    bool isActive;
    bool shouldSpawn;
} Boss;

typedef struct {
    SDL_Surface* sprite;
    SDL_Surface* attack_sprite;
    SDL_Rect pos;
    int health;
    bool isAttacking;
    Uint32 lastAttackTime;
    Uint32 attackStartTime;
    SDL_Rect attackHitbox;
    bool facingLeft;
    float vx;
    bool isActive;
    bool isAggroed;
    bool isDead;
    Uint32 lastMoveTime;
    bool isMoving;
    int moveDirection;
} Enemy;

bool checkCollision(SDL_Rect a, SDL_Rect b);
bool checkPixelPerfectCollision(SDL_Surface* src, SDL_Rect src_rect, SDL_Surface* dst, SDL_Rect dst_rect);
SDL_Surface* flipSurface(SDL_Surface* surface);
void initSDL();
void loadResources();
void initPlayer(Player* p, bool isPlayer2);
void initBackground(Background* bg);
void initPlatforms(Platform platforms[], int* count, int worldWidth);
void initBoss(Boss* boss, int worldWidth);
void initEnemies(Enemy enemies[], int worldWidth);
void handleEvents(Player* p, Player* p2, bool* running, Background* bg);
void updateGame(Player* p, Player* p2, Platform platforms[], int* count, Background* bg, Boss* boss, Enemy enemies[], int* enemiesRemaining);
void renderGame(Player* p, Player* p2, Background* bg, Platform platforms[], int count, Boss* boss, Enemy enemies[], int enemiesRemaining);
void renderTimer(Background* bg);
void renderHealthBars(Player* p, Player* p2, Boss* boss, Background* bg);
void renderEnemyCounter(int enemiesRemaining);
void renderEndMessage(Background* bg);
void renderMinimap(Player* p, Player* p2, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[]);
void renderMenu(Background* bg);
void renderScore(Background* bg);
void renderLives(Player* p, Player* p2);
void cleanUp(Player* p, Player* p2, Background* bg, Boss* boss);
void updateBossAI(Boss* boss, Player* player, Background* bg);
void updateEnemyAI(Enemy* enemy, Player* p1, Player* p2, Background* bg, bool* enemyIsAttacking);
void resetGame(Player* p, Player* p2, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining);
#endif
