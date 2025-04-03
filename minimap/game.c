#include "game.h"

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* static_platform_img = NULL;
SDL_Surface* moving_platform_img = NULL;
SDL_Surface* boss_img = NULL;
SDL_Surface* boss_attack_img = NULL;
SDL_Surface* player_img = NULL;
SDL_Surface* player_attack_img = NULL;

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

bool checkPixelPerfectCollision(SDL_Surface* src, SDL_Rect src_rect, SDL_Surface* dst, SDL_Rect dst_rect) {
    if (!src || !dst) return false;
    if (!checkCollision(src_rect, dst_rect)) return false;

    if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
    if (SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);

    bool collision = false;
    for (int y = MAX(src_rect.y, dst_rect.y); y < MIN(src_rect.y + src_rect.h, dst_rect.y + dst_rect.h); y++) {
        for (int x = MAX(src_rect.x, dst_rect.x); x < MIN(src_rect.x + src_rect.w, dst_rect.x + dst_rect.w); x++) {
            Uint32 src_pixel = ((Uint32*)src->pixels)[(y - src_rect.y) * src->w + (x - src_rect.x)];
            Uint32 dst_pixel = ((Uint32*)dst->pixels)[(y - dst_rect.y) * dst->w + (x - dst_rect.x)];
            if ((src_pixel >> 24) && (dst_pixel >> 24)) {
                collision = true;
                break;
            }
        }
        if (collision) break;
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
    
    for(int y = 0; y < surface->h; y++) {
        for(int x = 0; x < surface->w; x++) {
            for(int b = 0; b < surface->format->BytesPerPixel; b++) {
                flipped_pixels[y * pitch + x * surface->format->BytesPerPixel + b] = 
                    pixels[y * pitch + (surface->w - 1 - x) * surface->format->BytesPerPixel + b];
            }
        }
    }
    
    return flipped;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
    
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        exit(1);
    }
    
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if (!screen) {
        fprintf(stderr, "Failed to set video mode: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }
    
    SDL_WM_SetCaption("Platform Game with Minimap", NULL);
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
    boss_img = IMG_Load("boss.png");
    boss_attack_img = IMG_Load("boss_attack.png");
    player_img = IMG_Load("player.png");
    player_attack_img = IMG_Load("player_attack_slash.png");

    if (!static_platform_img || !moving_platform_img || !boss_img || 
        !boss_attack_img || !player_img || !player_attack_img) {
        fprintf(stderr, "Failed to load game images! SDL_Error: %s\n", IMG_GetError());
        exit(1);
    }

    if (player_img->w <= 0 || player_img->h <= 0) {
        fprintf(stderr, "Invalid player image dimensions!\n");
        exit(1);
    }
}

void initPlayer(Player* p) {
    if (!player_img || !player_attack_img) {
        fprintf(stderr, "Player images not loaded!\n");
        exit(1);
    }

    p->sprite = player_img;
    p->attack_sprite = player_attack_img;
    p->pos.w = player_img->w;
    p->pos.h = player_img->h;
    p->pos.x = PLAYER_X_POSITION;
    p->pos.y = GROUND_PLATFORM_HEIGHT - p->pos.h;
    p->vx = p->vy = 0;
    p->ax = p->ay = 0;
    p->isJumping = false;
    p->isOnMovingPlatform = false;
    p->movingPlatformId = -1;
    p->lastGroundY = p->pos.y;
    p->lastJumpTime = 0;
    p->health = PLAYER_HEALTH;
    p->isAttacking = false;
    p->lastAttackTime = 0;
    p->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    p->facingRight = true;
}

void initBackground(Background* bg) {
    bg->img = IMG_Load("background.png");
    if (!bg->img) {
        fprintf(stderr, "Failed to load background.png: %s\n", IMG_GetError());
        exit(1);
    }
    bg->cameraX = 0;
    bg->width = bg->img->w;
    bg->startTime = SDL_GetTicks();
    bg->levelCompleted = false;
    bg->gameOver = false;
    bg->shakeOffset = 0;
    bg->shakeStartTime = 0;
}

void initPlatforms(Platform platforms[], int* count, int worldWidth) {
    int platformCount = 0;
    platforms[platformCount++] = (Platform){
        {0, GROUND_PLATFORM_HEIGHT, worldWidth, PLATFORM_HEIGHT},
        NULL, false, true, false, 0, 0, 0
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
    if (!boss_img || !boss_attack_img) {
        fprintf(stderr, "Boss images not loaded!\n");
        exit(1);
    }

    boss->sprite = boss_img;
    boss->attack_sprite = boss_attack_img;
    boss->pos = (SDL_Rect){
        worldWidth - BOSS_X_OFFSET - BOSS_WIDTH,
        GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT,
        BOSS_WIDTH,
        BOSS_HEIGHT
    };
    boss->health = BOSS_HEALTH;
    boss->isAttacking = false;
    boss->lastAttackTime = 0;
    boss->attackHitbox = (SDL_Rect){0, 0, 0, 0};
    boss->facingLeft = true;
    boss->vx = 0;
    boss->isActive = false;
    boss->shouldSpawn = false;
}

void initEnemies(Enemy enemies[], int worldWidth) {
    if (!boss_img || !boss_attack_img) {
        fprintf(stderr, "Enemy images not loaded!\n");
        exit(1);
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i] = (Enemy){
            boss_img,
            boss_attack_img,
            {500 + (i * 300), GROUND_PLATFORM_HEIGHT - BOSS_HEIGHT, BOSS_WIDTH, BOSS_HEIGHT},
            ENEMY_HEALTH,
            false,
            0,
            {0, 0, 0, 0},
            true,
            0,
            true,
            false,
            false
        };
    }
}

void handleEvents(Player* p, bool* running, Background* bg) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: *running = false; break;
                
            case SDL_KEYDOWN:
                if (!bg->levelCompleted && !bg->gameOver) {
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
                            if (SDL_GetTicks() - p->lastAttackTime > ATTACK_DURATION * 2) {
                                p->isAttacking = true;
                                p->lastAttackTime = SDL_GetTicks();
                                p->attackHitbox = (SDL_Rect){
                                    p->pos.x + (p->facingRight ? p->pos.w : -PLAYER_ATTACK_RANGE),
                                    p->pos.y + (p->pos.h/2 - p->attack_sprite->h/2),
                                    PLAYER_ATTACK_RANGE,
                                    p->attack_sprite->h
                                };
                            }
                            break;
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) *running = false;
                if (event.key.keysym.sym == SDLK_r && (bg->levelCompleted || bg->gameOver)) *running = false;
                break;
                
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT: if (p->ax < 0) p->ax = 0; break;
                    case SDLK_RIGHT: if (p->ax > 0) p->ax = 0; break;
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
        boss->attackHitbox = (SDL_Rect){
            boss->facingLeft ? boss->pos.x - boss->pos.w/2 : boss->pos.x + boss->pos.w,
            boss->pos.y,
            boss->pos.w * 1.5,
            boss->pos.h
        };
    }
}

