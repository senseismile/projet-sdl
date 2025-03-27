#include "game.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Initialize game systems
    initSDL();
    loadResources();
    
    // Create game objects
    Player player;
    Background bg;
    Platform platforms[MAX_PLATFORMS];
    int platformCount;
    
    // Initialize game state
    initPlayer(&player);
    initBackground(&bg);
    initPlatforms(platforms, &platformCount);

    // Game loop variables
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    Uint32 gameStartTime = SDL_GetTicks();
    
    while (running) {
        // Timing
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Check game duration (90 seconds)
        if (currentTime - gameStartTime >= GAME_DURATION_MS) {
            printf("Game completed!\n");
            running = false;
        }

        // Process input
        handleEvents(&player, &running);
        
        // Update game state
        updateGame(&player, platforms, &platformCount, &bg);
        
        // Render frame
        renderGame(&player, &bg, platforms, platformCount);
        
        // Cap at ~60 FPS
        if (deltaTime < 0.016f) {
            SDL_Delay((Uint32)((0.016f - deltaTime) * 1000));
        }
    }

    // Clean up resources
    cleanUp(&player, &bg);
    return 0;
}
