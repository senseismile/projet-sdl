#include "game.h"
#include <stdio.h>

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* static_platform_img = NULL;
SDL_Surface* moving_platform_img = NULL;
SDL_Surface* boss_img = NULL;
SDL_Surface* boss_attack_img = NULL;
SDL_Surface* player_idle_img = NULL;
SDL_Surface* player_walk_imgs[WALK_ANIMATION_FRAMES];
SDL_Surface* player_attack_img = NULL;
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
    SDL_Surface* flipped = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel,
                                               0, 0, 0, 0);
    if (!flipped) return NULL;
    Uint8* pixels = (Uint8*)surface->pixels, *flipped_pixels = (Uint8*)flipped->pixels;
    int pitch = surface->pitch, bytesPerPixel = surface->format->BytesPerPixel;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            for (int b = 0; b < bytesPerPixel; b++) {
                flipped_pixels[y * pitch + x * bytesPerPixel + b] = 
                    pixels[y * pitch + (surface->w - 1 - x) * bytesPerPixel + b];
            }
        }
    }
    return flipped;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 || TTF_Init() < 0 || Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        fprintf(stderr, "Video mode failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    SDL_WM_SetCaption("Enhanced Platform Game", NULL);
    srand((unsigned int)time(NULL));
}

void loadResources() {
    font = TTF_OpenFont("arial.ttf", 24);
    static_platform_img = IMG_Load("platform_static.png");
    moving_platform_img = IMG_Load("platform_moving.png");
    boss_img = IMG_Load("boss.png");
    boss_attack_img = IMG_Load("boss_attack.png");
    player_idle_img = IMG_Load("idle.png");
    char filename[20];
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) {
        snprintf(filename, sizeof(filename), "pwalk%d.png", i+1);
        player_walk_imgs[i] = IMG_Load(filename);
    }
    player_attack_img = IMG_Load("player_attack_slash.png");
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
        !save_button_img || !resume_button_img || !stop_sound || !pause_sound || !background_music) {
        fprintf(stderr, "Resource load failed: %s\n", IMG_GetError());
        exit(1);
    }
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) {
        if (!player_walk_imgs[i]) {
            fprintf(stderr, "Walk frame %d load failed\n", i+1);
            exit(1);
        }
    }
    if (!vertical_enemy_img) vertical_enemy_img = boss_img;
    SDL_SetColorKey(player_idle_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_idle_img->format, 0, 0, 0));
    SDL_SetColorKey(player_attack_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_attack_img->format, 0, 0, 0));
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) {
        SDL_SetColorKey(player_walk_imgs[i], SDL_SRCCOLORKEY, SDL_MapRGB(player_walk_imgs[i]->format, 0, 0, 0));
    }
}

void initPlayer(Player* p) {
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
    FILE* file = fopen("savegame.dat", "wb");
    if (!file) return;
    fwrite(p, sizeof(Player), 1, file);
    fwrite(bg, sizeof(Background), 1, file);
    fwrite craving = (SDL_Surface*[]) {volume0, volume32, volume64, volume96, volume128, bouton, retour, bouton_hover};
    for (int i = 0; i < 8; i++) SDL_FreeSurface(resizedImages[i]);
    TTF_CloseFont(font);
    TTF_CloseFont(font_retour);
    Mix_FreeMusic(musique);
    Mix_FreeChunk(clickSound);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}

