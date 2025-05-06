#include "game.h"

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* ground_platform_img = NULL;
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
SDL_Surface* pit_enemy_img = NULL;
SDL_Surface* rock_img = NULL;
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

void loadResources() {
    font = TTF_OpenFont("arial.ttf", 24);
    ground_platform_img = IMG_Load("ground_platform.png");
    static_platform_img = IMG_Load("platform_static.png");
    moving_platform_img = IMG_Load("platform_moving.png");
    boss_img = IMG_Load("boss.png");
    boss_attack_img = IMG_Load("boss_attack.png");
    player_idle_img = IMG_Load("idle.png");
    pit_enemy_img = IMG_Load("pit_enemy.png");
    rock_img = IMG_Load("rock.png");
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
    if (!font || !ground_platform_img || !static_platform_img || !moving_platform_img || !boss_img || !boss_attack_img || 
        !player_idle_img || !player_attack_img || !background_img || !fall_effect_img || !pause_img || 
        !save_button_img || !resume_button_img || !stop_sound || !pause_sound || !background_music || !pit_enemy_img || !rock_img) exit(1);
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) if (!player_walk_imgs[i]) exit(1);
    if (!vertical_enemy_img) vertical_enemy_img = boss_img;
    SDL_SetColorKey(player_idle_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_idle_img->format, 0, 0, 0));
    SDL_SetColorKey(player_attack_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_attack_img->format, 0, 0, 0));
    for (int i = 0; i < WALK_ANIMATION_FRAMES; i++) SDL_SetColorKey(player_walk_imgs[i], SDL_SRCCOLORKEY, SDL_MapRGB(player_walk_imgs[i]->format, 0, 0, 0));
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
    platforms[platformCount++] = (Platform){{0, GROUND_PLATFORM_HEIGHT, worldWidth, PLATFORM_HEIGHT}, ground_platform_img, false, true, false, 0, 0, 0};
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

void initPits(Pit pits[], int* count, int worldWidth) {
    memset(pits, 0, sizeof(Pit) * MAX_PITS);
    int pitCount = 0;
    for (int i = 0; i < MAX_PITS && pitCount < MAX_PITS; i++) {
        int x = 500 + i * 600;
        if (x + PIT_WIDTH < worldWidth) {
            pits[pitCount++] = (Pit){{x, GROUND_PLATFORM_HEIGHT, PIT_WIDTH, PIT_HEIGHT}, NULL, true};
        }
    }
    *count = pitCount;
}

void initPitEnemies(PitEnemy enemies[], Pit pits[], int pitCount) {
    memset(enemies, 0, sizeof(PitEnemy) * MAX_PITS);
    for (int i = 0; i < pitCount; i++) {
        enemies[i] = (PitEnemy){
            {pits[i].rect.x + pits[i].rect.w/2 - 25, pits[i].rect.y + pits[i].rect.h, 50, 50},
            pit_enemy_img,
            PIT_ENEMY_HEALTH,
            true
        };
    }
}

void initRocks(Rock rocks[], int worldWidth) {
    memset(rocks, 0, sizeof(Rock) * MAX_ROCKS);
    for (int i = 0; i < MAX_ROCKS; i++) {
        rocks[i] = (Rock){
            {300 + i * 400, -ROCK_HEIGHT, ROCK_WIDTH, ROCK_HEIGHT},
            rock_img,
            5.0f,
            false,
            0
        };
    }
}

