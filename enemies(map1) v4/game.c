#include "game.h"

// Global variables
SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* static_platform_img = NULL;
SDL_Surface* moving_platform_img = NULL;
SDL_Surface* player_img = NULL;
SDL_Surface* player2_img = NULL;
SDL_Surface* player_attack_img = NULL;
SDL_Surface* background_img = NULL;
Mix_Music* bgMusic = NULL;

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

bool checkPixelPerfectCollision(SDL_Surface* src, SDL_Rect src_rect, SDL_Surface* dst, SDL_Rect dst_rect) {
    if (!src || !dst) return false;
    if (!checkCollision(src_rect, dst_rect)) return false;

    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);

    bool collision = false;

    // Calculate overlapping area
    int startX = MAX(src_rect.x, dst_rect.x);
    int endX = MIN(src_rect.x + src_rect.w, dst_rect.x + dst_rect.w);
    int startY = MAX(src_rect.y, dst_rect.y);
    int endY = MIN(src_rect.y + src_rect.h, dst_rect.y + dst_rect.h);

    for (int y = startY; y < endY && !collision; y++) {
        for (int x = startX; x < endX && !collision; x++) {
            Uint32 src_pixel = ((Uint32*)src->pixels)[(y - src_rect.y) * src->w + (x - src_rect.x)];
            Uint32 dst_pixel = ((Uint32*)dst->pixels)[(y - dst_rect.y) * dst->w + (x - dst_rect.x)];
            if ((src_pixel >> 24) && (dst_pixel >> 24)) {
                collision = true;
            }
        }
    }

    if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);

    return collision;
}

SDL_Surface* flipSurface(SDL_Surface* surface) {
    if (!surface) return NULL;

    SDL_Surface* flipped = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h,
                                              surface->format->BitsPerPixel,
                                              surface->format->Rmask,
                                              surface->format->Gmask,
                                              surface->format->Bmask,
                                              surface->format->Amask);
    if (!flipped) return NULL;

    Uint8* pixels = (Uint8*)surface->pixels;
    Uint8* flipped_pixels = (Uint8*)flipped->pixels;
    int pitch = surface->pitch;
    int bytesPerPixel = surface->format->BytesPerPixel;

    for(int y = 0; y < surface->h; y++) {
        for(int x = 0; x < surface->w; x++) {
            for(int b = 0; b < bytesPerPixel; b++) {
                flipped_pixels[y * pitch + x * bytesPerPixel + b] = 
                    pixels[y * pitch + (surface->w - 1 - x) * bytesPerPixel + b];
            }
        }
    }

    return flipped;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        exit(1);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if (!screen) {
        fprintf(stderr, "Failed to set video mode: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    SDL_WM_SetCaption("Enhanced Platform Game", NULL);
    srand((unsigned int)time(NULL));
}

void loadResources() {
    font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        exit(1);
    }

    static_platform_img = IMG_Load("platform_static.png");
    moving_platform_img = IMG_Load("platform_moving.png");
    player_img = IMG_Load("player.png");
    player2_img = IMG_Load("player2.png");
    player_attack_img = IMG_Load("player_attack_slash.png");
    background_img = IMG_Load("background.png");

    if (!static_platform_img || !moving_platform_img || !player_img || 
        !player2_img || !player_attack_img || !background_img) {
        fprintf(stderr, "Failed to load game images! SDL_Error: %s\n", IMG_GetError());
        exit(1);
    }

    // Set color key for transparency if needed
    SDL_SetColorKey(player_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_img->format, 0, 0, 0));
    SDL_SetColorKey(player2_img, SDL_SRCCOLORKEY, SDL_MapRGB(player2_img->format, 0, 0, 0));
    SDL_SetColorKey(player_attack_img, SDL_SRCCOLORKEY, SDL_MapRGB(player_attack_img->format, 0, 0, 0));

    // Load music
    bgMusic = Mix_LoadMUS("m1ost.mp3");
    if (!bgMusic) {
        fprintf(stderr, "Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        Mix_PlayMusic(bgMusic, -1); // -1 for infinite loop
    }
}

void initPlayer(Player* p, bool isPlayer2) {
    if ((!player_img && !isPlayer2) || (!player2_img && isPlayer2) || !player_attack_img) {
        fprintf(stderr, "Player images not loaded!\n");
        exit(1);
    }

    memset(p, 0, sizeof(Player));
    p->sprite = isPlayer2 ? player2_img : player_img;
    p->attack_sprite = player_attack_img;
    p->pos.w = isPlayer2 ? player2_img->w : player_img->w;
    p->pos.h = isPlayer2 ? player2_img->h : player_img->h;
    p->pos.x = PLAYER_X_POSITION + (isPlayer2 ? 50 : 0);
    p->pos.y = GROUND_PLATFORM_HEIGHT - p->pos.h;
    p->health = PLAYER_HEALTH;
    p->facingRight = true;
    p->lastGroundY = p->pos.y;
    p->isPlayer2 = isPlayer2;
}

void initBackground(Background* bg) {
    memset(bg, 0, sizeof(Background));
    bg->img = background_img;
    bg->width = bg->img ? bg->img->w : SCREEN_WIDTH * 3; // Default width if no image
    bg->startTime = SDL_GetTicks();
    bg->state = GAME_STATE_MENU;
}

void initPlatforms(Platform platforms[], int* count, int worldWidth) {
    memset(platforms, 0, sizeof(Platform) * MAX_PLATFORMS);

    int platformCount = 0;
    platforms[platformCount++] = (Platform){
        {0, GROUND_PLATFORM_HEIGHT, worldWidth, PLATFORM_HEIGHT},
        static_platform_img, false, true, false, 0, 0, 0
    };

    int currentX = 0;
    while (platformCount < MAX_PLATFORMS && currentX < worldWidth - END_MARGIN) {
        int spacing = MIN_PLATFORM_SPACING + rand() % (MAX_PLATFORM_SPACING - MIN_PLATFORM_SPACING);
        int y = 150 + rand() % 350;
        bool moving = (rand() % 100) < MOVING_PLATFORM_CHANCE;
        
        platforms[platformCount] = (Platform){
            {currentX, y, PLATFORM_WIDTH, PLATFORM_HEIGHT},
            moving ? moving_platform_img : static_platform_img,
            false, true, moving,
            moving ? (rand() % 2 ? 1 : -1) : 0,
            moving ? (100 + rand() % 200) : 0,
            currentX
        };
        platformCount++;
        currentX += PLATFORM_WIDTH + spacing;
    }
    *count = platformCount;
}