bool loadGame(Player* p, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    FILE* file = fopen("savegame.dat", "rb");
    if (!file) {
        fprintf(stderr, "Failed to open savegame.dat for reading\n");
        return false;
    }
    if (fread(p, sizeof(Player), 1, file) != 1 ||
        fread(bg, sizeof(Background), 1, file) != 1 ||
        fread(platformCount, sizeof(int), 1, file) != 1 ||
        fread(platforms, sizeof(Platform), *platformCount, file) != (size_t)*platformCount ||
        fread(boss, sizeof(Boss), 1, file) != 1 ||
        fread(enemies, sizeof(Enemy), MAX_ENEMIES, file) != MAX_ENEMIES ||
        fread(enemiesRemaining, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Failed to read savegame.dat\n");
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

void handleEvents(Player* p, bool* running, Background* bg, Platform platforms[], int* count, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
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
                            p->ax = -PLAYER_ACCEL; 
                            p->facingRight = false;
                            p->isWalking = true;
                            break;
                        case SDLK_RIGHT: 
                            p->ax = PLAYER_ACCEL; 
                            p->facingRight = true;
                            p->isWalking = true;
                            break;
                        case SDLK_UP:
                            if (!p->isJumping) {
                                p->vy = JUMP_FORCE;
                                p->isJumping = true;
                                p->lastJumpTime = SDL_GetTicks();
                                p->wasInAir = true;
                                bg->targetCameraY = -100;
                            }
                            break;
                        case SDLK_SPACE:
                            if (SDL_GetTicks() - p->lastAttackTime > PLAYER_ATTACK_DURATION * 2) {
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
                            p->isSprinting = true;
                            break;
                        case SDLK_x:
                            if (bg->qteActive && SDL_GetTicks() - bg->qteStartTime <= QTE_DURATION) {
                                bg->qteActive = false;
                                bg->timeStopPrep = 0;
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
                            if (p->ax < 0) p->ax = 0; 
                            if (p->vx == 0) p->isWalking = false;
                            break;
                        case SDLK_RIGHT: 
                            if (p->ax > 0) p->ax = 0; 
                            if (p->vx == 0) p->isWalking = false;
                            break;
                        case SDLK_LCTRL: 
                            p->isSprinting = false; 
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
            enemy->last
            enemy->lastMoveTime = currentTime;
        }
        if (enemy->isMoving) {
            enemy->vx = enemy->moveDirection * ENEMY_ROAM_SPEED;
        }
    }
    if (enemy->isAttacking && currentTime - enemy->attackStartTime > ENEMY_ATTACK_DURATION) {
        enemy->isAttacking = false;
        enemy->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
    if (!enemy->isVertical) {
        enemy->pos.x += enemy->vx;
    }
}

void resetGame(Player* p, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining, Minimap* minimap) {
    initPlayer(p);
    initBackground(bg);
    initPlatforms(platforms, platformCount, bg->width);
    initBoss(boss, bg->width);
    initEnemies(enemies, bg->width);
    initMinimap(minimap);
    *enemiesRemaining = MAX_ENEMIES;
    bg->state = GAME_STATE_PLAYING;
    bg->needsReset = false;
    Mix_PlayMusic(background_music, -1);
}

void updateGame(Player* p, Platform platforms[], int* count, Background* bg, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    if (bg->state != GAME_STATE_PLAYING || bg->paused) return;
    Uint32 currentTime = SDL_GetTicks();
    bool enemyIsAttacking = false;

    // Update player
    float maxSpeed = p->isSprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED;
    p->vx += p->ax;
    p->vx = CLAMP(p->vx * FRICTION, -maxSpeed, maxSpeed);
    p->pos.x += (int)p->vx;
    p->pos.x = CLAMP(p->pos.x, 0, bg->width - p->pos.w);

    // Apply gravity
    float gravity = (p->isJumping && p->vy < 0 && (currentTime - p->lastJumpTime < 150)) ? GRAVITY * JUMP_GRAVITY_SCALE : 
                    (p->isJumping && p->vy < 0) ? GRAVITY * JUMP_PEAK_GRAVITY : GRAVITY;
    p->vy += gravity;
    p->pos.y += (int)p->vy;

    // Platform collision
    p->isOnMovingPlatform = false;
    for (int i = 0; i < *count; i++) {
        if (!platforms[i].isActive) continue;
        SDL_Rect platformRect = platforms[i].rect;
        if (checkCollision(p->pos, platformRect)) {
            if (p->vy > 0 && p->pos.y + p->pos.h - p->vy <= platformRect.y) {
                p->pos.y = platformRect.y - p->pos.h;
                p->vy = 0;
                p->isJumping = false;
                p->lastGroundY = p->pos.y;
                if (platforms[i].isMoving) {
                    p->isOnMovingPlatform = true;
                    p->movingPlatformId = i;
                }
            } else if (p->vy < 0 && p->pos.y - p->vy >= platformRect.y + platformRect.h) {
                p->pos.y = platformRect.y + platformRect.h;
                p->vy = 0;
            } else if (p->vx > 0 && p->pos.x + p->pos.w - p->vx <= platformRect.x) {
                p->pos.x = platformRect.x - p->pos.w;
                p->vx = 0;
            } else if (p->vx < 0 && p->pos.x - p->vx >= platformRect.x + platformRect.w) {
                p->pos.x = platformRect.x + platformRect.w;
                p->vx = 0;
            }
        }
    }

    // Moving platform movement
    if (p->isOnMovingPlatform) {
        Platform* plat = &platforms[p->movingPlatformId];
        p->pos.x += plat->moveDirection * MOVING_PLATFORM_SPEED;
        p->pos.x = CLAMP(p->pos.x, 0, bg->width - p->pos.w);
    }

    // Fall damage
    if (!p->isJumping && p->pos.y > p->lastGroundY && p->wasInAir) {
        float fallDistance = p->pos.y - p->fallStartY;
        if (fallDistance > FALL_DAMAGE_THRESHOLD) {
            int damage = (int)(fallDistance * FALL_DAMAGE_MULTIPLIER);
            if (damage > 0 && !p->isInvincible) {
                p->health -= damage;
                p->fallDamageTime = currentTime;
                p->isInvincible = true;
                p->lastHitTime = currentTime;
            }
        }
        p->wasInAir = false;
    } else if (p->isJumping) {
        p->fallStartY = p->pos.y;
        p->wasInAir = true;
    }

    // Update invincibility
    if (p->isInvincible && currentTime - p->lastHitTime > INVINCIBILITY_DURATION) {
        p->isInvincible = false;
    }

    // Update attack
    if (p->isAttacking && currentTime - p->attackStartTime > PLAYER_ATTACK_DURATION) {
        p->isAttacking = false;
        p->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }

    // Update platforms
    for (int i = 0; i < *count; i++) {
        if (!platforms[i].isMoving || !platforms[i].isActive) continue;
        platforms[i].rect.x += platforms[i].moveDirection * MOVING_PLATFORM_SPEED;
        if (platforms[i].rect.x <= platforms[i].originalX - platforms[i].moveDistance ||
            platforms[i].rect.x >= platforms[i].originalX + platforms[i].moveDistance) {
            platforms[i].moveDirection *= -1;
        }
        platforms[i].rect.x = CLAMP(platforms[i].rect.x, 0, bg->width - platforms[i].rect.w);
    }

    // Update boss
    updateBossAI(boss, p, bg);
    if (boss->isAttacking && checkCollision(p->pos, boss->attackHitbox) && !p->isInvincible) {
        p->health -= BOSS_ATTACK_DAMAGE;
        p->isInvincible = true;
        p->lastHitTime = currentTime;
        bg->score -= HIT_PENALTY;
    }
    if (p->isAttacking && checkCollision(boss->pos, p->attackHitbox) && boss->isActive) {
        boss->health -= PLAYER_ATTACK_DAMAGE;
        if (boss->health <= 0) {
            boss->isActive = false;
            bg->score += BOSS_KILL_SCORE;
            bg->state = GAME_STATE_VICTORY;
        }
    }

    // Update enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        updateEnemyAI(&enemies[i], p, bg, &enemyIsAttacking);
        if (enemies[i].isAttacking && checkCollision(p->pos, enemies[i].attackHitbox) && !p->isInvincible) {
            p->health -= ENEMY_ATTACK_DAMAGE;
            p->isInvincible = true;
            p->lastHitTime = currentTime;
            bg->score -= HIT_PENALTY;
        }
        if (checkCollision(p->pos, enemies[i].pos) && !p->isInvincible && !enemies[i].isAttacking) {
            p->health -= ENEMY_COLLISION_DAMAGE;
            p->isInvincible = true;
            p->lastHitTime = currentTime;
            bg->score -= HIT_PENALTY;
        }
        if (p->isAttacking && checkCollision(enemies[i].pos, p->attackHitbox)) {
            enemies[i].health -= PLAYER_ATTACK_DAMAGE;
            if (enemies[i].health <= 0) {
                enemies[i].isDead = true;
                enemies[i].isActive = false;
                (*enemiesRemaining)--;
                bg->score += ENEMY_KILL_SCORE;
            }
        }
    }

    // Update camera
    bg->cameraX = p->pos.x - SCREEN_WIDTH / 2;
    bg->cameraX = CLAMP(bg->cameraX, 0, bg->width - SCREEN_WIDTH);
    bg->lastCameraY = bg->cameraY;
    bg->cameraY += (bg->targetCameraY - bg->cameraY) * 0.1f;
    bg->cameraY = CLAMP(bg->cameraY, -MAX_CAMERA_SCROLL, 0);

    // Check game over
    if (p->health <= 0) {
        p->lives--;
        if (p->lives <= 0) {
            bg->state = GAME_STATE_GAME_OVER;
        } else {
            p->health = PLAYER_HEALTH;
            p->pos.x = PLAYER_X_POSITION;
            p->pos.y = p->lastGroundY;
            p->vx = p->vy = 0;
            bg->cameraX = 0;
        }
    }

    // Animation
    if (p->isWalking && !p->isAttacking && !p->isJumping) {
        if (currentTime - p->last_frame_time >= WALK_ANIMATION_DELAY) {
            p->current_walk_frame = (p->current_walk_frame + 1) % WALK_ANIMATION_FRAMES;
            p->last_frame_time = currentTime;
        }
    } else {
        p->current_walk_frame = 0;
    }
}

void renderGame(Player* p, Background* bg, Platform platforms[], int count, Boss* boss, Enemy enemies[], int enemiesRemaining, Minimap* minimap) {
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Rect bgRect = {-(int)bg->cameraX, -(int)bg->cameraY, bg->img->w, bg->img->h};
    SDL_BlitSurface(bg->img, NULL, screen, &bgRect);

    // Render platforms
    for (int i = 0; i < count; i++) {
        if (!platforms[i].isActive) continue;
        SDL_Rect platformRect = {platforms[i].rect.x - (int)bg->cameraX, platforms[i].rect.y - (int)bg->cameraY, platforms[i].rect.w, platforms[i].rect.h};
        SDL_BlitSurface(platforms[i].image, NULL, screen, &platformRect);
    }

    // Render enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        SDL_Surface* sprite = enemies[i].isAttacking ? enemies[i].attack_sprite : (enemies[i].isVertical ? enemies[i].vertical_sprite : enemies[i].sprite);
        SDL_Rect enemyRect = {enemies[i].pos.x - (int)bg->cameraX, enemies[i].pos.y - (int)bg->cameraY, enemies[i].pos.w, enemies[i].pos.h};
        if (enemies[i].facingLeft) {
            SDL_Surface* flipped = flipSurface(sprite);
            SDL_BlitSurface(flipped, NULL, screen, &enemyRect);
            SDL_FreeSurface(flipped);
        } else {
            SDL_BlitSurface(sprite, NULL, screen, &enemyRect);
        }
    }

    // Render boss
    if (boss->isActive) {
        SDL_Surface* sprite = boss->isAttacking ? boss->attack_sprite : boss->sprite;
        SDL_Rect bossRect = {boss->pos.x - (int)bg->cameraX, boss->pos.y - (int)bg->cameraY, boss->pos.w, boss->pos.h};
        if (boss->facingLeft) {
            SDL_Surface* flipped = flipSurface(sprite);
            SDL_BlitSurface(flipped, NULL, screen, &bossRect);
            SDL_FreeSurface(flipped);
        } else {
            SDL_BlitSurface(sprite, NULL, screen, &bossRect);
        }
    }

    // Render player
    SDL_Surface* playerSprite = p->isAttacking ? p->attack_sprite : (p->isWalking ? p->walk_sprites[p->current_walk_frame] : p->idle_sprite);
    SDL_Rect playerRect = {p->pos.x - (int)bg->cameraX, p->pos.y - (int)bg->cameraY, p->pos.w, p->pos.h};
    if (!p->facingRight) {
        SDL_Surface* flipped = flipSurface(playerSprite);
        SDL_BlitSurface(flipped, NULL, screen, &playerRect);
        SDL_FreeSurface(flipped);
    } else {
        SDL_BlitSurface(playerSprite, NULL, screen, &playerRect);
    }

    // Render fall effect
    if (p->fallDamageTime && SDL_GetTicks() - p->fallDamageTime < FALL_DAMAGE_EFFECT_DURATION) {
        SDL_Rect fallRect = {p->pos.x - (int)bg->cameraX, p->pos.y - (int)bg->cameraY + p->pos.h, fall_effect_img->w, fall_effect_img->h};
        SDL_BlitSurface(fall_effect_img, NULL, screen, &fallRect);
    }

    // Render UI
    renderTimer(bg);
    renderHealthBars(p, boss, bg);
    renderEnemyCounter(enemiesRemaining);
    renderScore(bg);
    renderLives(p);
    renderMinimap(minimap, p, bg, platforms, count, boss, enemies);
    renderEnemyWarning(bg);
    renderPauseButton(bg);

    if (bg->paused) {
        SDL_Rect saveRect = {SCREEN_WIDTH/2 - save_button_img->w/2, SCREEN_HEIGHT/2 - save_button_img->h - 10, save_button_img->w, save_button_img->h};
        SDL_Rect resumeRect = {SCREEN_WIDTH/2 - resume_button_img->w/2, SCREEN_HEIGHT/2 + 10, resume_button_img->w, resume_button_img->h};
        SDL_BlitSurface(save_button_img, NULL, screen, &saveRect);
        SDL_BlitSurface(resume_button_img, NULL, screen, &resumeRect);
    }

    if (bg->state == GAME_STATE_GAME_OVER || bg->state == GAME_STATE_VICTORY) {
        renderEndMessage(bg);
    }

    SDL_Flip(screen);
}

void renderTimer(Background* bg) {
    if (bg->state != GAME_STATE_PLAYING) return;
    Uint32 elapsed = (SDL_GetTicks() - bg->startTime) / 1000;
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d", elapsed / 60, elapsed % 60);
    SDL_Surface* text = TTF_RenderText_Solid(font, timeStr, (SDL_Color){255, 255, 255, 255});
    if (text) {
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){TIMER_POS_X, TIMER_POS_Y, text->w, text->h});
        SDL_FreeSurface(text);
    }
}