void initBoss(Boss* boss, int worldWidth) {
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

void updatePitEnemies(PitEnemy enemies[], int count, Player* p, Background* bg) {
    for (int i = 0; i < count; i++) {
        if (!enemies[i].isActive) continue;
        SDL_Rect playerWorldPos = {p->pos.x + (int)bg->cameraX, p->pos.y, p->pos.w, p->pos.h};
        if (checkCollision(playerWorldPos, enemies[i].pos)) {
            if (!p->isInvincible) {
                p->health -= PIT_ENEMY_DAMAGE;
                p->isInvincible = true;
                p->lastHitTime = SDL_GetTicks();
            }
        }
    }
}

void updateRocks(Rock rocks[], int count, Player* p, Background* bg) {
    Uint32 currentTime = SDL_GetTicks();
    for (int i = 0; i < count; i++) {
        if (!rocks[i].isActive) {
            if (currentTime - rocks[i].spawnTime > ROCK_SPAWN_INTERVAL) {
                rocks[i].isActive = true;
                rocks[i].spawnTime = currentTime;
            }
            continue;
        }
        rocks[i].pos.y += rocks[i].vy;
        if (rocks[i].pos.y > SCREEN_HEIGHT) {
            rocks[i].isActive = false;
            rocks[i].pos.y = -ROCK_HEIGHT;
            rocks[i].spawnTime = currentTime;
        }
        SDL_Rect playerWorldPos = {p->pos.x + (int)bg->cameraX, p->pos.y, p->pos.w, p->pos.h};
        if (checkCollision(playerWorldPos, rocks[i].pos)) {
            if (!p->isInvincible) {
                p->health -= ROCK_DAMAGE;
                p->isInvincible = true;
                p->lastHitTime = SDL_GetTicks();
            }
            rocks[i].isActive = false;
            rocks[i].pos.y = -ROCK_HEIGHT;
            rocks[i].spawnTime = currentTime;
        }
    }
}

void saveGame(Player* p, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[], int enemiesRemaining) {
    FILE* file = fopen("../menu/savegame.dat", "wb");
    if (!file) {
        fprintf(stderr, "Failed to open ../menu/savegame.dat for writing: %s\n", strerror(errno));
        return;
    }
    fwrite(p, sizeof(Player), 1, file);
    fwrite(bg, sizeof(Background), 1, file);
    fwrite(&platformCount, sizeof(int), 1, file);
    fwrite(platforms, sizeof(Platform), platformCount, file);
    fwrite(boss, sizeof(Boss), 1, file);
    fwrite(enemies, sizeof(Enemy), MAX_ENEMIES, file);
    fwrite(&enemiesRemaining, sizeof(int), 1, file);
    fclose(file);
    printf("Game saved successfully to ../menu/savegame.dat\n");
}

bool loadGame(Player* p, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    FILE* file = fopen("../menu/savegame.dat", "rb");
    if (!file) {
        fprintf(stderr, "Failed to open ../menu/savegame.dat for reading: %s\n", strerror(errno));
        return false;
    }
    
    if (fread(p, sizeof(Player), 1, file) != 1 ||
        fread(bg, sizeof(Background), 1, file) != 1 ||
        fread(platformCount, sizeof(int), 1, file) != 1 ||
        fread(platforms, sizeof(Platform), *platformCount, file) != *platformCount ||
        fread(boss, sizeof(Boss), 1, file) != 1 ||
        fread(enemies, sizeof(Enemy), MAX_ENEMIES, file) != MAX_ENEMIES ||
        fread(enemiesRemaining, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Failed to read save data from ../menu/savegame.dat: %s\n", strerror(errno));
        fclose(file);
        return false;
    }
    
    fclose(file);

    // Reinitialize SDL surface pointers
    p->idle_sprite = player_idle_img;
    memcpy(p->walk_sprites, player_walk_imgs, sizeof(player_walk_imgs));
    p->attack_sprite = player_attack_img;
    bg->img = background_img;
    for (int i = 0; i < *platformCount; i++) {
        if (i == 0) {
            // Ground platform always uses ground_platform_img
            platforms[i].image = ground_platform_img;
        } else {
            // Other platforms use moving or static sprites
            platforms[i].image = platforms[i].isMoving ? moving_platform_img : static_platform_img;
        }
    }
    boss->sprite = boss_img;
    boss->attack_sprite = boss_attack_img;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].sprite = enemies[i].isVertical ? vertical_enemy_img : boss_img;
        enemies[i].attack_sprite = boss_attack_img;
        enemies[i].vertical_sprite = enemies[i].isVertical ? vertical_enemy_img : boss_img;
    }

    printf("Game loaded successfully from ../menu/savegame.dat\n");
    return true;
}
void handleEvents(Player* p, bool* running, Background* bg, Platform platforms[], int* count, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: *running = false; break;
            case SDL_KEYDOWN:
                if (bg->state == GAME_STATE_PLAYING && !bg->timeStopped && !bg->paused) {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT: p->ax = -PLAYER_ACCEL; p->facingRight = false; p->isWalking = true; break;
                        case SDLK_RIGHT: p->ax = PLAYER_ACCEL; p->facingRight = true; p->isWalking = true; break;
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
                        case SDLK_LCTRL: p->isSprinting = true; break;
                        case SDLK_x:
                            if (bg->qteActive && SDL_GetTicks() - bg->qteStartTime <= QTE_DURATION) {
                                bg->qteActive = false;
                                bg->timeStopPrep = 0;
                            }
                            break;
                    }
                } else if (bg->state == GAME_STATE_MENU && event.key.keysym.sym == SDLK_RETURN) {
                    bg->state = GAME_STATE_PLAYING;
                    bg->needsReset = true;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) *running = false;
                if (event.key.keysym.sym == SDLK_r && (bg->state == GAME_STATE_GAME_OVER || bg->state == GAME_STATE_VICTORY)) {
                    bg->needsReset = true;
                    bg->state = GAME_STATE_PLAYING;
                }
                break;
            case SDL_KEYUP:
                if (bg->state == GAME_STATE_PLAYING && !bg->timeStopped && !bg->paused) {
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT: if (p->ax < 0) p->ax = 0; if (p->vx == 0) p->isWalking = false; break;
                        case SDLK_RIGHT: if (p->ax > 0) p->ax = 0; if (p->vx == 0) p->isWalking = false; break;
                        case SDLK_LCTRL: p->isSprinting = false; break;
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
        if (boss->shouldSpawn && fabs((player->pos.x + bg->cameraX) - boss->pos.x) < BOSS_AGGRO_RANGE) boss->isActive = true;
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
        } else if (enemy->isMoving) enemy->vx = enemy->moveDirection * ENEMY_ROAM_SPEED;
    }
    enemy->pos.x += enemy->vx;
    if (enemy->isAttacking && currentTime - enemy->attackStartTime > ENEMY_ATTACK_DURATION) {
        enemy->isAttacking = *enemyIsAttacking = false;
        enemy->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
}

void resetGame(Player* p, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining, Minimap* minimap, Pit pits[], int* pitCount, PitEnemy pitEnemies[], Rock rocks[]) {
    initPlayer(p);
    initMinimap(minimap);
    bg->cameraX = bg->score = 0;
    bg->startTime = SDL_GetTicks();
    bg->needsReset = bg->timeStopped = bg->qteActive = bg->paused = false;
    bg->timeStopStart = bg->timeStopDuration = bg->timeStopPrep = bg->qteStartTime = 0;
    initPlatforms(platforms, platformCount, bg->width);
    initPits(pits, pitCount, bg->width);
    initPitEnemies(pitEnemies, pits, *pitCount);
    initRocks(rocks, bg->width);
    initBoss(boss, bg->width);
    initEnemies(enemies, bg->width);
    *enemiesRemaining = MAX_ENEMIES;
    Mix_PlayMusic(background_music, -1);
}