void updateEnemyAI(Enemy* enemy, Player* player, Background* bg, bool* enemyIsAttacking) {
    if (!enemy->isActive || enemy->isDead) return;

    float playerX = player->pos.x + bg->cameraX;
    float distance = fabs(playerX - enemy->pos.x);

    if (distance < ENEMY_AGGRO_RANGE && !(*enemyIsAttacking)) {
        enemy->isAggroed = true;
        *enemyIsAttacking = true;
    }

    if (enemy->isAggroed) {
        enemy->facingLeft = (playerX < enemy->pos.x);
        
        if (distance > BOSS_ATTACK_RANGE) {
            if (playerX < enemy->pos.x) {
                enemy->vx = -BOSS_MOVE_SPEED;
            } else {
                enemy->vx = BOSS_MOVE_SPEED;
            }
        } else {
            enemy->vx = 0;
        }
        
        enemy->pos.x += enemy->vx;
        
        if (distance < BOSS_ATTACK_RANGE && 
            SDL_GetTicks() - enemy->lastAttackTime > BOSS_ATTACK_COOLDOWN) {
            enemy->isAttacking = true;
            enemy->lastAttackTime = SDL_GetTicks();
            enemy->attackHitbox = (SDL_Rect){
                enemy->facingLeft ? enemy->pos.x - enemy->pos.w/2 : enemy->pos.x + enemy->pos.w,
                enemy->pos.y,
                enemy->pos.w * 1.5,
                enemy->pos.h
            };
        }
    }

    if (enemy->isAttacking) {
        enemy->attackHitbox.x = enemy->facingLeft ? 
            enemy->pos.x - enemy->pos.w/2 : 
            enemy->pos.x + enemy->pos.w;
        enemy->attackHitbox.y = enemy->pos.y;
    } else if (SDL_GetTicks() - enemy->lastAttackTime > BOSS_ATTACK_COOLDOWN / 2) {
        enemy->isAttacking = false;
        enemy->attackHitbox = (SDL_Rect){0, 0, 0, 0};
        *enemyIsAttacking = false;
    }
}