void initBoss(Boss* boss, int worldWidth) {
    memset(boss, 0, sizeof(Boss));
    
    boss->spriteSheet = IMG_Load("anim.png");
    if (!boss->spriteSheet) {
        fprintf(stderr, "Failed to load boss sprite sheet!\n");
        exit(1);
    }
    
    // Initialize animation frames
    int frameWidth = boss->spriteSheet->w / ANIMATION_FRAMES;
    int frameHeight = boss->spriteSheet->h / 5; // 5 rows: Attack, Death, Hurt, Idle, Walk
    
    // Attack frames (top row)
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        boss->attackFrames[i] = (SDL_Rect){i * frameWidth, 0, frameWidth, frameHeight};
    }
    
    // Death frames (second row)
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        boss->deathFrames[i] = (SDL_Rect){i * frameWidth, frameHeight, frameWidth, frameHeight};
    }
    
    // Hurt frames (third row)
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        boss->hurtFrames[i] = (SDL_Rect){i * frameWidth, frameHeight * 2, frameWidth, frameHeight};
    }
    
    // Idle frames (fourth row)
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        boss->idleFrames[i] = (SDL_Rect){i * frameWidth, frameHeight * 3, frameWidth, frameHeight};
    }
    
    // Walk frames (fifth row)
    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        boss->walkFrames[i] = (SDL_Rect){i * frameWidth, frameHeight * 4, frameWidth, frameHeight};
    }
    
    boss->currentFrame = boss->idleFrames[0];
    boss->frameTime = SDL_GetTicks();
    boss->currentAnimFrame = 0;
    
    boss->pos = (SDL_Rect){
        worldWidth - BOSS_X_OFFSET - frameWidth,
        GROUND_PLATFORM_HEIGHT - frameHeight,
        frameWidth,
        frameHeight
    };
    boss->health = BOSS_HEALTH;
    boss->facingLeft = true;
    boss->isHurt = false;
    boss->hurtTimer = 0;
    boss->isActive = false;
    boss->shouldSpawn = false;
}

void initEnemies(Enemy enemies[], int worldWidth) {
    memset(enemies, 0, sizeof(Enemy) * MAX_ENEMIES);
    
    SDL_Surface* enemySpriteSheet = IMG_Load("anim.png");
    if (!enemySpriteSheet) {
        fprintf(stderr, "Failed to load enemy sprite sheet!\n");
        exit(1);
    }
    
    int frameWidth = enemySpriteSheet->w / ANIMATION_FRAMES;
    int frameHeight = enemySpriteSheet->h / 5; // Same layout as boss
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].spriteSheet = enemySpriteSheet;
        
        // Initialize animation frames (same as boss)
        for (int j = 0; j < ANIMATION_FRAMES; j++) {
            enemies[i].attackFrames[j] = (SDL_Rect){j * frameWidth, 0, frameWidth, frameHeight};
            enemies[i].deathFrames[j] = (SDL_Rect){j * frameWidth, frameHeight, frameWidth, frameHeight};
            enemies[i].hurtFrames[j] = (SDL_Rect){j * frameWidth, frameHeight * 2, frameWidth, frameHeight};
            enemies[i].idleFrames[j] = (SDL_Rect){j * frameWidth, frameHeight * 3, frameWidth, frameHeight};
            enemies[i].walkFrames[j] = (SDL_Rect){j * frameWidth, frameHeight * 4, frameWidth, frameHeight};
        }
        
        enemies[i].currentFrame = enemies[i].idleFrames[0];
        enemies[i].frameTime = SDL_GetTicks();
        enemies[i].currentAnimFrame = 0;
        
        enemies[i].pos = (SDL_Rect){
            500 + (i * 300),
            GROUND_PLATFORM_HEIGHT - frameHeight,
            frameWidth,
            frameHeight
        };
        enemies[i].health = ENEMY_HEALTH;
        enemies[i].facingLeft = true;
        enemies[i].isHurt = false;
        enemies[i].hurtTimer = 0;
        enemies[i].isActive = true;
        enemies[i].isDead = false;
    }
}

void updateBossAnimation(Boss* boss) {
    Uint32 currentTime = SDL_GetTicks();
    
    if (currentTime - boss->frameTime > ANIMATION_DELAY) {
        boss->frameTime = currentTime;
        boss->currentAnimFrame = (boss->currentAnimFrame + 1) % ANIMATION_FRAMES;
        
        if (boss->health <= 0) {
            boss->currentFrame = boss->deathFrames[boss->currentAnimFrame];
        } 
        else if (boss->isAttacking) {
            boss->currentFrame = boss->attackFrames[boss->currentAnimFrame];
        } 
        else if (boss->isHurt && currentTime - boss->hurtTimer < HURT_DURATION) {
            boss->currentFrame = boss->hurtFrames[boss->currentAnimFrame];
        }
        else if (fabs(boss->vx) > 0.1f) {
            boss->currentFrame = boss->walkFrames[boss->currentAnimFrame];
        } 
        else {
            boss->currentFrame = boss->idleFrames[boss->currentAnimFrame];
        }
    }
}