void updateGame(Player* p, Platform platforms[], int* count, Background* bg, Boss* boss, Enemy enemies[], int* enemiesRemaining, Pit pits[], int pitCount, PitEnemy pitEnemies[], Rock rocks[]) {
    if (bg->needsReset) {
        Minimap minimap;
        Pit pits[MAX_PITS];
        PitEnemy pitEnemies[MAX_PITS];
        Rock rocks[MAX_ROCKS];
        resetGame(p, bg, platforms, count, boss, enemies, enemiesRemaining, &minimap, pits, &pitCount, pitEnemies, rocks);
        return;
    }
    if (bg->state != GAME_STATE_PLAYING || bg->paused) return;
    static bool enemyIsAttacking = false, wasTimeStopped = false;
    int aliveEnemies = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemies[i].isActive && !enemies[i].isDead) aliveEnemies++;
    *enemiesRemaining = aliveEnemies;
    if (*enemiesRemaining <= 0 && !boss->shouldSpawn) boss->shouldSpawn = true;
    if (!p->isJumping && !p->wasInAir) p->fallStartY = p->pos.y;
    if (bg->timeStopped != wasTimeStopped) {
        wasTimeStopped = bg->timeStopped;
        bg->timeStopped ? Mix_PauseMusic() : Mix_ResumeMusic();
    }
    if (!bg->timeStopped || (bg->timeStopped && SDL_GetTicks() - bg->timeStopStart < UNSTOPPED_TIME)) {
        p->vx += p->ax;
        if (p->ax == 0) p->vx *= FRICTION;
        if (fabs(p->vx) < 0.1f) p->vx = 0;
        float maxSpeed = p->isSprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED;
        p->vx = CLAMP(p->vx, -maxSpeed, maxSpeed);
        if (!bg->timeStopped) p->pos.x += p->vx;
        p->vy += GRAVITY * (p->isJumping && p->vy > 0 ? JUMP_PEAK_GRAVITY : JUMP_GRAVITY_SCALE);
        p->pos.y += p->vy;
        p->isJumping = true;
    }
    if (p->pos.x + p->pos.w >= bg->width - LEVEL_END_MARGIN) {
        bg->state = GAME_STATE_VICTORY;
        p->vx = p->ax = 0;
        return;
    }
    SDL_Rect playerWorldPos = {p->pos.x + (int)bg->cameraX, p->pos.y, p->pos.w, p->pos.h};
    for (int j = 0; j < MAX_ENEMIES; j++) {
        if (!enemies[j].isActive || enemies[j].isDead) continue;
        SDL_Rect enemyWorldPos = {enemies[j].pos.x, enemies[j].pos.y, enemies[j].pos.w, enemies[j].pos.h};
        if (checkCollision(playerWorldPos, enemyWorldPos)) {
            int overlapX = MIN((playerWorldPos.x + playerWorldPos.w) - enemyWorldPos.x, (enemyWorldPos.x + enemyWorldPos.w) - playerWorldPos.x);
            int overlapY = MIN((playerWorldPos.y + playerWorldPos.h) - enemyWorldPos.y, (enemyWorldPos.y + enemyWorldPos.h) - playerWorldPos.y);
            if (overlapX < overlapY) {
                p->pos.x = playerWorldPos.x < enemyWorldPos.x ? enemyWorldPos.x - p->pos.w - (int)bg->cameraX : enemyWorldPos.x + enemyWorldPos.w - (int)bg->cameraX;
                p->vx = 0;
            } else {
                p->pos.y = playerWorldPos.y < enemyWorldPos.y ? enemyWorldPos.y - p->pos.h : enemyWorldPos.y + enemyWorldPos.h;
                p->vy = 0;
            }
            if (!p->isInvincible) {
                p->health -= ENEMY_COLLISION_DAMAGE;
                p->isInvincible = true;
                p->lastHitTime = SDL_GetTicks();
            }
        }
        if (enemies[j].isAttacking && checkCollision((SDL_Rect){enemies[j].attackHitbox.x, enemies[j].attackHitbox.y, enemies[j].attackHitbox.w, enemies[j].attackHitbox.h}, playerWorldPos) && !p->isInvincible) {
            p->health -= ENEMY_ATTACK_DAMAGE;
            p->isInvincible = true;
            p->lastHitTime = SDL_GetTicks();
        }
    }
    if (boss->isActive) {
        if (checkCollision(playerWorldPos, boss->pos)) {
            p->pos.x = playerWorldPos.x < boss->pos.x ? boss->pos.x - p->pos.w - (int)bg->cameraX : boss->pos.x + boss->pos.w - (int)bg->cameraX;
            p->vx = 0;
            if (!p->isInvincible) {
                p->health -= BOSS_ATTACK_DAMAGE;
                p->isInvincible = true;
                p->lastHitTime = SDL_GetTicks();
            }
        }
        if (boss->isAttacking && checkCollision((SDL_Rect){boss->attackHitbox.x, boss->attackHitbox.y, boss->attackHitbox.w, boss->attackHitbox.h}, playerWorldPos) && !p->isInvincible) {
            p->health -= BOSS_ATTACK_DAMAGE;
            p->isInvincible = true;
            p->lastHitTime = SDL_GetTicks();
        }
    }
    bool wasOnPlatform = p->isOnMovingPlatform;
    p->isOnMovingPlatform = false;
    for (int j = 0; j < *count; j++) {
        if (!platforms[j].isActive) continue;
        SDL_Rect platformRect = platforms[j].rect;
        if (checkCollision(playerWorldPos, platformRect)) {
            if (p->vy >= 0 && playerWorldPos.y + playerWorldPos.h >= platformRect.y && playerWorldPos.y + playerWorldPos.h <= platformRect.y + PLATFORM_COLLISION_MARGIN) {
                if (!p->isOnMovingPlatform && p->wasInAir) {
                    float fallDistance = p->pos.y - p->fallStartY;
                    if (fallDistance > FALL_DAMAGE_THRESHOLD) {
                        p->health -= (int)((fallDistance - FALL_DAMAGE_THRESHOLD) / FALL_DAMAGE_THRESHOLD * FALL_DAMAGE_MULTIPLIER);
                        p->fallDamageTime = SDL_GetTicks();
                        if (p->health <= 0) {
                            p->lives--;
                            if (p->lives <= 0) bg->state = GAME_STATE_GAME_OVER;
                            else {
                                p->health = PLAYER_HEALTH;
                                p->pos.y = GROUND_PLATFORM_HEIGHT - p->pos.h;
                                p->vy = 0;
                                p->isJumping = false;
                            }
                        }
                    }
                }
                p->pos.y = platformRect.y - p->pos.h;
                p->vy = 0;
                p->isJumping = false;
                p->lastGroundY = p->pos.y;
                p->wasInAir = false;
                if (platforms[j].isMoving) {
                    p->isOnMovingPlatform = true;
                    p->movingPlatformId = j;
                    if (wasOnPlatform && p->movingPlatformId == j) p->pos.x += MOVING_PLATFORM_SPEED * platforms[j].moveDirection;
                }
            } else if (playerWorldPos.y + playerWorldPos.h > platformRect.y + PLATFORM_COLLISION_MARGIN) {
                p->pos.x = playerWorldPos.x < platformRect.x ? platformRect.x - p->pos.w : platformRect.x + platformRect.w;
                p->vx = 0;
            }
        }
    }
    for (int i = 0; i < pitCount; i++) {
        if (!pits[i].isActive) continue;
        if (checkCollision(playerWorldPos, pits[i].rect)) {
            if (!p->isInvincible) {
                p->health -= PIT_ENEMY_DAMAGE;
                p->isInvincible = true;
                p->lastHitTime = SDL_GetTicks();
            }
        }
    }
    updatePitEnemies(pitEnemies, pitCount, p, bg);
    updateRocks(rocks, MAX_ROCKS, p, bg);
    if (p->pos.y > SCREEN_HEIGHT || p->health <= 0) {
        p->lives--;
        if (p->lives <= 0) bg->state = GAME_STATE_GAME_OVER;
        else {
            p->health = PLAYER_HEALTH;
            p->pos.y = GROUND_PLATFORM_HEIGHT - p->pos.h;
            p->vy = 0;
            p->isJumping = false;
            p->wasInAir = false;
        }
        bg->score = MAX(0, bg->score - 5);
    }
    if (p->isAttacking && SDL_GetTicks() - p->attackStartTime > PLAYER_ATTACK_DURATION) {
        p->isAttacking = false;
        p->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
    if (p->isInvincible && SDL_GetTicks() - p->lastHitTime > INVINCIBILITY_DURATION) p->isInvincible = false;
    if (p->isWalking && SDL_GetTicks() - p->last_frame_time > WALK_ANIMATION_DELAY) {
        p->current_walk_frame = (p->current_walk_frame + 1) % WALK_ANIMATION_FRAMES;
        p->last_frame_time = SDL_GetTicks();
    }
    int playerWorldX = p->pos.x + bg->cameraX;
    bg->cameraX = CLAMP(playerWorldX - SCREEN_WIDTH / 2, 0, bg->width - SCREEN_WIDTH);
    int playerY = p->pos.y;
    bg->targetCameraY = playerY < SCREEN_HEIGHT / 3 ? SCREEN_HEIGHT / 3 - playerY : 0;
    bg->cameraY += (bg->targetCameraY - bg->cameraY) * 0.1f;
    bg->cameraY = CLAMP(bg->cameraY, -MAX_CAMERA_SCROLL, 0);
    p->pos.x = playerWorldX - bg->cameraX;
    for (int i = 0; i < *count; i++) {
        if (platforms[i].isMoving && platforms[i].isActive && !bg->timeStopped) {
            platforms[i].rect.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
            if (abs(platforms[i].rect.x - platforms[i].originalX) > platforms[i].moveDistance) platforms[i].moveDirection *= -1;
        }
    }
    updateBossAI(boss, p, bg);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        updateEnemyAI(&enemies[i], p, bg, &enemyIsAttacking);
        if (enemies[i].pos.y > SCREEN_HEIGHT + 100) enemies[i].isDead = true;
    }
    if (p->isAttacking) {
        SDL_Rect attackWorldPos = {p->attackHitbox.x + (int)bg->cameraX, p->attackHitbox.y, p->attackHitbox.w, p->attackHitbox.h};
        if (checkCollision(attackWorldPos, boss->pos)) {
            boss->health -= PLAYER_ATTACK_DAMAGE;
            if (boss->health <= 0) {
                bg->score += BOSS_KILL_SCORE;
                bg->state = GAME_STATE_VICTORY;
            }
        }
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].isActive && !enemies[j].isDead && checkCollision(attackWorldPos, enemies[j].pos)) {
                enemies[j].health -= PLAYER_ATTACK_DAMAGE;
                if (enemies[j].health <= 0) {
                    enemies[j].isDead = true;
                    bg->score += ENEMY_KILL_SCORE;
                }
            }
        }
    }
    if (checkCollision(playerWorldPos, boss->pos) && p->vy > 0) {
        boss->health -= PLAYER_ATTACK_DAMAGE * 2;
        p->vy = JUMP_FORCE / 2;
        if (boss->health <= 0) {
            bg->score += BOSS_KILL_SCORE;
            bg->state = GAME_STATE_VICTORY;
        }
    }
}

