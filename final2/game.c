#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include "game.h"

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* static_platform_img = NULL;
SDL_Surface* moving_platform_img = NULL;
SDL_Surface* boss_img = NULL;
SDL_Surface* boss_attack_img = NULL;
SDL_Surface* player_idle_img = NULL;
SDL_Surface* player_walk_imgs[WALK_ANIMATION_FRAMES];
SDL_Surface* player_attack_img = NULL;
SDL_Surface* player_idle_img2 = NULL;
SDL_Surface* player_walk_imgs2[WALK_ANIMATION_FRAMES];
SDL_Surface* player_attack_img2 = NULL;
SDL_Surface* background_img = NULL;
SDL_Surface* vertical_enemy_img = NULL;
SDL_Surface* fall_effect_img = NULL;
SDL_Surface* pause_img = NULL;
SDL_Surface* save_button_img = NULL;
SDL_Surface* resume_button_img = NULL;
Mix_Chunk* stop_sound = NULL;
Mix_Chunk* pause_sound = NULL;
Mix_Music* background_music = NULL;

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

bool checkPixelPerfectCollision(SDL_Surface* src, SDL_Rect src_rect, SDL_Surface* dst, SDL_Rect dst_rect) {
    if (!src || !dst || !checkCollision(src_rect, dst_rect)) return false;
    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);
    bool collision = false;
    int startX = MAX(src_rect.x, dst_rect.x), endX = MIN(src_rect.x + src_rect.w, dst_rect.x + dst_rect.w);
    int startY = MAX(src_rect.y, dst_rect.y), endY = MIN(src_rect.y + src_rect.h, dst_rect.y + dst_rect.h);
    for (int y = startY; y < endY && !collision; y++) {
        for (int x = startX; x < endX && !collision; x++) {
            Uint32 src_pixel = ((Uint32*)src->pixels)[(y - src_rect.y) * src->w + (x - src_rect.x)];
            Uint32 dst_pixel = ((Uint32*)dst->pixels)[(y - dst_rect.y) * dst->w + (x - dst_rect.x)];
            if ((src_pixel >> 24) && (dst_pixel >> 24)) collision = true;
        }
    }
    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
    return collision;
}

SDL_Surface* flipSurface(SDL_Surface* surface) {
    if (!surface) return NULL;
    SDL_Surface* flipped = SDL_ConvertSurface(surface, surface->format, surface->flags);
    if (!flipped) return NULL;
    if (surface->flags & SDL_SRCCOLORKEY) SDL_SetColorKey(flipped, SDL_SRCCOLORKEY, surface->format->colorkey);
    if (surface->flags & SDL_SRCALPHA) SDL_SetAlpha(flipped, SDL_SRCALPHA, surface->format->alpha);
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
    if (SDL_MUSTLOCK(flipped)) SDL_LockSurface(flipped);
    int width = surface->w, height = surface->h, bpp = surface->format->BytesPerPixel;
    int src_pitch = surface->pitch, dst_pitch = flipped->pitch;
    for (int y = 0; y < height; y++) {
        Uint8* src_row = (Uint8*)surface->pixels + y * src_pitch;
        Uint8* dst_row = (Uint8*)flipped->pixels + y * dst_pitch;
        for (int x = 0; x < width; x++) {
            Uint8* src_pixel = src_row + x * bpp;
            Uint8* dst_pixel = dst_row + (width - 1 - x) * bpp;
            memcpy(dst_pixel, src_pixel, bpp);
        }
    }
    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    if (SDL_MUSTLOCK(flipped)) SDL_UnlockSurface(flipped);
    return flipped;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() < 0 || Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) exit(1);
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) exit(1);
    SDL_WM_SetCaption("Enhanced Platform Game", NULL);
    srand((unsigned int)time(NULL));
}

void loadResources(int character_id) {
    font = TTF_OpenFont("arial.ttf", 24);
    static_platform_img = IMG_Load("platform_static.png");
    moving_platform_img = IMG_Load("platform_moving.png");
    boss_img = IMG_Load("boss.png");
    boss_attack_img = IMG_Load("boss_attack.png");
    if (character_id == 1) {
        player_idle_img = IMG_Load("idle.png");
        char filename[20];
        for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) {
            snprintf(filename, sizeof(filename), "pwalk%d.png", i+1);
            player_walk_imgs[i] = IMG_Load(filename);
        }
        player_attack_img = IMG_Load("player_attack_slash.png");
    } else {
        player_idle_img = IMG_Load("idle2.png");
        char filename[20];
        for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) {
            snprintf(filename, sizeof(filename), "pwalk2_%d.png", i+1);
            player_walk_imgs[i] = IMG_Load(filename);
        }
        player_attack_img = IMG_Load("player_attack_slash2.png");
    }
    background_img = IMG_Load("background.png");
    vertical_enemy_img = IMG_Load("vertical_enemy.png");
    fall_effect_img = IMG_Load("fall.png");
    pause_img = IMG_Load("pause.png");
    save_button_img = IMG_Load("save_button.png");
    resume_button_img = IMG_Load("resume_button.png");
    stop_sound = Mix_LoadWAV("stop.wav");
    pause_sound = Mix_LoadWAV("pause.wav");
    background_music = Mix_LoadMUS("mapost1.wav");
    if (!font || !static_platform_img || !moving_platform_img || !boss_img || !boss_attack_img || 
        !player_idle_img || !player_attack_img || !background_img || !fall_effect_img || !pause_img || 
        !save_button_img || !resume_button_img || !stop_sound || !pause_sound || !background_music) exit(1);
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) if (!player_walk_imgs[i]) exit(1);
    if (!vertical_enemy_img) vertical_enemy_img = boss_img;
    SDL_SetColorKey(player_idle_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_idle_img->format, 0, 0, 0));
    SDL_SetColorKey(player_attack_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_attack_img->format, 0, 0, 0));
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) SDL_SetColorKey(player_walk_imgs[i], SDL_SRCCOLORKEY, SDL_MapRGB(player_walk_imgs[i]->format, 0, 0, 0));
}