void updateEnemyAnimation(Enemy* enemy) {
    Uint32 currentTime = SDL_GetTicks();
    
    if (currentTime - enemy->frameTime > ANIMATION_DELAY) {
        enemy->frameTime = currentTime;
        enemy->currentAnimFrame = (enemy->currentAnimFrame + 1) % ANIMATION_FRAMES;
        
        if (enemy->health <= 0) {
            enemy->currentFrame = enemy->deathFrames[enemy->currentAnimFrame];
        } 
        else if (enemy->isAttacking) {
            enemy->currentFrame = enemy->attackFrames[enemy->currentAnimFrame];
        } 
        else if (enemy->isHurt && currentTime - enemy->hurtTimer < HURT_DURATION) {
            enemy->currentFrame = enemy->hurtFrames[enemy->currentAnimFrame];
        }
        else if (fabs(enemy->vx) > 0.1f) {
            enemy->currentFrame = enemy->walkFrames[enemy->currentAnimFrame];
        } 
        else {
            enemy->currentFrame = enemy->idleFrames[enemy->currentAnimFrame];
        }
    }
}

void handleEvents(Player* p, Player* p2, bool* running, Background* bg) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;

            case SDL_KEYDOWN:
                if (bg->state == GAME_STATE_PLAYING) {
                    // Player 1 controls
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT: 
                            p->ax = -PLAYER_ACCEL; 
                            p->facingRight = false;
                            break;
                        case SDLK_RIGHT: 
                            p->ax = PLAYER_ACCEL; 
                            p->facingRight = true;
                            break;
                        case SDLK_UP:
                            if (!p->isJumping) {
                                p->vy = JUMP_FORCE;
                                p->isJumping = true;
                                p->lastJumpTime = SDL_GetTicks();
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
                    }
                    
                    // Player 2 controls
                    switch (event.key.keysym.sym) {
                        case SDLK_q: 
                            p2->ax = -PLAYER_ACCEL; 
                            p2->facingRight = false;
                            break;
                        case SDLK_d: 
                            p2->ax = PLAYER_ACCEL; 
                            p2->facingRight = true;
                            break;
                        case SDLK_z:
                            if (!p2->isJumping) {
                                p2->vy = JUMP_FORCE;
                                p2->isJumping = true;
                                p2->lastJumpTime = SDL_GetTicks();
                            }
                            break;
                        case SDLK_j:
                            if (SDL_GetTicks() - p2->lastAttackTime > PLAYER_ATTACK_DURATION * 2) {
                                p2->isAttacking = true;
                                p2->lastAttackTime = SDL_GetTicks();
                                p2->attackStartTime = SDL_GetTicks();
                                p2->attackHitbox = (SDL_Rect){
                                    p2->pos.x + (p2->facingRight ? p2->pos.w : -PLAYER_ATTACK_RANGE),
                                    p2->pos.y + (p2->pos.h/2 - p2->attack_sprite->h/2),
                                    PLAYER_ATTACK_RANGE,
                                    p2->attack_sprite->h
                                };
                            }
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
                if (bg->state == GAME_STATE_PLAYING) {
                    // Player 1 controls
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT: if (p->ax < 0) p->ax = 0; break;
                        case SDLK_RIGHT: if (p->ax > 0) p->ax = 0; break;
                    }
                    
                    // Player 2 controls
                    switch (event.key.keysym.sym) {
                        case SDLK_q: if (p2->ax < 0) p2->ax = 0; break;
                        case SDLK_d: if (p2->ax > 0) p2->ax = 0; break;
                    }
                }
                break;
        }
    }
}

void updateBossAI(Boss* boss, Player* player, Background* bg) {
    if (!boss->isActive) {
        if (boss->shouldSpawn) {
            float distance = fabs((player->pos.x + bg->cameraX) - boss->pos.x);
            if (distance < BOSS_AGGRO_RANGE) {
                boss->isActive = true;
            }
        }
        return;
    }

    float playerX = player->pos.x + bg->cameraX;
    boss->facingLeft = (playerX < boss->pos.x);

    float distance = fabs(playerX - boss->pos.x);
    if (distance > BOSS_ATTACK_RANGE) {
        if (playerX < boss->pos.x) {
            boss->vx = -BOSS_MOVE_SPEED;
        } else {
            boss->vx = BOSS_MOVE_SPEED;
        }
    } else {
        boss->vx = 0;
    }

    boss->pos.x += boss->vx;

    if (distance < BOSS_ATTACK_RANGE && 
        SDL_GetTicks() - boss->lastAttackTime > BOSS_ATTACK_COOLDOWN) {
        boss->isAttacking = true;
        boss->lastAttackTime = SDL_GetTicks();
        boss->attackStartTime = SDL_GetTicks();
        boss->attackHitbox = (SDL_Rect){
            boss->facingLeft ? boss->pos.x - boss->pos.w/2 : boss->pos.x + boss->pos.w,
            boss->pos.y,
            boss->pos.w * 1.5,
            boss->pos.h
        };
    }

    if (boss->isAttacking && SDL_GetTicks() - boss->attackStartTime > BOSS_ATTACK_DURATION) {
        boss->isAttacking = false;
        boss->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    }
}