void renderEnemyWarning(Background* bg) {
    if (!font || !screen || bg->state != GAME_STATE_PLAYING) return;
    SDL_Surface* warningSurface = TTF_RenderText_Solid(font, "Enemy Nearby!", (SDL_Color){255, 0, 0, 255});
    if (warningSurface) {
        SDL_BlitSurface(warningSurface, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - warningSurface->w/2, 50, warningSurface->w, warningSurface->h});
        SDL_FreeSurface(warningSurface);
    }
}

void renderTimer(Background* bg) {
    if (!font || !screen || bg->state == GAME_STATE_MENU) return;
    Uint32 elapsedTime = ((bg->timeStopped ? bg->timeStopStart : SDL_GetTicks()) - bg->startTime) / 1000;
    char timeText[20];
    snprintf(timeText, sizeof(timeText), "%02d:%02d", elapsedTime / 60, elapsedTime % 60);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, timeText, (SDL_Color){255, 255, 255, 255});
    if (textSurface) {
        SDL_BlitSurface(textSurface, NULL, screen, &(SDL_Rect){TIMER_POS_X, TIMER_POS_Y, textSurface->w, textSurface->h});
        SDL_FreeSurface(textSurface);
    }
}

void renderScore(Background* bg) {
    if (!font || !screen || bg->state == GAME_STATE_MENU) return;
    char scoreText[50];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", bg->score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, (SDL_Color){255, 255, 255, 255});
    if (textSurface) {
        SDL_BlitSurface(textSurface, NULL, screen, &(SDL_Rect){SCORE_POS_X - 342, SCORE_POS_Y + 100, textSurface->w, textSurface->h});
        SDL_FreeSurface(textSurface);
    }
}