void renderHealthBars(Player* p, Boss* boss, Background* bg) {
    if (bg->state != GAME_STATE_PLAYING) return;
    SDL_Rect playerHealthBar = {TIMER_POS_X, TIMER_POS_Y + 30, p->health * 2, 10};
    SDL_FillRect(screen, &playerHealthBar, SDL_MapRGB(screen->format, 0, 255, 0));
    if (boss->isActive) {
        SDL_Rect bossHealthBar = {SCREEN_WIDTH - BOSS_HEALTH - 10, 10, boss->health / 2, 10};
        SDL_FillRect(screen, &bossHealthBar, SDL_MapRGB(screen->format, 255, 0, 0));
    }
}

void renderEnemyCounter(int enemiesRemaining) {
    char enemyStr[20];
    snprintf(enemyStr, sizeof(enemyStr), "Enemies: %d", enemiesRemaining);
    SDL_Surface* text = TTF_RenderText_Solid(font, enemyStr, (SDL_Color){255, 255, 255, 255});
    if (text) {
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCREEN_WIDTH - 100, 30, text->w, text->h});
        SDL_FreeSurface(text);
    }
}

void renderPauseButton(Background* bg) {
    if (bg->state != GAME_STATE_PLAYING) return;
    SDL_Rect pauseRect = {SCREEN_WIDTH - pause_img->w - 10, 40, pause_img->w, pause_img->h};
    SDL_BlitSurface(pause_img, NULL, screen, &pauseRect);
}