void updateEnemyAI(Enemy* enemy, Player* p1, Player* p2, Background* bg, bool* enemyIsAttacking) {
    if (!enemy->isActive || enemy->isDead) return;

    Uint32 currentTime = SDL_GetTicks();
    
    // Calculate distances to both players
    float distanceToP1 = fabs((p1->pos.x + bg->cameraX) - enemy->pos.x);
    float distanceToP2 = fabs((p2->pos.x + bg->cameraX) - enemy->pos.x);
    
    // Determine which player to target
    Player* targetPlayer = NULL;
    if (distanceToP1 < ENEMY_AGGRO_RANGE || distanceToP2 < ENEMY_AGGRO_RANGE) {
        // At least one player is in range
        if (distanceToP1 < distanceToP2) {
            targetPlayer = p1;
        } else {
            targetPlayer = p2;
        }
        
        enemy->isAggroed = true;
        *enemyIsAttacking = true;
        
        float playerX = targetPlayer->pos.x + bg->cameraX;
        float distance = fabs(playerX - enemy->pos.x);
        
        // Chase behavior
        enemy->facingLeft = (playerX < enemy->pos.x);
        
        if (distance > BOSS_ATTACK_RANGE) {
            if (playerX < enemy->pos.x) {
                enemy->vx = -ENEMY_CHASE_SPEED;
            } else {
                enemy->vx = ENEMY_CHASE_SPEED;
            }
        } else {
            enemy->vx = 0;
        }
        
        // Attack if in range
        if (distance < BOSS_ATTACK_RANGE && 
            currentTime - enemy->lastAttackTime > ENEMY_ATTACK_COOLDOWN) {
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
        enemy->isAggroed = false;
        *enemyIsAttacking = false;
        
        // Roaming behavior when no players are in range
        if (!enemy->isMoving) {
            // Idle state
            if (currentTime - enemy->lastMoveTime > ENEMY_IDLE_TIME) {
                // Time to move - choose random direction
                enemy->isMoving = true;
                enemy->moveDirection = (rand() % 2) ? 1 : -1;
                enemy->facingLeft = (enemy->moveDirection == -1);
                enemy->lastMoveTime = currentTime;
            }
        } else {
            // Moving state
            if (currentTime - enemy->lastMoveTime > ENEMY_MOVE_TIME) {
                // Done moving, go back to idle
                enemy->isMoving = false;
                enemy->vx = 0;
                enemy->lastMoveTime = currentTime;
            } else {
                // Continue moving in chosen direction
                enemy->vx = enemy->moveDirection * ENEMY_ROAM_SPEED;
            }
        }
    }

    // Apply movement
    enemy->pos.x += enemy->vx;

    // Check if attack duration has passed
    if (enemy->isAttacking && currentTime - enemy->attackStartTime > ENEMY_ATTACK_DURATION) {
        enemy->isAttacking = false;
        enemy->attackHitbox = (SDL_Rect){0, 0, 0, 0};
        *enemyIsAttacking = false;
    }
}

void resetGame(Player* p, Player* p2, Background* bg, Platform platforms[], int* platformCount, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    initPlayer(p, false);
    initPlayer(p2, true);
    bg->cameraX = 0;
    bg->startTime = SDL_GetTicks();
    bg->needsReset = false;

    initPlatforms(platforms, platformCount, bg->width);
    initBoss(boss, bg->width);
    initEnemies(enemies, bg->width);

    *enemiesRemaining = MAX_ENEMIES;
}

void updateGame(Player* p, Player* p2, Platform platforms[], int* count, Background* bg, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    if (bg->needsReset) {
        resetGame(p, p2, bg, platforms, count, boss, enemies, enemiesRemaining);
        return;
    }

    if (bg->state != GAME_STATE_PLAYING) return;

    static bool enemyIsAttacking = false;

    int aliveEnemies = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].isActive && !enemies[i].isDead) aliveEnemies++;
    }
    *enemiesRemaining = aliveEnemies;

    if (*enemiesRemaining <= 0 && !boss->shouldSpawn) {
        boss->shouldSpawn = true;
    }

    // Update both players
    Player* players[] = {p, p2};
    for (int i = 0; i < 2; i++) {
        Player* currentPlayer = players[i];
        
        // Player movement
        currentPlayer->vx += currentPlayer->ax;
        if (currentPlayer->ax == 0) currentPlayer->vx *= FRICTION;
        if (fabs(currentPlayer->vx) < 0.1f) currentPlayer->vx = 0;
        currentPlayer->vx = CLAMP(currentPlayer->vx, -PLAYER_SPEED, PLAYER_SPEED);
        
        // Apply horizontal movement
        currentPlayer->pos.x += currentPlayer->vx;
        
        // Check level completion
        if (currentPlayer->pos.x + currentPlayer->pos.w >= bg->width - LEVEL_END_MARGIN) {
            bg->state = GAME_STATE_VICTORY;
            currentPlayer->vx = 0;
            currentPlayer->ax = 0;
            return;
        }
        
        // Apply gravity
        currentPlayer->vy += GRAVITY;
        currentPlayer->pos.y += currentPlayer->vy;
        
        // Check platform collisions
        currentPlayer->isJumping = true;
        bool wasOnPlatform = currentPlayer->isOnMovingPlatform;
        currentPlayer->isOnMovingPlatform = false;
        
        for (int j = 0; j < *count; j++) {
            if (!platforms[j].isActive) continue;
            
            SDL_Rect playerWorldPos = { 
                currentPlayer->pos.x + (int)bg->cameraX, 
                currentPlayer->pos.y, 
                currentPlayer->pos.w, 
                currentPlayer->pos.h 
            };
            
            if (checkCollision(playerWorldPos, platforms[j].rect)) {
                if (currentPlayer->vy >= 0 && 
                    currentPlayer->pos.y + currentPlayer->pos.h >= platforms[j].rect.y && 
                    currentPlayer->pos.y + currentPlayer->pos.h <= platforms[j].rect.y + platforms[j].rect.h) {
                    
                    if (!platforms[j].isMoving) {
                        if (checkPixelPerfectCollision(currentPlayer->sprite, currentPlayer->pos, platforms[j].image, platforms[j].rect)) {
                            // Collision detected
                        }
                    }
                    
                    currentPlayer->pos.y = platforms[j].rect.y - currentPlayer->pos.h;
                    currentPlayer->vy = 0;
                    currentPlayer->isJumping = false;
                    currentPlayer->lastGroundY = currentPlayer->pos.y;
                    
                    if (platforms[j].isMoving) {
                        currentPlayer->isOnMovingPlatform = true;
                        currentPlayer->movingPlatformId = j;
                        if (wasOnPlatform && currentPlayer->movingPlatformId == j) {
                            currentPlayer->pos.x += MOVING_PLATFORM_SPEED * platforms[j].moveDirection;
                        }
                    }
                }
            }
        }

        // Check if player fell off screen
        if (currentPlayer->pos.y > SCREEN_HEIGHT) {
            currentPlayer->health -= 10;
            currentPlayer->pos.y = GROUND_PLATFORM_HEIGHT - currentPlayer->pos.h;
            currentPlayer->vy = 0;
            currentPlayer->isJumping = false;
            if (currentPlayer->health <= 0) {
                bg->state = GAME_STATE_GAME_OVER;
            }
        }
        
        // Update player attack state
        if (currentPlayer->isAttacking && SDL_GetTicks() - currentPlayer->attackStartTime > PLAYER_ATTACK_DURATION) {
            currentPlayer->isAttacking = false;
            currentPlayer->attackHitbox = (SDL_Rect){0, 0, 0, 0};
        }
    }

    // Update camera position to center between both players
    int player1WorldX = p->pos.x + bg->cameraX;
    int player2WorldX = p2->pos.x + bg->cameraX;
    int centerX = (player1WorldX + player2WorldX) / 2;
    bg->cameraX = centerX - SCREEN_WIDTH / 2;

    // Keep camera within bounds
    if (bg->cameraX < 0) bg->cameraX = 0;
    if (bg->cameraX + SCREEN_WIDTH > bg->width) bg->cameraX = bg->width - SCREEN_WIDTH;

    // Update player screen positions based on camera
    p->pos.x = player1WorldX - bg->cameraX;
    p2->pos.x = player2WorldX - bg->cameraX;

    // Update moving platforms
    for (int i = 0; i < *count; i++) {
        if (platforms[i].isMoving && platforms[i].isActive) {
            platforms[i].rect.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
            if (abs(platforms[i].rect.x - platforms[i].originalX) > platforms[i].moveDistance) {
                platforms[i].moveDirection *= -1;
            }
        }
    }

    updateBossAI(boss, p, bg);
    updateBossAnimation(boss);

    // Check collisions for both players
    for (int i = 0; i < 2; i++) {
        Player* currentPlayer = players[i];
        SDL_Rect playerWorldPos = currentPlayer->pos;
        playerWorldPos.x += (int)bg->cameraX;
        
        // Check boss attack collision
        if (boss->isAttacking && checkCollision(playerWorldPos, boss->attackHitbox) && 
            !currentPlayer->isInvincible && SDL_GetTicks() - currentPlayer->lastHitTime > INVINCIBILITY_DURATION) {
            currentPlayer->health -= BOSS_ATTACK_DAMAGE;
            currentPlayer->lastHitTime = SDL_GetTicks();
            currentPlayer->isInvincible = true;
            if (currentPlayer->health <= 0) {
                bg->state = GAME_STATE_GAME_OVER;
            }
        } else if (SDL_GetTicks() - currentPlayer->lastHitTime > INVINCIBILITY_DURATION) {
            currentPlayer->isInvincible = false;
        }
        
        // Check player attack collision with boss
        if (currentPlayer->isAttacking) {
            SDL_Rect attackWorldPos = currentPlayer->attackHitbox;
            attackWorldPos.x += (int)bg->cameraX;
            
            if (checkCollision(attackWorldPos, boss->pos)) {
                boss->health -= PLAYER_ATTACK_DAMAGE;
                boss->isHurt = true;
                boss->hurtTimer = SDL_GetTicks();
                if (boss->health <= 0) {
                    bg->state = GAME_STATE_VICTORY;
                }
            }
            
            // Check player attack collision with enemies
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].isActive && !enemies[j].isDead && checkCollision(attackWorldPos, enemies[j].pos)) {
                    enemies[j].health -= PLAYER_ATTACK_DAMAGE;
                    enemies[j].isHurt = true;
                    enemies[j].hurtTimer = SDL_GetTicks();
                    if (enemies[j].health <= 0) {
                        enemies[j].isDead = true;
                    }
                }
            }
        }
        
        // Check stomp attack on boss
        if (checkCollision(playerWorldPos, boss->pos) && currentPlayer->vy > 0) {
            boss->health -= PLAYER_ATTACK_DAMAGE * 2;
            boss->isHurt = true;
            boss->hurtTimer = SDL_GetTicks();
            currentPlayer->vy = JUMP_FORCE / 2;
            if (boss->health <= 0) {
                bg->state = GAME_STATE_VICTORY;
            }
        }
    }

    // Update enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        
        updateEnemyAI(&enemies[i], p, p2, bg, &enemyIsAttacking);
        updateEnemyAnimation(&enemies[i]);
        
        // Check enemy attack collision with both players
        for (int j = 0; j < 2; j++) {
            Player* currentPlayer = players[j];
            SDL_Rect playerWorldPos = currentPlayer->pos;
            playerWorldPos.x += (int)bg->cameraX;
            
            if (enemies[i].isAttacking && checkCollision(playerWorldPos, enemies[i].attackHitbox) && 
                !currentPlayer->isInvincible && SDL_GetTicks() - currentPlayer->lastHitTime > INVINCIBILITY_DURATION) {
                currentPlayer->health -= BOSS_ATTACK_DAMAGE / 2;
                currentPlayer->lastHitTime = SDL_GetTicks();
                currentPlayer->isInvincible = true;
                if (currentPlayer->health <= 0) {
                    bg->state = GAME_STATE_GAME_OVER;
                }
            }
        }
        
        if (enemies[i].pos.y > SCREEN_HEIGHT + 100) {
            enemies[i].isDead = true;
        }
    }
}