void renderLives(Player* p) {
    if (!font || !screen) return;
    char livesText[20];
    snprintf(livesText, sizeof(livesText), "Lives: %d", p->lives);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, livesText, (SDL_Color){0, 255, 0, 255});
    if (textSurface) {
        SDL_BlitSurface(textSurface, NULL, screen, &(SDL_Rect){10, 70, textSurface->w, textSurface->h});
        SDL_FreeSurface(textSurface);
    }
}

void renderHealthBars(Player* p, Boss* boss, Background* bg) {
    if (!screen || bg->state == GAME_STATE_MENU) return;
    SDL_FillRect(screen, &(SDL_Rect){10, 40, 100, 10}, SDL_MapRGB(screen->format, 255, 0, 0));
    SDL_FillRect(screen, &(SDL_Rect){10, 40, (int)(100 * ((float)p->health / PLAYER_HEALTH)), 10}, SDL_MapRGB(screen->format, 0, 255, 0));
    if (boss->isActive) {
        SDL_Rect bossScreenPos = {boss->pos.x - (int)bg->cameraX, boss->pos.y, boss->pos.w, boss->pos.h};
        if (bossScreenPos.x + boss->pos.w > 0 && bossScreenPos.x < SCREEN_WIDTH) {
            SDL_FillRect(screen, &(SDL_Rect){bossScreenPos.x, bossScreenPos.y - 10, BOSS_WIDTH, 5}, SDL_MapRGB(screen->format, 255, 0, 0));
            SDL_FillRect(screen, &(SDL_Rect){bossScreenPos.x, bossScreenPos.y - 10, (int)(BOSS_WIDTH * ((float)boss->health / BOSS_HEALTH)), 5}, SDL_MapRGB(screen->format, 255, 255, 0));
        }
    }
}

void renderEnemyCounter(int enemiesRemaining) {
    if (!font || !screen) return;
    char enemyText[50];
    snprintf(enemyText, sizeof(enemyText), enemiesRemaining > 0 ? "Enemies Left: %d" : "Head to the boss!", enemiesRemaining);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, enemyText, (SDL_Color){255, 255, 255, 255});
    if (textSurface) {
        SDL_BlitSurface(textSurface, NULL, screen, &(SDL_Rect){SCREEN_WIDTH - textSurface->w - 10, 10, textSurface->w, textSurface->h});
        SDL_FreeSurface(textSurface);
    }
}

void renderPauseButton(Background* bg) {
    if (!pause_img || !screen || bg->state != GAME_STATE_PLAYING) return;
    SDL_BlitSurface(pause_img, NULL, screen, &(SDL_Rect){SCREEN_WIDTH - pause_img->w - 10, 40, pause_img->w, pause_img->h});
    if (bg->paused) {
        SDL_BlitSurface(save_button_img, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - save_button_img->w/2, SCREEN_HEIGHT/2 - save_button_img->h - 10, save_button_img->w, save_button_img->h});
        SDL_BlitSurface(resume_button_img, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - resume_button_img->w/2, SCREEN_HEIGHT/2 + 10, resume_button_img->w, resume_button_img->h});
    }
}

void renderEndMessage(Background* bg) {
    if (!font || !screen || (bg->state != GAME_STATE_GAME_OVER && bg->state != GAME_STATE_VICTORY)) return;
    char message[100];
    snprintf(message, sizeof(message), bg->state == GAME_STATE_VICTORY ? "VICTORY! Score: %d\nPress R to restart" : "GAME OVER! Score: %d\nPress R to restart", bg->score);
    char* line1 = strtok(message, "\n"), *line2 = strtok(NULL, "\n");
    if (line1) {
        SDL_Surface* textSurface1 = TTF_RenderText_Solid(font, line1, bg->state == GAME_STATE_VICTORY ? (SDL_Color){0, 255, 0, 255} : (SDL_Color){255, 0, 0, 255});
        if (textSurface1) {
            SDL_BlitSurface(textSurface1, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - textSurface1->w/2, SCREEN_HEIGHT/2 - textSurface1->h - 10, textSurface1->w, textSurface1->h});
            SDL_FreeSurface(textSurface1);
        }
    }
    if (line2) {
        SDL_Surface* textSurface2 = TTF_RenderText_Solid(font, line2, (SDL_Color){255, 255, 255, 255});
        if (textSurface2) {
            SDL_BlitSurface(textSurface2, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - textSurface2->w/2, SCREEN_HEIGHT/2 + 10, textSurface2->w, textSurface2->h});
            SDL_FreeSurface(textSurface2);
        }
    }
}