void applyShakeEffect(Background* bg) {
    if (SDL_GetTicks() - bg->shakeStartTime < SHAKE_DURATION) {
        bg->shakeOffset = (rand() % 5) - 2;
    } else {
        bg->shakeOffset = 0;
    }
}

void updateGame(Player* p, Platform platforms[], int* count, Background* bg, Boss* boss, Enemy enemies[], int* enemiesRemaining) {
    static bool enemyIsAttacking = false;
    
    if (!bg->levelCompleted && !bg->gameOver) {
        int aliveEnemies = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].isActive && !enemies[i].isDead) aliveEnemies++;
        }
        *enemiesRemaining = aliveEnemies;

        if (*enemiesRemaining <= 0 && !boss->shouldSpawn) {
            boss->shouldSpawn = true;
        }

        p->vx += p->ax;
        if (p->ax == 0) p->vx *= FRICTION;
        if (fabs(p->vx) < 0.1f) p->vx = 0;
        p->vx = MAX(-PLAYER_SPEED, MIN(PLAYER_SPEED, p->vx));
        bg->cameraX += p->vx;
        
        if (p->pos.x + p->pos.w >= bg->width - LEVEL_END_MARGIN) {
            bg->levelCompleted = true;
            p->vx = 0;
            p->ax = 0;
        }
        
        p->vy += GRAVITY;
        p->pos.y += p->vy;
        
        for (int i = 0; i < *count; i++) {
            if (platforms[i].isMoving && platforms[i].isActive) {
                platforms[i].rect.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
                if (abs(platforms[i].rect.x - platforms[i].originalX) > platforms[i].moveDistance) {
                    platforms[i].moveDirection *= -1;
                }
            }
        }
        
        p->isJumping = true;
        bool wasOnPlatform = p->isOnMovingPlatform;
        p->isOnMovingPlatform = false;
        
        for (int i = 0; i < *count; i++) {
            if (!platforms[i].isActive) continue;
            
            SDL_Rect playerWorldPos = { p->pos.x + (int)bg->cameraX, p->pos.y, p->pos.w, p->pos.h };
            if (checkCollision(playerWorldPos, platforms[i].rect)) {
                if (p->vy >= 0 && 
                    p->pos.y + p->pos.h >= platforms[i].rect.y && 
                    p->pos.y + p->pos.h <= platforms[i].rect.y + platforms[i].rect.h) {
                    
                    if (!platforms[i].isMoving) {
                        if (checkPixelPerfectCollision(p->sprite, p->pos, platforms[i].image, platforms[i].rect)) {
                            bg->shakeStartTime = SDL_GetTicks();
                        }
                    }
                    
                    p->pos.y = platforms[i].rect.y - p->pos.h;
                    p->vy = 0;
                    p->isJumping = false;
                    p->lastGroundY = p->pos.y;
                    
                    if (platforms[i].isMoving) {
                        p->isOnMovingPlatform = true;
                        p->movingPlatformId = i;
                        if (wasOnPlatform && p->movingPlatformId == i) {
                            p->pos.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
                        }
                    }
                }
            }
        }

        if (p->pos.y > SCREEN_HEIGHT) {
            p->health -= 10;
            p->pos.y = GROUND_PLATFORM_HEIGHT - p->pos.h;
            p->vy = 0;
            p->isJumping = false;
            if (p->health <= 0) {
                bg->gameOver = true;
            }
        }
        
        if (p->isAttacking && SDL_GetTicks() - p->lastAttackTime > ATTACK_DURATION) {
            p->isAttacking = false;
            p->attackHitbox = (SDL_Rect){0, 0, 0, 0};
        }
        
        updateBossAI(boss, p, bg);
        
        if (boss->isAttacking) {
            boss->attackHitbox.x = boss->facingLeft ? 
                boss->pos.x - boss->pos.w/2 : 
                boss->pos.x + boss->pos.w;
            boss->attackHitbox.y = boss->pos.y;
        } else if (SDL_GetTicks() - boss->lastAttackTime > BOSS_ATTACK_COOLDOWN / 2) {
            boss->isAttacking = false;
            boss->attackHitbox = (SDL_Rect){0, 0, 0, 0};
        }
        
        SDL_Rect playerWorldPos = p->pos;
        playerWorldPos.x += (int)bg->cameraX;
        
        if (boss->isAttacking && checkCollision(playerWorldPos, boss->attackHitbox)) {
            p->health -= BOSS_ATTACK_DAMAGE;
            if (p->health <= 0) {
                bg->gameOver = true;
            }
        }
        
        if (p->isAttacking) {
            SDL_Rect attackWorldPos = p->attackHitbox;
            attackWorldPos.x += (int)bg->cameraX;
            
            if (checkCollision(attackWorldPos, boss->pos)) {
                boss->health -= PLAYER_ATTACK_DAMAGE;
                if (boss->health <= 0) {
                    bg->levelCompleted = true;
                }
            }
            
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].isActive && !enemies[i].isDead && checkCollision(attackWorldPos, enemies[i].pos)) {
                    enemies[i].health -= PLAYER_ATTACK_DAMAGE;
                    if (enemies[i].health <= 0) {
                        enemies[i].isDead = true;
                    }
                }
            }
        }
        
        if (checkCollision(playerWorldPos, boss->pos) && p->vy > 0) {
            boss->health -= PLAYER_ATTACK_DAMAGE * 2;
            p->vy = JUMP_FORCE / 2;
            if (boss->health <= 0) {
                bg->levelCompleted = true;
            }
        }
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].isActive || enemies[i].isDead) continue;
            
            updateEnemyAI(&enemies[i], p, bg, &enemyIsAttacking);
            
            if (enemies[i].isAttacking && checkCollision(playerWorldPos, enemies[i].attackHitbox)) {
                p->health -= BOSS_ATTACK_DAMAGE / 2;
                if (p->health <= 0) {
                    bg->gameOver = true;
                }
            }
            
            if (enemies[i].pos.y > SCREEN_HEIGHT + 100) {
                enemies[i].isDead = true;
            }
        }

        applyShakeEffect(bg);
    }
}