void renderEndMessage(Background* bg) {
    const char* message = bg->state == GAME_STATE_GAME_OVER ? "Game Over! Press R to Retry" : "Victory! Press R to Restart";
    SDL_Surface* text = TTF_RenderText_Solid(font, message, (SDL_Color){255, 255, 255, 255});
    if (text) {
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - text->w/2, SCREEN_HEIGHT/2, text->w, text->h});
        SDL_FreeSurface(text);
    }
}

void renderMinimap(Minimap* minimap, Player* p, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[]) {
    if (!minimap->enabled) return;
    SDL_FillRect(screen, &minimap->position, SDL_MapRGB(screen->format, minimap->backgroundColor.r, minimap->backgroundColor.g, minimap->backgroundColor.b));
    SDL_Rect frameRect = {minimap->position.x, minimap->position.y, minimap->position.w, minimap->position.h};
    SDL_FillRect(screen, &frameRect, SDL_MapRGB(screen->format, minimap->frameColor.r, minimap->frameColor.g, minimap->frameColor.b));
    SDL_Rect innerRect = {minimap->position.x + 6, minimap->position.y + 6, minimap->position.w - 12, minimap->position.h - 12};
    SDL_FillRect(screen, &innerRect, SDL_MapRGB(screen->format, minimap->backgroundColor.r, minimap->backgroundColor.g, minimap->backgroundColor.b));

    // Render platforms
    for (int i = 0; i < platformCount; i++) {
        if (!platforms[i].isActive) continue;
        SDL_Rect platRect = {
            minimap->position.x + 6 + (int)(platforms[i].rect.x * minimap->scale),
            minimap->position.y + 6 + (int)(platforms[i].rect.y * minimap->scale),
            (int)(platforms[i].rect.w * minimap->scale),
            (int)(platforms[i].rect.h * minimap->scale)
        };
        SDL_Color color = platforms[i].isMoving ? minimap->movingPlatformColor : minimap->platformColor;
        SDL_FillRect(screen, &platRect, SDL_MapRGB(screen->format, color.r, color.g, color.b));
    }

    // Render enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        SDL_Rect enemyRect = {
            minimap->position.x + 6 + (int)(enemies[i].pos.x * minimap->scale),
            minimap->position.y + 6 + (int)(enemies[i].pos.y * minimap->scale),
            (int)(enemies[i].pos.w * minimap->scale),
            (int)(enemies[i].pos.h * minimap->scale)
        };
        SDL_FillRect(screen, &enemyRect, SDL_MapRGB(screen->format, minimap->enemyColor.r, minimap->enemyColor.g, minimap->enemyColor.b));
    }

    // Render boss
    if (boss->isActive) {
        SDL_Rect bossRect = {
            minimap->position.x + 6 + (int)(boss->pos.x * minimap->scale),
            minimap->position.y + 6 + (int)(boss->pos.y * minimap->scale),
            (int)(boss->pos.w * minimap->scale),
            (int)(boss->pos.h * minimap->scale)
        };
        SDL_FillRect(screen, &bossRect, SDL_MapRGB(screen->format, minimap->bossColor.r, minimap->bossColor.g, minimap->bossColor.b));
    }

    // Render player
    SDL_Rect playerRect = {
        minimap->position.x + 6 + (int)(p->pos.x * minimap->scale),
        minimap->position.y + 6 + (int)(p->pos.y * minimap->scale),
        (int)(p->pos.w * minimap->scale),
        (int)(p->pos.h * minimap->scale)
    };
    SDL_FillRect(screen, &playerRect, SDL_MapRGB(screen->format, minimap->playerColor.r, minimap->playerColor.g, minimap->playerColor.b));
}