void renderMenu(Background* bg) {
    if (!font || !screen) return;
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    TTF_Font* titleFont = TTF_OpenFont("arial.ttf", 48);
    if (titleFont) {
        SDL_Surface* titleSurface = TTF_RenderText_Solid(titleFont, "PLATFORM ADVENTURE", (SDL_Color){255, 255, 0, 255});
        if (titleSurface) {
            SDL_BlitSurface(titleSurface, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - titleSurface->w/2, SCREEN_HEIGHT/3 - titleSurface->h/2, titleSurface->w, titleSurface->h});
            SDL_FreeSurface(titleSurface);
        }
        TTF_CloseFont(titleFont);
    }
    SDL_Surface* instruction1 = TTF_RenderText_Solid(font, "Arrow Keys to Move, Space to Attack", (SDL_Color){255, 255, 255, 255});
    SDL_Surface* instruction2 = TTF_RenderText_Solid(font, "Press Enter to Start", (SDL_Color){255, 255, 0, 255});
    if (instruction1 && instruction2) {
        SDL_BlitSurface(instruction1, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - instruction1->w/2, SCREEN_HEIGHT/2, instruction1->w, instruction1->h});
        SDL_BlitSurface(instruction2, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - instruction2->w/2, SCREEN_HEIGHT/2 + 40, instruction2->w, instruction2->h});
        SDL_FreeSurface(instruction1);
        SDL_FreeSurface(instruction2);
    }
}

void renderMinimap(Minimap* minimap, Player* p, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[]) {
    if (!minimap->enabled || !screen || !bg->img || bg->state != GAME_STATE_PLAYING) return;
    minimap->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, minimap->width, minimap->height, screen->format->BitsPerPixel, 0, 0, 0, 0);
    minimap->frame = SDL_CreateRGBSurface(SDL_SWSURFACE, minimap->width + 12, minimap->height + 12, screen->format->BitsPerPixel, 0, 0, 0, 0);
    if (!minimap->surface || !minimap->frame) return;
    SDL_FillRect(minimap->frame, NULL, SDL_MapRGB(screen->format, minimap->frameColor.r, minimap->frameColor.g, minimap->frameColor.b));
    SDL_FillRect(minimap->frame, &(SDL_Rect){2, 2, minimap->width + 8, minimap->height + 8}, SDL_MapRGB(screen->format, 255, 50, 50));
    SDL_FillRect(minimap->frame, &(SDL_Rect){6, 6, minimap->width, minimap->height}, SDL_MapRGB(minimap->frame->format, minimap->backgroundColor.r, minimap->backgroundColor.g, minimap->backgroundColor.b));
    SDL_Surface* minimapBg = SDL_CreateRGBSurface(SDL_SWSURFACE, minimap->width, minimap->height, screen->format->BitsPerPixel, 0, 0, 0, 0);
    if (minimapBg) {
        SDL_BlitSurface(bg->img, &(SDL_Rect){(int)bg->cameraX, 0, minimap->width / minimap->scale, minimap->height / minimap->scale}, 
                        minimapBg, &(SDL_Rect){0, 0, minimap->width, minimap->height});
        SDL_BlitSurface(minimapBg, NULL, minimap->surface, NULL);
        SDL_FreeSurface(minimapBg);
    }
    for (int i = 0; i < platformCount; i++) {
        if (!platforms[i].isActive) continue;
        SDL_FillRect(minimap->surface, &(SDL_Rect){(int)(platforms[i].rect.x * minimap->scale) - (int)(bg->cameraX * minimap->scale), 
                                                  (int)(platforms[i].rect.y * minimap->scale), 
                                                  (int)(platforms[i].rect.w * minimap->scale), 
                                                  (int)(platforms[i].rect.h * minimap->scale)}, 
                     platforms[i].isMoving ? SDL_MapRGB(minimap->surface->format, minimap->movingPlatformColor.r, minimap->movingPlatformColor.g, minimap->movingPlatformColor.b) : 
                                            SDL_MapRGB(minimap->surface->format, minimap->platformColor.r, minimap->platformColor.g, minimap->platformColor.b));
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        SDL_FillRect(minimap->surface, &(SDL_Rect){(int)(enemies[i].pos.x * minimap->scale) - (int)(bg->cameraX * minimap->scale), 
                                                  (int)(enemies[i].pos.y * minimap->scale) + 5, 6, 6}, 
                     SDL_MapRGB(minimap->surface->format, minimap->enemyColor.r, minimap->enemyColor.g, minimap->enemyColor.b));
    }
    if (boss->isActive || boss->shouldSpawn) {
        SDL_FillRect(minimap->surface, &(SDL_Rect){(int)(boss->pos.x * minimap->scale) - (int)(bg->cameraX * minimap->scale), 
                                                  (int)(boss->pos.y * minimap->scale) + 5, 6, 6}, 
                     SDL_MapRGB(minimap->surface->format, minimap->bossColor.r, minimap->bossColor.g, minimap->bossColor.b));
    }
    SDL_FillRect(minimap->surface, &(SDL_Rect){(int)(p->pos.x * minimap->scale), (int)(p->pos.y * minimap->scale) + 5, 6, 6}, 
                 SDL_MapRGB(minimap->surface->format, minimap->playerColor.r, minimap->playerColor.g, minimap->playerColor.b));
    SDL_BlitSurface(minimap->surface, NULL, minimap->frame, &(SDL_Rect){6, 6, minimap->width, minimap->height});
    SDL_BlitSurface(minimap->frame, NULL, screen, &minimap->position);
    SDL_FreeSurface(minimap->surface);
    SDL_FreeSurface(minimap->frame);
}

