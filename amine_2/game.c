#include "game.h"

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* static_platform_img = NULL;
SDL_Surface* moving_platform_img = NULL;

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
    
    SDL_WM_SetCaption("Platform Game", NULL);
    srand((unsigned int)time(NULL));
}

void loadResources() {
    font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    }
    
    static_platform_img = IMG_Load("platform_static.png");
    moving_platform_img = IMG_Load("platform_moving.png");
    if (!static_platform_img || !moving_platform_img) {
        fprintf(stderr, "Failed to load platform images!\n");
        exit(1);
    }
}

void initPlayer(Player* p) {
    p->sprite = IMG_Load("player1.png");
    if (!p->sprite) {
        fprintf(stderr, "Failed to load player1.png: %s\n", IMG_GetError());
        exit(1);
    }
    
    p->pos = (SDL_Rect){PLAYER_X_POSITION, GROUND_PLATFORM_HEIGHT - p->sprite->h, 
                       p->sprite->w, p->sprite->h};
    p->vx = p->vy = 0;
    p->ax = p->ay = 0;
    p->isJumping = false;
    p->isOnMovingPlatform = false;
    p->movingPlatformId = -1;
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
}

void initPlatforms(Platform platforms[], int* count, int worldWidth) {
    int platformCount = 0;
    int currentX = 0;
    
    // Ground platform (spans entire width with static platform image)
    platforms[platformCount++] = (Platform){
        {0, GROUND_PLATFORM_HEIGHT, worldWidth, PLATFORM_HEIGHT},
        static_platform_img,
        false, true, false, 0, 0, 0
    };
    
    // Generate additional platforms
    while (platformCount < MAX_PLATFORMS && currentX < worldWidth - END_MARGIN) {
        int width = PLATFORM_WIDTH;
        int spacing = MIN_PLATFORM_SPACING + 
                     rand() % (MAX_PLATFORM_SPACING - MIN_PLATFORM_SPACING);
        int y = 150 + rand() % 350;
        bool moving = (rand() % 100) < MOVING_PLATFORM_CHANCE;
        
        SDL_Surface* platform_img = moving ? moving_platform_img : static_platform_img;
        
        platforms[platformCount] = (Platform){
            {currentX, y, width, PLATFORM_HEIGHT},
            platform_img,
            false,
            true,
            moving,
            moving ? (rand() % 2 ? 1 : -1) : 0,
            moving ? (100 + rand() % 200) : 0,
            currentX
        };
        
        platformCount++;
        currentX += width + spacing;
    }
    
    *count = platformCount;
}

void handleEvents(Player* p, bool* running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT: p->ax = -0.5; break;
                    case SDLK_RIGHT: p->ax = 0.5; break;
                    case SDLK_UP:
                        if (!p->isJumping) {
                            p->vy = JUMP_FORCE;
                            p->isJumping = true;
                        }
                        break;
                    case SDLK_ESCAPE: *running = false; break;
                    case SDLK_DOWN: p->vy = 8; break;
                }
                break;
                
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                    case SDLK_RIGHT:
                        p->ax = 0;
                        break;
                }
                break;
        }
    }
}

void updateGame(Player* p, Platform platforms[], int* count, Background* bg) {
    p->vx += p->ax;
    p->vy += GRAVITY;
    
    if (p->vx > PLAYER_SPEED) p->vx = PLAYER_SPEED;
    if (p->vx < -PLAYER_SPEED) p->vx = -PLAYER_SPEED;
    
    // Camera movement with boundary checks
    float newCameraX = bg->cameraX + p->vx;
    if (newCameraX >= 0 && newCameraX + SCREEN_WIDTH <= bg->width) {
        bg->cameraX = newCameraX;
    }
    else if (newCameraX + SCREEN_WIDTH > bg->width) {
        bg->cameraX = bg->width - SCREEN_WIDTH;
    }
    
    p->pos.y += p->vy;
    
    // Update moving platforms
    for (int i = 0; i < *count; i++) {
        if (platforms[i].isMoving && platforms[i].isActive) {
            platforms[i].rect.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
            
            if (abs(platforms[i].rect.x - platforms[i].originalX) > platforms[i].moveDistance) {
                platforms[i].moveDirection *= -1;
            }
        }
    }
    
    // Collision detection
    p->isJumping = true;
    p->isOnMovingPlatform = false;
    for (int i = 0; i < *count; i++) {
        if (!platforms[i].isActive) continue;
        
        Platform* pl = &platforms[i];
        float platformScreenX = pl->rect.x - bg->cameraX;
        
        // Check if player is landing on platform
        if (p->pos.y + p->pos.h >= pl->rect.y &&
            p->pos.y + p->pos.h <= pl->rect.y + 10 &&
            p->pos.x + p->pos.w > platformScreenX &&
            p->pos.x < platformScreenX + pl->rect.w) {
            
            p->pos.y = pl->rect.y - p->pos.h;
            p->vy = 0;
            p->isJumping = false;
            
            if (pl->isMoving) {
                p->isOnMovingPlatform = true;
                p->movingPlatformId = i;
            }
        }
    }
}

void renderTimer(Background* bg) {
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

void renderGame(Player* p, Background* bg, Platform platforms[], int count) {
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 50, 50, 100));
    
    // Render background
    SDL_Rect src = {
        (int)bg->cameraX, 0,
        MIN(SCREEN_WIDTH, bg->width - (int)bg->cameraX),
        SCREEN_HEIGHT
    };
    
    SDL_Rect dest = {0, 0, src.w, src.h};
    SDL_BlitSurface(bg->img, &src, screen, &dest);
    
    // Fill right side if background doesn't cover screen
    if (src.w < SCREEN_WIDTH) {
        SDL_Rect fill = {src.w, 0, SCREEN_WIDTH - src.w, SCREEN_HEIGHT};
        SDL_FillRect(screen, &fill, SDL_MapRGB(screen->format, 50, 50, 100));
    }
    
    // Render platforms
    for (int i = 0; i < count; i++) {
        if (!platforms[i].isActive) continue;
        
        int platformScreenX = platforms[i].rect.x - (int)bg->cameraX;
        
        if (platformScreenX + platforms[i].rect.w > 0 && platformScreenX < SCREEN_WIDTH) {
            SDL_Rect r = {
                platformScreenX,
                platforms[i].rect.y,
                platforms[i].rect.w,
                platforms[i].rect.h
            };
            
            // Tile the platform image if needed
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
    
    // Render player
    SDL_BlitSurface(p->sprite, NULL, screen, &p->pos);
    
    // Render timer
    renderTimer(bg);
    
    SDL_Flip(screen);
}

void cleanUp(Player* p, Background* bg) {
    if (p->sprite) SDL_FreeSurface(p->sprite);
    if (bg->img) SDL_FreeSurface(bg->img);
    if (font) TTF_CloseFont(font);
    if (static_platform_img) SDL_FreeSurface(static_platform_img);
    if (moving_platform_img) SDL_FreeSurface(moving_platform_img);
    TTF_Quit();
    SDL_Quit();
}