void renderScore(Background* bg) {
    char scoreStr[20];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %d", bg->score);
    SDL_Surface* text = TTF_RenderText_Solid(font, scoreStr, (SDL_Color){255, 255, 255, 255});
    if (text) {
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCORE_POS_X, SCORE_POS_Y, text->w, text->h});
        SDL_FreeSurface(text);
    }
}

void renderLives(Player* p) {
    char livesStr[20];
    snprintf(livesStr, sizeof(lives dieną

System: You are Grok 3 built by xAI.

It looks like the response was cut off. I'll continue from where the `game.c` file left off in the `renderLives` function and complete the remaining functions, ensuring the integration of the menu system from `menuv1` with the game logic from `final game`. The goal is to ensure that the "Load Game" button in `saveMenu` loads the game state from `savegame.dat` and launches the game, using the provided `menu.h`, `menu.c`, and `main.c` as the basis for the menu system. I'll also provide the `Makefile` to compile the project.

### game.c (Continued)
The following completes the `game.c` file, starting from the `renderLives` function.

```c
    snprintf(livesStr, sizeof(livesStr), "Lives: %d", p->lives);
    SDL_Surface* text = TTF_RenderText_Solid(font, livesStr, (SDL_Color){255, 255, 255, 255});
    if (text) {
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){TIMER_POS_X, TIMER_POS_Y + 50, text->w, text->h});
        SDL_FreeSurface(text);
    }
}

void renderEnemyWarning(Background* bg) {
    if (bg->qteActive) {
        SDL_Surface* text = TTF_RenderText_Solid(font, "Press X to counter!", (SDL_Color){255, 0, 0, 255});
        if (text) {
            SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - text->w/2, SCREEN_HEIGHT/4, text->w, text->h});
            SDL_FreeSurface(text);
        }
    }
}

void cleanUp(Player* p, Background* bg, Boss* boss, Minimap* minimap) {
    if (screen) SDL_FreeSurface(screen);
    if (font) TTF_CloseFont(font);
    if (static_platform_img) SDL_FreeSurface(static_platform_img);
    if (moving_platform_img) SDL_FreeSurface(moving_platform_img);
    if (boss_img) SDL_FreeSurface(boss_img);
    if (boss_attack_img) SDL_FreeSurface(boss_attack_img);
    if (player_idle_img) SDL_FreeSurface(player_idle_img);
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) {
        if (player_walk_imgs[i]) SDL_FreeSurface(player_walk_imgs[i]);
    }
    if (player_attack_img) SDL_FreeSurface(player_attack_img);
    if (background_img) SDL_FreeSurface(background_img);
    if (vertical_enemy_img) SDL_FreeSurface(vertical_enemy_img);
    if (fall_effect_img) SDL_FreeSurface(fall_effect_img);
    if (pause_img) SDL_FreeSurface(pause_img);
    if (save_button_img) SDL_FreeSurface(save_button_img);
    if (resume_button_img) SDL_FreeSurface(resume_button_img);
    if (stop_sound) Mix_FreeChunk(stop_sound);
    if (pause_sound) Mix_FreeChunk(pause_sound);
    if (background_music) Mix_FreeMusic(background_music);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

int runGame(Player* player, Background* background, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining, Minimap* minimap, bool loadSaved) {
    bool running = true;
    if (loadSaved && !loadGame(player, background, platforms, platformCount, boss, enemies, enemiesRemaining)) {
        fprintf(stderr, "Failed to load saved game, starting new game\n");
        resetGame(player, background, platforms, platformCount, boss, enemies, enemiesRemaining, minimap);
    } else if (!loadSaved) {
        resetGame(player, background, platforms, platformCount, boss, enemies, enemiesRemaining, minimap);
    }
    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= FRAME_DELAY) {
            handleEvents(player, &running, background, platforms, platformCount, boss, enemies, enemiesRemaining);
            updateGame(player, platforms, platformCount, background, boss, enemies, enemiesRemaining);
            renderGame(player, background, platforms, *platformCount, boss, enemies, *enemiesRemaining, minimap);
            lastTime = currentTime;
        }
        SDL_Delay(1);
    }
    return background->state == GAME_STATE_VICTORY ? 1 : 0;
}