void renderGame(Player* p, Background* bg, Platform platforms[], int count, Boss* boss, Enemy enemies[], int enemiesRemaining, Minimap* minimap, Pit pits[], int pitCount, PitEnemy pitEnemies[], Rock rocks[]) {
    if (!screen) return;
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 50, 50, 100));
    if (bg->state == GAME_STATE_MENU) {
        renderMenu(bg);
        SDL_Flip(screen);
        return;
    }
    if (!bg->img) return;
    SDL_Rect src_rect = {(int)bg->cameraX, (int)-bg->cameraY, MIN(SCREEN_WIDTH, bg->width - (int)bg->cameraX), SCREEN_HEIGHT};
    SDL_Rect dest_rect = {0, 0, src_rect.w, SCREEN_HEIGHT};
    SDL_Surface* stretched_bg = SDL_CreateRGBSurface(SDL_SWSURFACE, src_rect.w, SCREEN_HEIGHT, screen->format->BitsPerPixel, 0, 0, 0, 0);
    if (stretched_bg) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            SDL_BlitSurface(bg->img, &(SDL_Rect){src_rect.x, (y * bg->img->h) / SCREEN_HEIGHT, src_rect.w, 1}, 
                            stretched_bg, &(SDL_Rect){0, y, src_rect.w, 1});
        }
        SDL_BlitSurface(stretched_bg, NULL, screen, &dest_rect);
        SDL_FreeSurface(stretched_bg);
    }
    if (src_rect.w < SCREEN_WIDTH) SDL_FillRect(screen, &(SDL_Rect){src_rect.w, 0, SCREEN_WIDTH - src_rect.w, SCREEN_HEIGHT}, SDL_MapRGB(screen->format, 50, 50, 100));
    for (int i = 0; i < count; i++) {
        if (!platforms[i].isActive || !platforms[i].image) continue;
        int platformScreenX = platforms[i].rect.x - (int)bg->cameraX;
        if (platformScreenX + platforms[i].rect.w > 0 && platformScreenX < SCREEN_WIDTH) {
            SDL_Rect r = {platformScreenX, platforms[i].rect.y - (int)bg->cameraY, platforms[i].rect.w, platforms[i].rect.h};
            int tiles = (r.w + platforms[i].image->w - 1) / platforms[i].image->w;
            for (int t = 0; t < tiles; t++) {
                SDL_BlitSurface(platforms[i].image, NULL, screen, &(SDL_Rect){r.x + t * platforms[i].image->w, r.y, 
                                        MIN(platforms[i].image->w, r.w - t * platforms[i].image->w), r.h});
            }
        }
    }
    for (int i = 0; i < pitCount; i++) {
        if (!pits[i].isActive) continue;
        SDL_Rect pitScreenPos = {pits[i].rect.x - (int)bg->cameraX, pits[i].rect.y - (int)bg->cameraY, pits[i].rect.w, pits[i].rect.h};
        if (pitScreenPos.x + pits[i].rect.w > 0 && pitScreenPos.x < SCREEN_WIDTH) {
            SDL_FillRect(screen, &pitScreenPos, SDL_MapRGB(screen->format, 0, 0, 0));
        }
    }
    for (int i = 0; i < pitCount; i++) {
        if (!pitEnemies[i].isActive) continue;
        SDL_Rect enemyScreenPos = {pitEnemies[i].pos.x - (int)bg->cameraX, pitEnemies[i].pos.y - (int)bg->cameraY, pitEnemies[i].pos.w, pitEnemies[i].pos.h};
        if (enemyScreenPos.x + pitEnemies[i].pos.w > 0 && enemyScreenPos.x < SCREEN_WIDTH) {
            SDL_BlitSurface(pitEnemies[i].sprite, NULL, screen, &enemyScreenPos);
        }
    }
    for (int i = 0; i < MAX_ROCKS; i++) {
        if (!rocks[i].isActive) continue;
        SDL_Rect rockScreenPos = {rocks[i].pos.x - (int)bg->cameraX, rocks[i].pos.y - (int)bg->cameraY, rocks[i].pos.w, rocks[i].pos.h};
        if (rockScreenPos.x + rocks[i].pos.w > 0 && rockScreenPos.x < SCREEN_WIDTH) {
            SDL_BlitSurface(rocks[i].sprite, NULL, screen, &rockScreenPos);
        }
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        SDL_Rect enemyScreenPos = {enemies[i].pos.x - (int)bg->cameraX, enemies[i].pos.y, enemies[i].pos.w, enemies[i].pos.h};
        if (enemyScreenPos.x + enemies[i].pos.w > 0 && enemyScreenPos.x < SCREEN_WIDTH) {
            SDL_Surface* enemySprite = enemies[i].isVertical ? enemies[i].vertical_sprite : enemies[i].sprite;
            SDL_BlitSurface(enemySprite, NULL, screen, &enemyScreenPos);
            if (enemies[i].isAttacking) {
                SDL_Surface* attackSprite = enemies[i].facingLeft ? flipSurface(enemies[i].attack_sprite) : enemies[i].attack_sprite;
                if (attackSprite) {
                    SDL_BlitSurface(attackSprite, NULL, screen, &(SDL_Rect){enemies[i].attackHitbox.x - (int)bg->cameraX - 120, 
                                    enemies[i].attackHitbox.y - 100, enemies[i].attackHitbox.w, enemies[i].attackHitbox.h});
                    if (enemies[i].facingLeft) SDL_FreeSurface(attackSprite);
                }
            }
            SDL_FillRect(screen, &(SDL_Rect){enemyScreenPos.x, enemyScreenPos.y - 10, enemies[i].pos.w, 3}, SDL_MapRGB(screen->format, 255, 0, 0));
            SDL_FillRect(screen, &(SDL_Rect){enemyScreenPos.x, enemyScreenPos.y - 10, (int)(enemies[i].pos.w * ((float)enemies[i].health / ENEMY_HEALTH)), 3}, 
                         SDL_MapRGB(screen->format, 0, 255, 0));
        }
    }
    if (bg->qteActive && SDL_GetTicks() - bg->qteStartTime <= QTE_DURATION) {
        SDL_Surface* qteSurface = TTF_RenderText_Solid(font, "Press X to Negate", (SDL_Color){255, 255, 255, 255});
        if (qteSurface) {
            SDL_BlitSurface(qteSurface, NULL, screen, &(SDL_Rect){SCREEN_WIDTH/2 - qteSurface->w/2, SCREEN_HEIGHT/2 - qteSurface->h - 50, qteSurface->w, qteSurface->h});
            SDL_FreeSurface(qteSurface);
        }
    }
    if (boss->shouldSpawn) {
        SDL_Rect bossScreenPos = {boss->pos.x - (int)bg->cameraX, boss->pos.y, boss->pos.w, boss->pos.h};
        if (bossScreenPos.x + boss->pos.w > 0 && bossScreenPos.x < SCREEN_WIDTH) {
            SDL_BlitSurface(boss->sprite, NULL, screen, &bossScreenPos);
            if (boss->isAttacking) {
                SDL_Surface* attackSprite = boss->facingLeft ? flipSurface(boss->attack_sprite) : boss->attack_sprite;
                if (attackSprite) {
                    SDL_BlitSurface(attackSprite, NULL, screen, &(SDL_Rect){boss->attackHitbox.x - (int)bg->cameraX - 120, 
                                    boss->attackHitbox.y - 100, boss->attackHitbox.w, boss->attackHitbox.h});
                    if (boss->facingLeft) SDL_FreeSurface(attackSprite);
                }
            }
        }
    }
    if (!p->isInvincible || (SDL_GetTicks() / 100) % 2 == 0) {
        SDL_Surface* currentSprite = p->isWalking && p->vx != 0 ? p->walk_sprites[p->current_walk_frame] : p->idle_sprite;
        SDL_Surface* renderSprite = p->facingRight ? currentSprite : flipSurface(currentSprite);
        if (renderSprite) {
            SDL_BlitSurface(renderSprite, NULL, screen, &p->pos);
            if (!p->facingRight) SDL_FreeSurface(renderSprite);
        }
    }
    if (p->isAttacking && p->attack_sprite) {
        SDL_Surface* attackSprite = p->facingRight ? p->attack_sprite : flipSurface(p->attack_sprite);
        if (attackSprite) {
            SDL_BlitSurface(attackSprite, NULL, screen, &(SDL_Rect){p->pos.x + (p->facingRight ? p->pos.w : -p->attack_sprite->w), 
                            p->pos.y + (p->pos.h/2 - p->attack_sprite->h/2), p->attack_sprite->w, p->attack_sprite->h});
            if (!p->facingRight) SDL_FreeSurface(attackSprite);
        }
    }
    if (SDL_GetTicks() - p->fallDamageTime < FALL_DAMAGE_EFFECT_DURATION) SDL_BlitSurface(fall_effect_img, NULL, screen, &(SDL_Rect){0, -210, 800, 600});
    renderTimer(bg);
    renderScore(bg);
    renderHealthBars(p, boss, bg);
    renderLives(p);
    renderEnemyCounter(enemiesRemaining);
    renderPauseButton(bg);
    renderEndMessage(bg);
    renderMinimap(minimap, p, bg, platforms, count, boss, enemies);
    if (bg->timeStopped) {
        SDL_Surface* effectSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, screen->format->BitsPerPixel, 0, 0, 0, 0);
        if (effectSurface) {
            SDL_BlitSurface(screen, NULL, effectSurface, NULL);
            if (SDL_MUSTLOCK(effectSurface)) SDL_LockSurface(effectSurface);
            Uint8* pixels = (Uint8*)effectSurface->pixels;
            int pitch = effectSurface->pitch, bytesPerPixel = effectSurface->format->BytesPerPixel;
            float flicker = 0.8f + 0.2f * sinf((SDL_GetTicks() - bg->timeStopStart) / 200.0f);
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    Uint32 pixel = *(Uint32*)(pixels + y * pitch + x * bytesPerPixel);
                    Uint8 r, g, b;
                    SDL_GetRGB(pixel, effectSurface->format, &r, &g, &b);
                    r = (Uint8)(r * 0.5 + (128 * flicker) * 0.5);
                    g = (Uint8)(g * 0.5 + (128 * flicker) * 0.5);
                    b = (Uint8)(b * 0.5 + (128 * flicker) * 0.5);
                    *(Uint32*)(pixels + y * pitch + x * bytesPerPixel) = SDL_MapRGB(effectSurface->format, r, g, b);
                }
            }
            if (SDL_MUSTLOCK(effectSurface)) SDL_UnlockSurface(effectSurface);
            SDL_BlitSurface(effectSurface, NULL, screen, NULL);
            SDL_FreeSurface(effectSurface);
        }
    }
    SDL_Flip(screen);
}

void cleanUp(Player* p, Background* bg, Boss* boss, Minimap* minimap) {
    SDL_FreeSurface(ground_platform_img);
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
    SDL_FreeSurface(pit_enemy_img);
    SDL_FreeSurface(rock_img);
    Mix_FreeChunk(stop_sound);
    Mix_FreeChunk(pause_sound);
    Mix_FreeMusic(background_music);
    TTF_CloseFont(font);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}