void initPlayer(Player* p, int character_id) {
    memset(p, 0, sizeof(Player));
    p->idle_sprite = player_idle_img;
    memcpy(p->walk_sprites, player_walk_imgs, sizeof(player_walk_imgs));
    p->attack_sprite = player_attack_img;
    p->pos = (SDL_Rect){PLAYER_X_POSITION, GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT, BOSS_WIDTH, BOSS_HEIGHT};
    p->health = PLAYER_HEALTH;
    p->lives = PLAYER_LIVES;
    p->facingRight = true;
    p->lastGroundY = p->pos.y;
}

void initBackground(Background* bg) {
    memset(bg, 0, sizeof(Background));
    bg->img = background_img;
    bg->width = bg->img ? bg->img->w : SCREEN_WIDTH * 3;
    bg->startTime = SDL_GetTicks();
    bg->state = GAME_STATE_MENU;
}

void initPlatforms(Platform platforms[], int* count, int worldWidth) {
    memset(platforms, 0, sizeof(Platform) * MAX_PLATFORMS);
    int platformCount = 0;
    platforms[platformCount++] = (Platform){{0, GROUND_PLATFORM_HEIGHT, worldWidth, PLATFORM_HEIGHT}, static_platform_img, false, true, false, 0, 0, 0};
    int currentX = 0, lastY = GROUND_PLATFORM_HEIGHT;
    while (platformCount < MAX_PLATFORMS && currentX < worldWidth - END_MARGIN) {
        int spacing = MIN_PLATFORM_SPACING + rand() % (MAX_PLATFORM_SPACING - MIN_PLATFORM_SPACING);
        int y = CLAMP(lastY - 100 + rand() % 200, 150, GROUND_PLATFORM_HEIGHT - 50);
        bool moving = rand() % 100 < MOVING_PLATFORM_CHANCE;
        platforms[platformCount++] = (Platform){
            {currentX, y, PLATFORM_WIDTH, PLATFORM_HEIGHT},
            moving ? moving_platform_img : static_platform_img,
            false, true, moving,
            moving ? (rand() % 2 ? 1 : -1) : 0,
            moving ? (100 + rand() % 200) : 0,
            currentX
        };
        currentX += PLATFORM_WIDTH + spacing;
        lastY = y;
    }
    *count = platformCount;
}

void initBoss(Boss* boss, int worldWidth) {
    if (!boss_img || !boss_attack_img) exit(1);
    memset(boss, 0, sizeof(Boss));
    boss->sprite = boss_img;
    boss->attack_sprite = boss_attack_img;
    boss->pos = (SDL_Rect){worldWidth - BOSS_X_OFFSET - BOSS_WIDTH, GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT, BOSS_WIDTH, BOSS_HEIGHT};
    boss->health = BOSS_HEALTH;
    boss->facingLeft = true;
}

void initEnemies(Enemy enemies[], int worldWidth) {
    memset(enemies, 0, sizeof(Enemy) * MAX_ENEMIES);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        bool isVertical = i % 3 == 0;
        enemies[i] = (Enemy){
            isVertical ? vertical_enemy_img : boss_img,
            boss_attack_img,
            isVertical ? vertical_enemy_img : boss_img,
            {500 + i * 300, GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT, BOSS_WIDTH, BOSS_HEIGHT},
            ENEMY_HEALTH,
            false, 0, 0, {0, 0, 0, 0}, true, 0, true, false, false, 0, true, false,
            isVertical,
            isVertical ? 1.5f : 0,
            isVertical ? 100 : 0,
            GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT
        };
    }
}

void initMinimap(Minimap* minimap) {
    memset(minimap, 0, sizeof(Minimap));
    minimap->width = MINIMAP_WIDTH;
    minimap->height = MINIMAP_HEIGHT;
    minimap->scale = MINIMAP_SCALE;
    minimap->enabled = true;
    minimap->position = (SDL_Rect){SCREEN_WIDTH - MINIMAP_WIDTH - 260, 10, MINIMAP_WIDTH + 12, MINIMAP_HEIGHT + 12};
    minimap->backgroundColor = (SDL_Color){0, 0, 0, 255};
    minimap->frameColor = (SDL_Color){200, 0, 0, 255};
    minimap->playerColor = (SDL_Color){0, 255, 0, 255};
    minimap->enemyColor = (SDL_Color){255, 0, 0, 255};
    minimap->bossColor = (SDL_Color){255, 165, 0, 255};
    minimap->platformColor = (SDL_Color){100, 100, 100, 255};
    minimap->movingPlatformColor = (SDL_Color){0, 100, 255, 255};
}

void saveGame(Player* p, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[], int enemiesRemaining) {
    FILE* file = fopen("../menu/savegame.dat", "wb");
    if (!file) return;
    fwrite(p, sizeof(Player), 1, file);
    fwrite(bg, sizeof(Background), 1, file);
    fwrite(&platformCount, sizeof(int), 1, file);
    fwrite(platforms, sizeof(Platform), platformCount, file);
    fwrite(boss, sizeof(Boss), 1, file);
    fwrite(enemies, sizeof(Enemy), MAX_ENEMIES, file);
    fwrite(&enemiesRemaining, sizeof(int), 1, file);
    fclose(file);
}