void renderTimer(Background* bg) {
    if (!font || !screen) return;

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

void renderHealthBars(Player* p, Boss* boss, Background* bg) {
    if (!screen) return;

    SDL_Rect playerHealthBg = {10, 40, 100, 10};
    SDL_Rect playerHealthFg = {10, 40, (int)(100 * ((float)p->health / PLAYER_HEALTH)), 10};
    SDL_FillRect(screen, &playerHealthBg, SDL_MapRGB(screen->format, 255, 0, 0));
    SDL_FillRect(screen, &playerHealthFg, SDL_MapRGB(screen->format, 0, 255, 0));
    
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

void renderEndMessage(Background* bg) {
    if (!font || !screen) return;

    if (bg->levelCompleted) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "LEVEL COMPLETED! Press R to restart", white);
        
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
    
    if (bg->gameOver) {
        SDL_Color red = {255, 0, 0, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "GAME OVER! Press R to restart", red);
        
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
}

void renderMinimap(Player* p, Background* bg, Platform platforms[], int platformCount, Boss* boss, Enemy enemies[]) {
    if (!screen || !bg->img) return;

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
        (int)bg->cameraX + 2 , 50,
        MINIMAP_WIDTH / MINIMAP_SCALE,  // Scale inversely to fit
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
        
        Uint32 color = platforms[i].isMoving ? SDL_MapRGB(minimap->format, 0, 100, 255) 
                                            : SDL_MapRGB(minimap->format, 100, 100, 100);
        SDL_FillRect(minimap, &platformRect, color);
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
        SDL_FillRect(minimap, &bossRect, SDL_MapRGB(minimap->format, 255, 165, 0)); // Orange
    }

    // Draw player
    SDL_Rect playerRect = {
        (int)(p->pos.x * MINIMAP_SCALE),
        (int)(p->pos.y * MINIMAP_SCALE) + 5,
        4, 4
    };
    SDL_FillRect(minimap, &playerRect, SDL_MapRGB(minimap->format, 0, 255, 0));

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

void renderGame(Player* p, Background* bg, Platform platforms[], int count, Boss* boss, Enemy enemies[], int enemiesRemaining) {
    if (!screen || !bg->img) return;

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 50, 50, 100));
    
    SDL_Rect src_rect = {
        (int)bg->cameraX + bg->shakeOffset, 0,
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
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].isActive || enemies[i].isDead) continue;
        
        SDL_Rect enemyScreenPos = {
            enemies[i].pos.x - (int)bg->cameraX,
            enemies[i].pos.y,
            enemies[i].pos.w,
            enemies[i].pos.h
        };
        
        if (enemyScreenPos.x + enemies[i].pos.w > 0 && enemyScreenPos.x < SCREEN_WIDTH) {
            SDL_BlitSurface(enemies[i].sprite, NULL, screen, &enemyScreenPos);
            
            if (enemies[i].isAttacking) {
                SDL_Rect attackRect = {
                    enemies[i].attackHitbox.x - (int)bg->cameraX - 120,
                    enemies[i].attackHitbox.y - 100,
                    enemies[i].attackHitbox.w,
                    enemies[i].attackHitbox.h
                };
                if (enemies[i].facingLeft) {
                    SDL_Surface* flipped = flipSurface(enemies[i].attack_sprite);
                    if (flipped) {
                        SDL_BlitSurface(flipped, NULL, screen, &attackRect);
                        SDL_FreeSurface(flipped);
                    }
                } else {
                    SDL_BlitSurface(enemies[i].attack_sprite, NULL, screen, &attackRect);
                }
            }
            
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
    
    if (boss->shouldSpawn) {
        SDL_Rect bossScreenPos = {
            boss->pos.x - (int)bg->cameraX,
            boss->pos.y,
            boss->pos.w,
            boss->pos.h
        };
        if (bossScreenPos.x + boss->pos.w > 0 && bossScreenPos.x < SCREEN_WIDTH) {
            SDL_BlitSurface(boss->sprite, NULL, screen, &bossScreenPos);
            
            if (boss->isAttacking) {
                SDL_Rect attackRect = {
                    boss->attackHitbox.x - (int)bg->cameraX - 120,
                    boss->attackHitbox.y - 100,
                    boss->attackHitbox.w,
                    boss->attackHitbox.h
                };
                if (boss->facingLeft) {
                    SDL_Surface* flipped = flipSurface(boss->attack_sprite);
                    if (flipped) {
                        SDL_BlitSurface(flipped, NULL, screen, &attackRect);
                        SDL_FreeSurface(flipped);
                    }
                } else {
                    SDL_BlitSurface(boss->attack_sprite, NULL, screen, &attackRect);
                }
            }
            
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
    
    if (p->sprite) {
        SDL_BlitSurface(p->sprite, NULL, screen, &p->pos);
    }
    
    if (p->isAttacking && p->attack_sprite) {
        SDL_Rect attackEffect = {
            p->pos.x + (p->facingRight ? p->pos.w : -p->attack_sprite->w),
            p->pos.y + (p->pos.h/2 - p->attack_sprite->h/2),
            p->attack_sprite->w,
            p->attack_sprite->h
        };
        
        if (p->facingRight) {
            SDL_BlitSurface(p->attack_sprite, NULL, screen, &attackEffect);
        } else {
            SDL_Surface* flipped = flipSurface(p->attack_sprite);
            if (flipped) {
                SDL_BlitSurface(flipped, NULL, screen, &attackEffect);
                SDL_FreeSurface(flipped);
            }
        }
    }
    
    renderTimer(bg);
    renderHealthBars(p, boss, bg);
    renderEnemyCounter(enemiesRemaining);
    renderEndMessage(bg);
    renderMinimap(p, bg, platforms, count, boss, enemies);
    
    SDL_Flip(screen);
}

void cleanUp(Player* p, Background* bg, Boss* boss) {
    if (p->sprite) SDL_FreeSurface(p->sprite);
    if (bg->img) SDL_FreeSurface(bg->img);
    if (boss->sprite) SDL_FreeSurface(boss->sprite);
    if (boss->attack_sprite) SDL_FreeSurface(boss->attack_sprite);
    if (font) TTF_CloseFont(font);
    if (static_platform_img) SDL_FreeSurface(static_platform_img);
    if (moving_platform_img) SDL_FreeSurface(moving_platform_img);
    if (player_img) SDL_FreeSurface(player_img);
    if (player_attack_img) SDL_FreeSurface(player_attack_img);
    TTF_Quit();
    SDL_Quit();
}