void renderTimer(Background* bg) {
    if (!font || !screen || bg->state == GAME_STATE_MENU) return;

    Uint32 currentTime = SDL_GetTicks() - bg->startTime;
    Uint32 seconds = currentTime / 1000;
    Uint32 minutes = seconds / 60;
    seconds %= 60;

    char timeText[20];
    snprintf(timeText, sizeof(timeText), "%02d:%02d", minutes, seconds);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, timeText, white);

    if (textSurface) {
        SDL_Rect textRect = {TIMER_POS_X, TIMER_POS_Y, textSurface->w, textSurface->h};
        SDL_BlitSurface(textSurface, NULL, screen, &textRect);
        SDL_FreeSurface(textSurface);
    }
}

void renderHealthBars(Player* p, Player* p2, Boss* boss, Background* bg) {
    if (!screen || bg->state == GAME_STATE_MENU) return;

    // Player 1 health
    SDL_Rect playerHealthBg = {10, 40, 100, 10};
    SDL_Rect playerHealthFg = {10, 40, (int)(100 * ((float)p->health / PLAYER_HEALTH)), 10};
    SDL_FillRect(screen, &playerHealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
    SDL_FillRect(screen, &playerHealthFg, SDL_MapRGB(screen->format, 0, 255, 0));

    // Player 2 health
    SDL_Rect player2HealthBg = {10, 60, 100, 10};
    SDL_Rect player2HealthFg = {10, 60, (int)(100 * ((float)p2->health / PLAYER_HEALTH)), 10};
    SDL_FillRect(screen, &player2HealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
    SDL_FillRect(screen, &player2HealthFg, SDL_MapRGB(screen->format, 0, 0, 255)); // Blue for player 2

    // Boss health
    if (boss->isActive) {
        SDL_Rect bossScreenPos = {
            boss->pos.x - (int)bg->cameraX,
            boss->pos.y,
            boss->pos.w,
            boss->pos.h
        };
        
        if (bossScreenPos.x + boss->pos.w > 0 && bossScreenPos.x < SCREEN_WIDTH) {
            SDL_Rect bossHealthBg = {
                bossScreenPos.x,
                bossScreenPos.y - 10,
                BOSS_WIDTH,
                5
            };
            SDL_Rect bossHealthFg = {
                bossScreenPos.x,
                bossScreenPos.y - 10,
                (int)(BOSS_WIDTH * ((float)boss->health / BOSS_HEALTH)),
                5
            };
            SDL_FillRect(screen, &bossHealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
            SDL_FillRect(screen, &bossHealthFg, SDL_MapRGB(screen->format, 255, 255, 0));
        }
    }
}

void renderEnemyCounter(int enemiesRemaining) {
    if (!font || !screen) return;

    char enemyText[50];
    if (enemiesRemaining > 0) {
        snprintf(enemyText, sizeof(enemyText), "Enemies Left: %d", enemiesRemaining);
    } else {
        snprintf(enemyText, sizeof(enemyText), "Head to the boss!");
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, enemyText, white);

    if (textSurface) {
        SDL_Rect textRect = {SCREEN_WIDTH - textSurface->w - 10, 10, textSurface->w, textSurface->h};
        SDL_BlitSurface(textSurface, NULL, screen, &textRect);
        SDL_FreeSurface(textSurface);
    }
}

void renderMenu(Background* bg) {
    if (!font || !screen) return;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    // Title
    TTF_Font* titleFont = TTF_OpenFont("arial.ttf", 48);
    if (titleFont) {
        SDL_Surface* titleSurface = TTF_RenderText_Solid(titleFont, "PLATFORM ADVENTURE", yellow);
        if (titleSurface) {
            SDL_Rect titleRect = {
                SCREEN_WIDTH/2 - titleSurface->w/2,
                SCREEN_HEIGHT/3 - titleSurface->h/2,
                titleSurface->w,
                titleSurface->h
            };
            SDL_BlitSurface(titleSurface, NULL, screen, &titleRect);
            SDL_FreeSurface(titleSurface);
        }
        TTF_CloseFont(titleFont);
    }

    // Instructions
    SDL_Surface* instruction1 = TTF_RenderText_Solid(font, "Player 1: Arrow Keys to Move, Space to Attack", white);
    SDL_Surface* instruction2 = TTF_RenderText_Solid(font, "Player 2: Q/D to Move, Z to Jump, J to Attack", white);
    SDL_Surface* instruction3 = TTF_RenderText_Solid(font, "Press Enter to Start", yellow);

    if (instruction1 && instruction2 && instruction3) {
        SDL_Rect inst1Rect = {
            SCREEN_WIDTH/2 - instruction1->w/2,
            SCREEN_HEIGHT/2,
            instruction1->w,
            instruction1->h
        };
        SDL_Rect inst2Rect = {
            SCREEN_WIDTH/2 - instruction2->w/2,
            SCREEN_HEIGHT/2 + 40,
            instruction2->w,
            instruction2->h
        };
        SDL_Rect inst3Rect = {
            SCREEN_WIDTH/2 - instruction3->w/2,
            SCREEN_HEIGHT/2 + 100,
            instruction3->w,
            instruction3->h
        };
        
        SDL_BlitSurface(instruction1, NULL, screen, &inst1Rect);
        SDL_BlitSurface(instruction2, NULL, screen, &inst2Rect);
        SDL_BlitSurface(instruction3, NULL, screen, &inst3Rect);
    }

    if (instruction1) SDL_FreeSurface(instruction1);
    if (instruction2) SDL_FreeSurface(instruction2);
    if (instruction3) SDL_FreeSurface(instruction3);
}

void renderEndMessage(Background* bg) {
    if (!font || !screen || (bg->state != GAME_STATE_GAME_OVER && bg->state != GAME_STATE_VICTORY)) return;

    SDL_Color color = (bg->state == GAME_STATE_VICTORY) ? (SDL_Color){0, 255, 0, 255} : (SDL_Color){255, 0, 0, 255};
    const char* message = (bg->state == GAME_STATE_VICTORY) ? "VICTORY! Press R to restart" : "GAME OVER! Press R to restart";

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message, color);

    if (textSurface) {
        SDL_Rect textRect = {
            SCREEN_WIDTH/2 - textSurface->w/2,
            SCREEN_HEIGHT/2 - textSurface->h/2,
            textSurface->w,
            textSurface->h
        };
        SDL_BlitSurface(textSurface, NULL, screen, &textRect);
        SDL_FreeSurface(textSurface);
    }
}

void renderMinimap(Player* p, Player* p2, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[]) {
    if (!screen || !bg->img || bg->state != GAME_STATE_PLAYING) return;

    SDL_Surface* minimap = SDL_CreateRGBSurface(SDL_SWSURFACE, MINIMAP_WIDTH, MINIMAP_HEIGHT, 32, 0, 0, 0, 0);
    if (!minimap) return;

    // Create a scaled-down version of the background
    SDL_Surface* minimapBg = SDL_CreateRGBSurface(SDL_SWSURFACE, MINIMAP_WIDTH, MINIMAP_HEIGHT, 32, 0, 0, 0, 0);
    if (!minimapBg) {
        SDL_FreeSurface(minimap);
        return;
    }

    // Stretch the background to fit the minimap
    SDL_Rect bgSrcRect = {
        (int)bg->cameraX, 0,
        MINIMAP_WIDTH / MINIMAP_SCALE,
        MINIMAP_HEIGHT / MINIMAP_SCALE
    };
    SDL_Rect bgDestRect = {0, 0, MINIMAP_WIDTH, MINIMAP_HEIGHT};
    SDL_BlitSurface(bg->img, &bgSrcRect, minimapBg, &bgDestRect);

    // Fill the minimap with the scaled background
    SDL_BlitSurface(minimapBg, NULL, minimap, NULL);
    SDL_FreeSurface(minimapBg);

    // Draw platforms
    for (int i = 0; i < platformCount; i++) {
        if (!platforms[i].isActive) continue;
        
        SDL_Rect platformRect = {
            (int)(platforms[i].rect.x * MINIMAP_SCALE) - (int)(bg->cameraX * MINIMAP_SCALE),
            (int)(platforms[i].rect.y * MINIMAP_SCALE),
            (int)(platforms[i].rect.w * MINIMAP_SCALE),
            (int)(platforms[i].rect.h * MINIMAP_SCALE)
        };
        
        Uint32 platformColor = platforms[i].isMoving ? SDL_MapRGB(minimap->format, 0, 100, 255) 
                                                   : SDL_MapRGB(minimap->format, 100, 100, 100);
        SDL_FillRect(minimap, &platformRect, platformColor);
    }

    // Draw enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        
        SDL_Rect enemyRect = {
            (int)(enemies[i].pos.x * MINIMAP_SCALE) - (int)(bg->cameraX * MINIMAP_SCALE),
            (int)(enemies[i].pos.y * MINIMAP_SCALE) + 5,
            4, 4
        };
        SDL_FillRect(minimap, &enemyRect, SDL_MapRGB(minimap->format, 255, 0, 0));
    }

    // Draw boss if active
    if (boss->isActive || boss->shouldSpawn) {
        SDL_Rect bossRect = {
            (int)(boss->pos.x * MINIMAP_SCALE) - (int)(bg->cameraX * MINIMAP_SCALE),
            (int)(boss->pos.y * MINIMAP_SCALE) + 5,
            4, 4
        };
        SDL_FillRect(minimap, &bossRect, SDL_MapRGB(minimap->format, 255, 165, 0));
    }

    // Draw players
    SDL_Rect playerRect = {
        (int)(p->pos.x * MINIMAP_SCALE),
        (int)(p->pos.y * MINIMAP_SCALE) + 5,
        4, 4
    };
    SDL_FillRect(minimap, &playerRect, SDL_MapRGB(minimap->format, 0, 255, 0));

    SDL_Rect player2Rect = {
        (int)(p2->pos.x * MINIMAP_SCALE),
        (int)(p2->pos.y * MINIMAP_SCALE) + 5,
        4, 4
    };
    SDL_FillRect(minimap, &player2Rect, SDL_MapRGB(minimap->format, 0, 0, 255));

    // Position the minimap (top-right, below enemy counter)
    SDL_Color white = {255, 255, 255, 255};
    char enemyText[50];
    snprintf(enemyText, sizeof(enemyText), "Enemies Left: %d", 0);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, enemyText, white);
    int enemyCounterHeight = textSurface ? textSurface->h : 0;
    if (textSurface) SDL_FreeSurface(textSurface);

    SDL_Rect minimapPos = {
        SCREEN_WIDTH - MINIMAP_WIDTH - 10,
        10 + enemyCounterHeight + 10,
        MINIMAP_WIDTH,
        MINIMAP_HEIGHT
    };
    SDL_BlitSurface(minimap, NULL, screen, &minimapPos);
    SDL_FreeSurface(minimap);
}