bool loadGame(Player* p, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining, int character_id) {
    FILE* file = fopen("../menu/savegame.dat", "rb");
    if (!file) return false;
    if (fread(p, sizeof(Player), 1, file) != 1 ||
        fread(bg, sizeof(Background), 1, file) != 1 ||
        fread(platformCount, sizeof(int), 1, file) != 1 ||
        fread(platforms, sizeof(Platform), *platformCount, file) != *platformCount ||
        fread(boss, sizeof(Boss), 1, file) != 1 ||
        fread(enemies, sizeof(Enemy), MAX_ENEMIES, file) != MAX_ENEMIES ||
        fread(enemiesRemaining, sizeof(int), 1, file) != 1) {
        fclose(file);
        return false;
    }
    fclose(file);
    p->idle_sprite = player_idle_img;
    memcpy(p->walk_sprites, player_walk_imgs, sizeof(player_walk_imgs));
    p->attack_sprite = player_attack_img;
    bg->img = background_img;
    for (int i = 0; i < *platformCount; i++) {
        platforms[i].image = platforms[i].isMoving ? moving_platform_img : static_platform_img;
    }
    boss->sprite = boss_img;
    boss->attack_sprite = boss_attack_img;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].sprite = enemies[i].isVertical ? vertical_enemy_img : boss_img;
        enemies[i].attack_sprite = boss_attack_img;
        enemies[i].vertical_sprite = enemies[i].isVertical ? vertical_enemy_img : boss_img;
    }
    return true;
}

bool loadConfig(ControlConfig* controls, int* character_id) {
    FILE* file = fopen("../menuv1/config.dat", "rb");
    if (!file) {
        controls->left = SDLK_LEFT;
        controls->right = SDLK_RIGHT;
        controls->jump = SDLK_UP;
        controls->attack = SDLK_SPACE;
        controls->sprint = SDLK_LCTRL;
        controls->qte = SDLK_x;
        *character_id = 1;
        return false;
    }
    fread(controls, sizeof(ControlConfig), 1, file);
    fread(character_id, sizeof(int), 1, file);
    fclose(file);
    return true;
}

void handleEvents(Player* p, bool* running, Background* bg, Platform platforms[], int* count, Boss* boss, Enemy enemies[], int* enemiesRemaining, ControlConfig* controls) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;
            case SDL_KEYDOWN:
                if (bg->state == GAME_STATE_PLAYING && !bg->timeStopped && !bg->paused) {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (controls->left == SDLK_LEFT) {
                                p->ax = -PLAYER_ACCEL;
                                p->facingRight = false;
                                p->isWalking = true;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (controls->right == SDLK_RIGHT) {
                                p->ax = PLAYER_ACCEL;
                                p->facingRight = true;
                                p->isWalking = true;
                            }
                            break;
                        case SDLK_UP:
                            if (controls->jump == SDLK_UP && !p->isJumping) {
                                p->vy = JUMP_FORCE;
                                p->isJumping = true;
                                p->lastJumpTime = SDL_GetTicks();
                                p->wasInAir = true;
                                bg->targetCameraY = -100;
                            }
                            break;
                        case SDLK_SPACE:
                            if (controls->attack == SDLK_SPACE && SDL_GetTicks() - p->lastAttackTime > PLAYER_ATTACK_DURATION * 2) {
                                p->isAttacking = true;
                                p->lastAttackTime = SDL_GetTicks();
                                p->attackStartTime = SDL_GetTicks();
                                p->attackHitbox = (SDL_Rect){
                                    p->pos.x + (p->facingRight ? p->pos.w : -PLAYER_ATTACK_RANGE),
                                    p->pos.y + (p->pos.h/2 - p->attack_sprite->h/2),
                                    PLAYER_ATTACK_RANGE,
                                    p->attack_sprite->h
                                };
                            }
                            break;
                        case SDLK_LCTRL:
                            if (controls->sprint == SDLK_LCTRL) p->isSprinting = true;
                            break;
                        case SDLK_x:
                            if (controls->qte == SDLK_x && bg->qteActive && SDL_GetTicks() - bg->qteStartTime <= QTE_DURATION) {
                                bg->qteActive = false;
                                bg->timeStopPrep = 0;
                            }
                            break;
                        case SDLK_q:
                            if (controls->left == SDLK_q) {
                                p->ax = -PLAYER_ACCEL;
                                p->facingRight = false;
                                p->isWalking = true;
                            }
                            break;
                        case SDLK_d:
                            if (controls->right == SDLK_d) {
                                p->ax = PLAYER_ACCEL;
                                p->facingRight = true;
                                p->isWalking = true;
                            }
                            break;
                        case SDLK_z:
                            if (controls->jump == SDLK_z && !p->isJumping) {
                                p->vy = JUMP_FORCE;
                                p->isJumping = true;
                                p->lastJumpTime = SDL_GetTicks();
                                p->wasInAir = true;
                                bg->targetCameraY = -100;
                            }
                            break;
                        default:
                            break;
                    }
                } else if (bg->state == GAME_STATE_MENU) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        bg->state = GAME_STATE_PLAYING;
                        bg->needsReset = true;
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) *running = false;
                if (event.key.keysym.sym == SDLK_r && 
                    (bg->state == GAME_STATE_GAME_OVER || bg->state == GAME_STATE_VICTORY)) {
                    bg->needsReset = true;
                    bg->state = GAME_STATE_PLAYING;
                }
                break;
            case SDL_KEYUP:
                if (bg->state == GAME_STATE_PLAYING && !bg->timeStopped && !bg->paused) {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (controls->left == SDLK_LEFT && p->ax < 0) p->ax = 0;
                            if (p->vx == 0) p->isWalking = false;
                            break;
                        case SDLK_RIGHT:
                            if (controls->right == SDLK_RIGHT && p->ax > 0) p->ax = 0;
                            if (p->vx == 0) p->isWalking = false;
                            break;
                        case SDLK_LCTRL:
                            if (controls->sprint == SDLK_LCTRL) p->isSprinting = false;
                            break;
                        case SDLK_q:
                            if (controls->left == SDLK_q && p->ax < 0) p->ax = 0;
                            if (p->vx == 0) p->isWalking = false;
                            break;
                        case SDLK_d:
                            if (controls->right == SDLK_d && p->ax > 0) p->ax = 0;
                            if (p->vx == 0) p->isWalking = false;
                            break;
                        default:
                            break;
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (bg->state == GAME_STATE_PLAYING && event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Rect pauseRect = {SCREEN_WIDTH - pause_img->w - 10, 40, pause_img->w, pause_img->h};
                    if (event.button.x >= pauseRect.x && event.button.x <= pauseRect.x + pauseRect.w &&
                        event.button.y >= pauseRect.y && event.button.y <= pauseRect.y + pauseRect.h) {
                        if (!bg->paused) {
                            bg->paused = true;
                            bg->pauseChannel = Mix_PlayChannel(-1, pause_sound, -1);
                            Mix_PauseMusic();
                            bg->pauseStartTime = SDL_GetTicks();
                        }
                    } else if (bg->paused) {
                        SDL_Rect saveRect = {SCREEN_WIDTH/2 - save_button_img->w/2, SCREEN_HEIGHT/2 - save_button_img->h - 10, save_button_img->w, save_button_img->h};
                        SDL_Rect resumeRect = {SCREEN_WIDTH/2 - resume_button_img->w/2, SCREEN_HEIGHT/2 + 10, resume_button_img->w, resume_button_img->h};
                        if (event.button.x >= saveRect.x && event.button.x <= saveRect.x + saveRect.w &&
                            event.button.y >= saveRect.y && event.button.y <= saveRect.y + saveRect.h) {
                            saveGame(p, bg, platforms, *count, boss, enemies, *enemiesRemaining);
                        } else if (event.button.x >= resumeRect.x && event.button.x <= resumeRect.x + resumeRect.w &&
                                   event.button.y >= resumeRect.y && event.button.y <= resumeRect.y + resumeRect.h) {
                            bg->paused = false;
                            Mix_HaltChannel(bg->pauseChannel);
                            Mix_ResumeMusic();
                            bg->startTime += SDL_GetTicks() - bg->pauseStartTime;
                        }
                    }
                }
                break;
        }
    }
}

