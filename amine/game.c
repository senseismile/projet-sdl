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
    
    SDL_WM_SetCaption("Endless Platformer", NULL);
    srand((unsigned int)time(NULL));
}

void loadResources() {
    font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    }
    
    // Load platform images
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
    
    p->pos = (SDL_Rect){100, 400, p->sprite->w, p->sprite->h};
    p->vx = p->vy = 0;
    p->ax = p->ay = 0;
    p->isJumping = true;
    p->isOnMovingPlatform = false;
    p->movingPlatformId = -1;
}

void initBackground(Background* bg) {
    bg->img = IMG_Load("background.png");
    if (!bg->img) {
        fprintf(stderr, "Failed to load background.png: %s\n", IMG_GetError());
        exit(1);
    }
    bg->scrollX = 0;
    bg->width = bg->img->w;
}

void initPlatforms(Platform platforms[], int* count) {
    int platformCount = 0;
    int currentX = 0;
    
    // Create endless ground platform
    platforms[platformCount++] = (Platform){
        {0, 550, SCREEN_WIDTH * 20, static_platform_img->h},
        static_platform_img,
        false, true, false, true, 0, 0, 0, 0
    };
    
    // Generate floating platforms
    while (platformCount < MAX_PLATFORMS && currentX < SCREEN_WIDTH * 10) {
        int width = 80 + rand() % 120;
        int spacing = MIN_PLATFORM_SPACING + 
                     rand() % (MAX_PLATFORM_SPACING - MIN_PLATFORM_SPACING);
        int y = 150 + rand() % 350;
        bool moving = (rand() % 100) < MOVING_PLATFORM_CHANCE;
        
        SDL_Surface* platform_img = moving ? moving_platform_img : static_platform_img;
        
        platforms[platformCount] = (Platform){
            {currentX, y, width, platform_img->h},
            platform_img,
            false,
            true,
            moving,
            false,
            0,
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
                    case SDLK_DOWN: p->vy = 8; break; // Slam down
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
    // Apply physics
    p->vx += p->ax;
    p->vy += GRAVITY;
    
    // Limit speed
    if (p->vx > PLAYER_SPEED) p->vx = PLAYER_SPEED;
    if (p->vx < -PLAYER_SPEED) p->vx = -PLAYER_SPEED;
    
    // Update moving platforms
    for (int i = 0; i < *count; i++) {
        if (platforms[i].isMoving && platforms[i].isActive) {
            platforms[i].rect.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
            
            // Reverse direction at bounds
            if (abs(platforms[i].rect.x - platforms[i].originalX) > platforms[i].moveDistance) {
                platforms[i].moveDirection *= -1;
            }
            
            // Move player with platform if standing on it
            if (p->isOnMovingPlatform && p->movingPlatformId == i) {
                p->pos.x += MOVING_PLATFORM_SPEED * platforms[i].moveDirection;
            }
        }
    }
    
    // Update position
    p->pos.x += p->vx;
    p->pos.y += p->vy;
    
    // Screen boundaries
    if (p->pos.x < 0) p->pos.x = 0;
    
    // Background scrolling with parallax
    bg->scrollX += p->vx * BG_SCROLL_RATIO;
    
    // Collision detection
    p->isJumping = true;
    p->isOnMovingPlatform = false;
    for (int i = 0; i < *count; i++) {
        if (!platforms[i].isActive) continue;
        
        Platform* pl = &platforms[i];
        if (p->pos.y + p->pos.h >= pl->rect.y &&
            p->pos.y + p->pos.h <= pl->rect.y + 10 &&
            p->pos.x + p->pos.w > pl->rect.x &&
            p->pos.x < pl->rect.x + pl->rect.w) {
            
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

void renderGame(Player* p, Background* bg, Platform platforms[], int count) {
    // Clear screen with fallback color
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 50, 50, 100));
    
    // Render seamless background
    int bgX = (int)bg->scrollX % bg->width;
    
    // First segment
    SDL_Rect src = {bgX, 0, bg->width - bgX, SCREEN_HEIGHT};
    SDL_Rect dest = {0, 0, bg->width - bgX, SCREEN_HEIGHT};
    SDL_BlitSurface(bg->img, &src, screen, &dest);
    
    // Second segment if needed
    if (bgX > 0) {
        src = (SDL_Rect){0, 0, bgX, SCREEN_HEIGHT};
        dest = (SDL_Rect){bg->width - bgX, 0, bgX, SCREEN_HEIGHT};
        SDL_BlitSurface(bg->img, &src, screen, &dest);
    }
    
    // Render platforms
    for (int i = 0; i < count; i++) {
        if (!platforms[i].isActive) continue;
        
        SDL_Rect r = platforms[i].rect;
        r.x -= (int)(bg->scrollX * 0.9f);
        
        if (r.x + r.w > 0 && r.x < SCREEN_WIDTH) {
            // Create temporary surface for stretched platform
            SDL_Surface* stretched = SDL_CreateRGBSurface(SDL_HWSURFACE, r.w, r.h,
                screen->format->BitsPerPixel,
                screen->format->Rmask,
                screen->format->Gmask,
                screen->format->Bmask,
                screen->format->Amask);
                
            SDL_SoftStretch(platforms[i].image, NULL, stretched, NULL);
            SDL_BlitSurface(stretched, NULL, screen, &r);
            SDL_FreeSurface(stretched);
        }
    }
    
    // Render player
    SDL_Rect playerRect = p->pos;
    playerRect.x -= (int)(bg->scrollX * 0.9f);
    SDL_BlitSurface(p->sprite, NULL, screen, &playerRect);
    
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