void renderGame(Player* p, Player* p2, Background* bg, Platform platforms[], int count, Boss* boss, Enemy enemies[], int enemiesRemaining) {
    if (!screen) return;

    // Clear screen
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 50, 50, 100));

    if (bg->state == GAME_STATE_MENU) {
        renderMenu(bg);
        SDL_Flip(screen);
        return;
    }

    if (!bg->img) return;

    // Render background with camera offset
    SDL_Rect src_rect = {
        (int)bg->cameraX, 0,
        MIN(SCREEN_WIDTH, bg->width - (int)bg->cameraX),
        bg->img->h
    };

    SDL_Rect dest_rect = {0, 0, src_rect.w, SCREEN_HEIGHT};
    SDL_Surface* stretched_bg = SDL_CreateRGBSurface(SDL_SWSURFACE, src_rect.w, SCREEN_HEIGHT, screen->format->BitsPerPixel, 0, 0, 0, 0);

    if (stretched_bg) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            int src_y = (y * bg->img->h) / SCREEN_HEIGHT;
            SDL_Rect src_line = {src_rect.x, src_y, src_rect.w, 1};
            SDL_Rect dest_line = {0, y, src_rect.w, 1};
            SDL_BlitSurface(bg->img, &src_line, stretched_bg, &dest_line);
        }
        
        SDL_BlitSurface(stretched_bg, NULL, screen, &dest_rect);
        SDL_FreeSurface(stretched_bg);
    }

    if (src_rect.w < SCREEN_WIDTH) {
        SDL_Rect fill = {src_rect.w, 0, SCREEN_WIDTH - src_rect.w, SCREEN_HEIGHT};
        SDL_FillRect(screen, &fill, SDL_MapRGB(screen->format, 50, 50, 100));
    }

    // Render platforms
    for (int i = 0; i < count; i++) {
        if (!platforms[i].isActive || !platforms[i].image) continue;
        
        int platformScreenX = platforms[i].rect.x - (int)bg->cameraX;
        
        if (platformScreenX + platforms[i].rect.w > 0 && platformScreenX < SCREEN_WIDTH) {
            SDL_Rect r = {
                platformScreenX,
                platforms[i].rect.y,
                platforms[i].rect.w,
                platforms[i].rect.h
            };
            
            int tiles = (r.w + platforms[i].image->w - 1) / platforms[i].image->w;
            for (int t = 0; t < tiles; t++) {
                SDL_Rect tileRect = {
                    r.x + t * platforms[i].image->w,
                    r.y,
                    MIN(platforms[i].image->w, r.w - t * platforms[i].image->w),
                    r.h
                };
                SDL_BlitSurface(platforms[i].image, NULL, screen, &tileRect);
            }
        }
    }

    // Render enemies with animations
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        
        updateEnemyAnimation(&enemies[i]);
        
        SDL_Rect enemyScreenPos = {
            enemies[i].pos.x - (int)bg->cameraX,
            enemies[i].pos.y,
            enemies[i].pos.w,
            enemies[i].pos.h
        };
        
        if (enemyScreenPos.x + enemies[i].pos.w > 0 && enemyScreenPos.x < SCREEN_WIDTH) {
            if (enemies[i].facingLeft) {
                SDL_Surface* flipped = flipSurface(enemies[i].spriteSheet);
                if (flipped) {
                    SDL_BlitSurface(flipped, &enemies[i].currentFrame, screen, &enemyScreenPos);
                    SDL_FreeSurface(flipped);
                }
            } else {
                SDL_BlitSurface(enemies[i].spriteSheet, &enemies[i].currentFrame, screen, &enemyScreenPos);
            }
            
            // Enemy health bar
            SDL_Rect enemyHealthBg = {
                enemyScreenPos.x,
                enemyScreenPos.y - 10,
                enemies[i].pos.w,
                3
            };
            SDL_Rect enemyHealthFg = {
                enemyScreenPos.x,
                enemyScreenPos.y - 10,
                (int)(enemies[i].pos.w * ((float)enemies[i].health / ENEMY_HEALTH)),
                3
            };
            SDL_FillRect(screen, &enemyHealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
            SDL_FillRect(screen, &enemyHealthFg, SDL_MapRGB(screen->format, 0, 255, 0));
        }
    }

    // Render boss with animations
    if (boss->shouldSpawn) {
        updateBossAnimation(boss);
        
        SDL_Rect bossScreenPos = {
            boss->pos.x - (int)bg->cameraX,
            boss->pos.y,
            boss->pos.w,
            boss->pos.h
        };
        
        if (bossScreenPos.x + boss->pos.w > 0 && bossScreenPos.x < SCREEN_WIDTH) {
            if (boss->facingLeft) {
                SDL_Surface* flipped = flipSurface(boss->spriteSheet);
                if (flipped) {
                    SDL_BlitSurface(flipped, &boss->currentFrame, screen, &bossScreenPos);
                    SDL_FreeSurface(flipped);
                }
            } else {
                SDL_BlitSurface(boss->spriteSheet, &boss->currentFrame, screen, &bossScreenPos);
            }
        }
    }

    // Render players with invincibility flash
    if (p->sprite && (!p->isInvincible || (SDL_GetTicks() / 100) % 2 == 0)) {
        SDL_BlitSurface(p->sprite, NULL, screen, &p->pos);
    }
    if (p2->sprite && (!p2->isInvincible || (SDL_GetTicks() / 100) % 2 == 0)) {
        SDL_BlitSurface(p2->sprite, NULL, screen, &p2->pos);
    }

    // Render player attacks
    Player* players[] = {p, p2};
    for (int i = 0; i < 2; i++) {
        if (players[i]->isAttacking && players[i]->attack_sprite) {
            SDL_Rect attackEffect = {
                players[i]->pos.x + (players[i]->facingRight ? players[i]->pos.w : -players[i]->attack_sprite->w),
                players[i]->pos.y + (players[i]->pos.h/2 - players[i]->attack_sprite->h/2),
                players[i]->attack_sprite->w,
                players[i]->attack_sprite->h
            };
            
            if (players[i]->facingRight) {
                SDL_BlitSurface(players[i]->attack_sprite, NULL, screen, &attackEffect);
            } else {
                SDL_Surface* flipped = flipSurface(players[i]->attack_sprite);
                if (flipped) {
                    SDL_BlitSurface(flipped, NULL, screen, &attackEffect);
                    SDL_FreeSurface(flipped);
                }
            }
        }
    }

    // Render UI elements
    renderTimer(bg);
    renderHealthBars(p, p2, boss, bg);
    renderEnemyCounter(enemiesRemaining);
    renderEndMessage(bg);
    renderMinimap(p, p2, bg, platforms, count, boss, enemies);

    SDL_Flip(screen);
}

void cleanUp(Player* p, Player* p2, Background* bg, Boss* boss) {
    if (p->sprite) SDL_FreeSurface(p->sprite);
    if (p2->sprite) SDL_FreeSurface(p2->sprite);
    if (bg->img) SDL_FreeSurface(bg->img);
    if (boss->spriteSheet) SDL_FreeSurface(boss->spriteSheet);
    if (font) TTF_CloseFont(font);
    if (static_platform_img) SDL_FreeSurface(static_platform_img);
    if (moving_platform_img) SDL_FreeSurface(moving_platform_img);
    if (player_img) SDL_FreeSurface(player_img);
    if (player2_img) SDL_FreeSurface(player2_img);
    if (player_attack_img) SDL_FreeSurface(player_attack_img);
    if (background_img) SDL_FreeSurface(background_img);
    if (bgMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgMusic);
    }
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}