void updateBossAI(Boss* boss, Player* player, Background* bg) {
    if (!boss->isActive) {
        if (boss->shouldSpawn && fabs((player->pos.x + bg->cameraX) - boss->pos.x) < BOSS_AGGRO_RANGE) {
            boss->isActive = true;
            boss->lastTimeStopAttempt = SDL_GetTicks();
        }
        return;
    }
    float playerX = player->pos.x + bg->cameraX;
    boss->facingLeft = playerX < boss->pos.x;
    float distance = fabs(playerX - boss->pos.x);
    Uint32 currentTime = SDL_GetTicks();
    if (boss->isActive && currentTime - boss->lastTimeStopAttempt >= BOSS_TIME_STOP_INTERVAL && !bg->qteActive) {
        bg->qteActive = true;
        bg->qteStartTime = currentTime;
        boss->lastTimeStopAttempt = currentTime;
    }
    if (bg->qteActive && currentTime - bg->qteStartTime >= QTE_DURATION && bg->timeStopPrep == 0) {
        bg->qteActive = false;
        Mix_PlayChannel(-1, stop_sound, 0);
        bg->timeStopPrep = currentTime;
    }
    if (bg->timeStopPrep && currentTime - bg->timeStopPrep >= UNSTOPPED_TIME) {
        bg->timeStopped = true;
        bg->timeStopStart = currentTime;
        bg->timeStopDuration = BOSS_TIME_STOP_DURATION;
        bg->timeStopPrep = 0;
    }
    if (bg->timeStopped && currentTime - bg->timeStopStart >= bg->timeStopDuration) {
        bg->timeStopped = false;
        bg->timeStopStart = 0;
        bg->startTime += TIME_RESUMES_AGAIN;
    }
    if (!bg->timeStopped && !bg->paused) {
        boss->vx = distance > BOSS_ATTACK_RANGE ? (playerX < boss->pos.x ? -BOSS_MOVE_SPEED : BOSS_MOVE_SPEED) : 0;
        boss->pos.x += boss->vx;
    }
    if (distance < BOSS_ATTACK_RANGE && currentTime - boss->lastAttackTime > BOSS_ATTACK_COOLDOWN && !bg->paused) {
        boss->isAttacking = true;
        boss->lastAttackTime = currentTime;
        boss->attackStartTime = currentTime;
        boss->attackHitbox = (SDL_Rect){
            boss->facingLeft ? boss->pos.x - boss->pos.w/2 : boss->pos.x + boss->pos.w,
            boss->pos.y,
            boss->pos.w * 1.5,
            boss->pos.h
        };
    }
    if (boss->isAttacking && currentTime - boss->attackStartTime > BOSS_ATTACK_DURATION) {
        boss->isAttacking = false;
        boss->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
}

void updateEnemyAI(Enemy* enemy, Player* p, Background* bg, bool* enemyIsAttacking) {
    if (!enemy->isActive || enemy->isDead || (bg->timeStopped && SDL_GetTicks() - bg->timeStopStart >= UNSTOPPED_TIME) || bg->paused) return;
    Uint32 currentTime = SDL_GetTicks();
    if (enemy->isVertical) {
        enemy->pos.y += enemy->verticalSpeed;
        if (enemy->pos.y > GROUND_PLATFORM_HEIGHT - enemy->pos.h || enemy->pos.y < enemy->originalY - enemy->verticalDistance) {
            enemy->pos.y = CLAMP(enemy->pos.y, enemy->originalY - enemy->verticalDistance, GROUND_PLATFORM_HEIGHT - enemy->pos.h);
            enemy->verticalSpeed *= -1;
        }
    }
    float distance = fabs(p->pos.x + bg->cameraX - enemy->pos.x);
    if (distance < ENEMY_AGGRO_RANGE) {
        enemy->isAggroed = *enemyIsAttacking = true;
        float playerX = p->pos.x + bg->cameraX;
        enemy->facingLeft = playerX < enemy->pos.x;
        enemy->vx = distance > BOSS_ATTACK_RANGE ? (playerX < enemy->pos.x ? -ENEMY_CHASE_SPEED : ENEMY_CHASE_SPEED) : 0;
        if (distance < BOSS_ATTACK_RANGE && currentTime - enemy->lastAttackTime > ENEMY_ATTACK_COOLDOWN) {
            enemy->isAttacking = true;
            enemy->lastAttackTime = currentTime;
            enemy->attackStartTime = currentTime;
            enemy->attackHitbox = (SDL_Rect){
                enemy->facingLeft ? enemy->pos.x - enemy->pos.w/2 : enemy->pos.x + enemy->pos.w,
                enemy->pos.y,
                enemy->pos.w * 1.5,
                enemy->pos.h
            };
        }
    } else {
        enemy->isAggroed = *enemyIsAttacking = false;
        if (!enemy->isMoving && currentTime - enemy->lastMoveTime > ENEMY_IDLE_TIME) {
            enemy->isMoving = true;
            enemy->moveDirection = rand() % 2 ? 1 : -1;
            enemy->facingLeft = enemy->moveDirection == -1;
            enemy->lastMoveTime = currentTime;
        } else if (enemy->isMoving && currentTime - enemy->lastMoveTime > ENEMY_MOVE_TIME) {
            enemy->isMoving = false;
            enemy->vx = 0;
            enemy->lastMoveTime = currentTime;
        } else if (enemy->isMoving) {
            enemy->vx = enemy->moveDirection * ENEMY_ROAM_SPEED;
        }
    }
    enemy->pos.x += enemy->vx;
    if (enemy->isAttacking && currentTime - enemy->attackStartTime > ENEMY_ATTACK_DURATION) {
        enemy->isAttacking = *enemyIsAttacking = false;
        enemy->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
}

void updateGame(Player* p, Platform platforms[], int* count, Background* bg, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    if (bg->state != GAME_STATE_PLAYING || bg->paused || bg->timeStopped) return;
    Uint32 currentTime = SDL_GetTicks();
    p->vx += p->ax;
    float maxSpeed = p->isSprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED;
    p->vx = CLAMP(p->vx, -maxSpeed, maxSpeed);
    if (p->ax == 0) p->vx *= FRICTION;
    if (p->isJumping && p->vy < 0 && currentTime - p->lastJumpTime < 200) p->vy += GRAVITY * JUMP_GRAVITY_SCALE;
    else if (p->isJumping && p->vy < 0) p->vy += GRAVITY * JUMP_PEAK_GRAVITY;
    else p->vy += GRAVITY;
    p->pos.x += p->vx;
    p->pos.y += p->vy;
    if (p->pos.x < 0) p->pos.x = 0;
    if (p->pos.x + p->pos.w > bg->width) p->pos.x = bg->width - p->pos.w;
    p->isOnMovingPlatform = false;
    for (int i = 0; i < *count; i++) {
        if (!platforms[i].isActive) continue;
        if (platforms[i].isMoving) {
            platforms[i].rect.x += platforms[i].moveDirection * MOVING_PLATFORM_SPEED;
            if (abs(platforms[i].rect.x - platforms[i].originalX) > platforms[i].moveDistance) {
                platforms[i].rect.x = CLAMP(platforms[i].rect.x, platforms[i].originalX - platforms[i].moveDistance, platforms[i].originalX + platforms[i].moveDistance);
                platforms[i].moveDirection *= -1;
            }
        }
        SDL_Rect playerBottom = {p->pos.x, p->pos.y + p->pos.h - 5, p->pos.w, 10};
        if (checkCollision(playerBottom, platforms[i].rect) && p->vy >= 0) {
            p->pos.y = platforms[i].rect.y - p->pos.h;
            p->vy = 0;
            p->isJumping = false;
            p->lastGroundY = p->pos.y;
            if (platforms[i].isMoving) {
                p->isOnMovingPlatform = true;
                p->movingPlatformId = i;
                p->pos.x += platforms[i].moveDirection * MOVING_PLATFORM_SPEED;
            }
        } else if (checkCollision(p->pos, platforms[i].rect) && p->vy < 0 && p->pos.y + p->pos.h > platforms[i].rect.y) {
            p->pos.y = platforms[i].rect.y + platforms[i].rect.h;
            p->vy = 0;
        }
    }
    if (!p->isOnMovingPlatform && p->vy > 0 && p->pos.y >= p->lastGroundY) {
        int fallDistance = p->pos.y - p->fallStartY;
        if (fallDistance > FALL_DAMAGE_THRESHOLD && p->wasInAir) {
            int damage = (int)(fallDistance * FALL_DAMAGE_MULTIPLIER);
            p->health -= damage;
            p->fallDamageTime = currentTime;
            p->wasInAir = false;
        }
        p->pos.y = p->lastGroundY;
        p->vy = 0;
        p->isJumping = false;
    } else if (p->vy > 0 && !p->isJumping) {
        p->fallStartY = p->pos.y;
        p->wasInAir = true;
    }
    if (p->isAttacking && currentTime - p->attackStartTime > PLAYER_ATTACK_DURATION) {
        p->isAttacking = false;
        p->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
    bg->cameraX = p->pos.x - SCREEN_WIDTH / 2;
    bg->cameraX = CLAMP(bg->cameraX, 0, bg->width - SCREEN_WIDTH);
    bg->cameraY += (bg->targetCameraY - bg->cameraY) * 0.1f;
    bg->cameraY = CLAMP(bg->cameraY, -MAX_CAMERA_SCROLL, 0);
    updateBossAI(boss, p, bg);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        bool enemyIsAttacking = false;
        updateEnemyAI(&enemies[i], p, bg, &enemyIsAttacking);
        if (enemies[i].isActive && !enemies[i].isDead && checkCollision(p->pos, enemies[i].pos) && !p->isInvincible) {
            p->health -= ENEMY_COLLISION_DAMAGE;
            p->lastHitTime = currentTime;
            p->isInvincible = true;
            bg->score -= HIT_PENALTY;
        }
        if (enemies[i].isAttacking && checkCollision(p->attackHitbox, enemies[i].attackHitbox) && !p->isInvincible) {
            p->health -= ENEMY_ATTACK_DAMAGE;
            p->lastHitTime = currentTime;
            p->isInvincible = true;
            bg->score -= HIT_PENALTY;
        }
        if (p->isAttacking && checkCollision(p->attackHitbox, enemies[i].pos) && enemies[i].isActive && !enemies[i].isDead) {
            enemies[i].health -= PLAYER_ATTACK_DAMAGE;
            if (enemies[i].health <= 0) {
                enemies[i].isDead = true;
                enemies[i].isActive = false;
                (*enemiesRemaining)--;
                bg->score += ENEMY_KILL_SCORE;
            }
        }
    }
    if (boss->isActive && checkCollision(p->pos, boss->pos) && !p->isInvincible) {
        p->health -= BOSS_ATTACK_DAMAGE;
        p->lastHitTime = currentTime;
        p->isInvincible = true;
        bg->score -= HIT_PENALTY;
    }
    if (boss->isAttacking && checkCollision(p->attackHitbox, boss->attackHitbox) && !p->isInvincible) {
        p->health -= BOSS_ATTACK_DAMAGE;
        p->lastHitTime = currentTime;
        p->isInvincible = true;
        bg->score -= HIT_PENALTY;
    }
    if (p->isAttacking && checkCollision(p->attackHitbox, boss->pos) && boss->isActive) {
        boss->health -= PLAYER_ATTACK_DAMAGE;
        if (boss->health <= 0) {
            boss->isActive = false;
            bg->score += BOSS_KILL_SCORE;
            bg->state = GAME_STATE_VICTORY;
        }
    }
    if (p->isInvincible && currentTime - p->lastHitTime > INVINCIBILITY_DURATION) p->isInvincible = false;
    if (p->health <= 0) {
        p->lives--;
        p->health = PLAYER_HEALTH;
        p->pos.x = PLAYER_X_POSITION;
        p->pos.y = GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT;
        p->vx = p->vy = p->ax = 0;
        p->isJumping = false;
        if (p->lives <= 0) bg->state = GAME_STATE_GAME_OVER;
    }
    if (p->pos.x + bg->cameraX > bg->width - LEVEL_END_MARGIN && *enemiesRemaining == 0 && !boss->isActive) {
        bg->state = GAME_STATE_VICTORY;
    }
}

void renderGame(Player* p, Background* bg, Platform platforms[], int count, Boss* boss, Enemy enemies[], int enemiesRemaining, Minimap* minimap) {
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Rect bgRect = {-(int)bg->cameraX % bg->width, (int)bg->cameraY, bg->width, SCREEN_HEIGHT};
    SDL_BlitSurface(bg->img, NULL, screen, &bgRect);
    if (bgRect.x + bg->width < SCREEN_WIDTH) {
        SDL_Rect bgRect2 = {bgRect.x + bg->width, (int)bg->cameraY, bg->width, SCREEN_HEIGHT};
        SDL_BlitSurface(bg->img, NULL, screen, &bgRect2);
    }
    for (int i = 0; i < count; i++) {
        if (!platforms[i].isActive) continue;
        SDL_Rect platformRect = {platforms[i].rect.x - (int)bg->cameraX, platforms[i].rect.y + (int)bg->cameraY, platforms[i].rect.w, platforms[i].rect.h};
        SDL_BlitSurface(platforms[i].image, NULL, screen, &platformRect);
    }
    SDL_Surface* playerSprite = p->isAttacking ? p->attack_sprite : (p->isWalking ? p->walk_sprites[p->current_walk_frame] : p->idle_sprite);
    if (p->isWalking && SDL_GetTicks() - p->last_frame_time > WALK_ANIMATION_DELAY) {
        p->current_walk_frame = (p->current_walk_frame + 1) % WALK_ANIMATION_FRAMES;
        p->last_frame_time = SDL_GetTicks();
    }
    SDL_Surface* flippedSprite = p->facingRight ? playerSprite : flipSurface(playerSprite);
    SDL_Rect playerRect = {p->pos.x - (int)bg->cameraX, p->pos.y + (int)bg->cameraY, p->pos.w, p->pos.h};
    SDL_BlitSurface(flippedSprite, NULL, screen, &playerRect);
    if (!p->facingRight) SDL_FreeSurface(flippedSprite);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        SDL_Surface* enemySprite = enemies[i].isAttacking ? enemies[i].attack_sprite : (enemies[i].isVertical ? enemies[i].vertical_sprite : enemies[i].sprite);
        SDL_Surface* flippedEnemy = enemies[i].facingLeft ? flipSurface(enemySprite) : enemySprite;
        SDL_Rect enemyRect = {enemies[i].pos.x - (int)bg->cameraX, enemies[i].pos.y + (int)bg->cameraY, enemies[i].pos.w, enemies[i].pos.h};
        SDL_BlitSurface(flippedEnemy, NULL, screen, &enemyRect);
        if (enemies[i].facingLeft) SDL_FreeSurface(flippedEnemy);
    }
    if (boss->isActive) {
        SDL_Surface* bossSprite = boss->isAttacking ? boss->attack_sprite : boss->sprite;
        SDL_Surface* flippedBoss = boss->facingLeft ? bossSprite : flipSurface(bossSprite);
        SDL_Rect bossRect = {boss->pos.x - (int)bg->cameraX, boss->pos.y + (int)bg->cameraY, boss->pos.w, boss->pos.h};
        SDL_BlitSurface(flippedBoss, NULL, screen, &bossRect);
        if (!boss->facingLeft) SDL_FreeSurface(flippedBoss);
    }
    if (p->fallDamageTime && SDL_GetTicks() - p->fallDamageTime < FALL_DAMAGE_EFFECT_DURATION) {
        SDL_Rect fallRect = {p->pos.x - (int)bg->cameraX, p->pos.y + (int)bg->cameraY - fall_effect_img->h, fall_effect_img->w, fall_effect_img->h};
        SDL_BlitSurface(fall_effect_img, NULL, screen, &fallRect);
    }
    renderTimer(bg);
    renderHealthBars(p, boss, bg);
    renderEnemyCounter(enemiesRemaining);
    renderPauseButton(bg);
    renderMinimap(minimap, p, bg, platforms, count, boss, enemies);
    renderScore(bg);
    renderLives(p);
    if (bg->qteActive) renderEnemyWarning(bg);
    if (bg->state == GAME_STATE_MENU) renderMenu(bg);
    if (bg->state == GAME_STATE_GAME_OVER || bg->state == GAME_STATE_VICTORY) renderEndMessage(bg);
    if (bg->paused) {
        SDL_Rect saveRect = {SCREEN_WIDTH/2 - save_button_img->w/2, SCREEN_HEIGHT/2 - save_button_img->h - 10, save_button_img->w, save_button_img->h};
        SDL_Rect resumeRect = {SCREEN_WIDTH/2 - resume_button_img->w/2, SCREEN_HEIGHT/2 + 10, resume_button_img->w, resume_button_img->h};
        SDL_BlitSurface(save_button_img, NULL, screen, &saveRect);
        SDL_BlitSurface(resume_button_img, NULL, screen, &resumeRect);
    }
    SDL_Flip(screen);
}

void renderTimer(Background* bg) {
    char timeStr[20];
    Uint32 elapsed = (SDL_GetTicks() - bg->startTime) / 1000;
    snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d", elapsed / 60, elapsed % 60);
    SDL_Color color = {255, 255, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, timeStr, color);
    SDL_Rect textRect = {TIMER_POS_X, TIMER_POS_Y, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void renderHealthBars(Player* p, Boss* boss, Background* bg) {
    SDL_Rect playerHealthBg = {10, 50, 100, 10};
    SDL_Rect playerHealthFg = {10, 50, (int)(100.0f * p->health / PLAYER_HEALTH), 10};
    SDL_FillRect(screen, &playerHealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
    SDL_FillRect(screen, &playerHealthFg, SDL_MapRGB(screen->format, 0, 255, 0));
    if (boss->isActive) {
        SDL_Rect bossHealthBg = {SCREEN_WIDTH - 110, 50, 100, 10};
        SDL_Rect bossHealthFg = {SCREEN_WIDTH - 110, 50, (int)(100.0f * boss->health / BOSS_HEALTH), 10};
        SDL_FillRect(screen, &bossHealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
        SDL_FillRect(screen, &bossHealthFg, SDL_MapRGB(screen->format, 255, 165, 0));
    }
}

void renderEnemyCounter(int enemiesRemaining) {
    char enemyStr[20];
    snprintf(enemyStr, sizeof(enemyStr), "Enemies: %d", enemiesRemaining);
    SDL_Color color = {255, 255, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, enemyStr, color);
    SDL_Rect textRect = {SCREEN_WIDTH - text->w - 10, 70, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void renderPauseButton(Background* bg) {
    SDL_Rect pauseRect = {SCREEN_WIDTH - pause_img->w - 10, 40, pause_img->w, pause_img->h};
    SDL_BlitSurface(pause_img, NULL, screen, &pauseRect);
}

void renderEndMessage(Background* bg) {
    char* message = bg->state == GAME_STATE_GAME_OVER ? "Game Over" : "Victory!";
    SDL_Color color = {255, 255, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, message, color);
    SDL_Rect textRect = {SCREEN_WIDTH/2 - text->w/2, SCREEN_HEIGHT/2 - text->h/2, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void renderMinimap(Minimap* minimap, Player* p, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[]) {
    if (!minimap->enabled) return;
    SDL_FillRect(screen, &minimap->position, SDL_MapRGB(screen->format, minimap->backgroundColor.r, minimap->backgroundColor.g, minimap->backgroundColor.b));
    SDL_Rect frameRect = {minimap->position.x, minimap->position.y, minimap->position.w, minimap->position.h};
    SDL_FillRect(screen, &frameRect, SDL_MapRGB(screen->format, minimap->frameColor.r, minimap->frameColor.g, minimap->frameColor.b));
    SDL_Rect innerRect = {minimap->position.x + 2, minimap->position.y + 2, minimap->position.w - 4, minimap->position.h - 4};
    SDL_FillRect(screen, &innerRect, SDL_MapRGB(screen->format, minimap->backgroundColor.r, minimap->backgroundColor.g, minimap->backgroundColor.b));
    for (int i = 0; i < platformCount; i++) {
        if (!platforms[i].isActive) continue;
        SDL_Rect platformRect = {
            minimap->position.x + 2 + (int)(platforms[i].rect.x * minimap->scale),
            minimap->position.y + 2 + (int)(platforms[i].rect.y * minimap->scale),
            (int)(platforms[i].rect.w * minimap->scale),
            (int)(platforms[i].rect.h * minimap->scale)
        };
        SDL_Color color = platforms[i].isMoving ? minimap->movingPlatformColor : minimap->platformColor;
        SDL_FillRect(screen, &platformRect, SDL_MapRGB(screen->format, color.r, color.g, color.b));
    }
    SDL_Rect playerRect = {
        minimap->position.x + 2 + (int)((p->pos.x + bg->cameraX) * minimap->scale),
        minimap->position.y + 2 + (int)(p->pos.y * minimap->scale),
        4, 4
    };
    SDL_FillRect(screen, &playerRect, SDL_MapRGB(screen->format, minimap->playerColor.r, minimap->playerColor.g, minimap->playerColor.b));
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        SDL_Rect enemyRect = {
            minimap->position.x + 2 + (int)(enemies[i].pos.x * minimap->scale),
            minimap->position.y + 2 + (int)(enemies[i].pos.y * minimap->scale),
            3, 3
        };
        SDL_FillRect(screen, &enemyRect, SDL_MapRGB(screen->format, minimap->enemyColor.r, minimap->enemyColor.g, minimap->enemyColor.b));
    }
    if (boss->isActive) {
        SDL_Rect bossRect = {
            minimap->position.x + 2 + (int)(boss->pos.x * minimap->scale),
            minimap->position.y + 2 + (int)(boss->pos.y * minimap->scale),
            5, 5
        };
        SDL_FillRect(screen, &bossRect, SDL_MapRGB(screen->format, minimap->bossColor.r, minimap->bossColor.g, minimap->bossColor.b));
    }
}

void renderMenu(Background* bg) {
    SDL_Color color = {255, 255, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, "Press Enter to Start", color);
    SDL_Rect textRect = {SCREEN_WIDTH/2 - text->w/2, SCREEN_HEIGHT/2 - text->h/2, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void renderScore(Background* bg) {
    char scoreStr[20];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %d", bg->score);
    SDL_Color color = {255, 255, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, scoreStr, color);
    SDL_Rect textRect = {SCORE_POS_X, SCORE_POS_Y, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void renderLives(Player* p) {
    char livesStr[20];
    snprintf(livesStr, sizeof(livesStr), "Lives: %d", p->lives);
    SDL_Color color = {255, 255, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, livesStr, color);
    SDL_Rect textRect = {10, 70, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void renderEnemyWarning(Background* bg) {
    SDL_Color color = {255, 0, 0};
    SDL_Surface* text = TTF_RenderText_Solid(font, "QTE: Press X!", color);
    SDL_Rect textRect = {SCREEN_WIDTH/2 - text->w/2, SCREEN_HEIGHT/4, text->w, text->h};
    SDL_BlitSurface(text, NULL, screen, &textRect);
    SDL_FreeSurface(text);
}

void cleanUp(Player* p, Background* bg, Boss* boss, Minimap* minimap) {
    SDL_FreeSurface(static_platform_img);
    SDL_FreeSurface(moving_platform_img);
    SDL_FreeSurface(boss_img);
    SDL_FreeSurface(boss_attack_img);
    SDL_FreeSurface(player_idle_img);
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) SDL_FreeSurface(player_walk_imgs[i]);
    SDL_FreeSurface(player_attack_img);
    SDL_FreeSurface(background_img);
    SDL_FreeSurface(vertical_enemy_img);
    SDL_FreeSurface(fall_effect_img);
    SDL_FreeSurface(pause_img);
    SDL_FreeSurface(save_button_img);
    SDL_FreeSurface(resume_button_img);
    Mix_FreeChunk(stop_sound);
    Mix_FreeChunk(pause_sound);
    Mix_FreeMusic(background_music);
    TTF_CloseFont(font);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

void resetGame(Player* p, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining, Minimap* minimap, int character_id) {
    initPlayer(p, character_id);
    initBackground(bg);
    initPlatforms(platforms, platformCount, bg->width);
    initBoss(boss, bg->width);
    initEnemies(enemies, bg->width);
    initMinimap(minimap);
    *enemiesRemaining = MAX_ENEMIES;
    bg->score = 0;
    bg->needsReset = false;
}
