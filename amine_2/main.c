#include "game.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    initSDL();
    loadResources();
    
    Player player;
    Background bg;
    Platform platforms[MAX_PLATFORMS];
    int platformCount;
    
    initPlayer(&player);
    initBackground(&bg);
    initPlatforms(platforms, &platformCount, bg.width);

    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleEvents(&player, &running);
        updateGame(&player, platforms, &platformCount, &bg);
        renderGame(&player, &bg, platforms, platformCount);
        
        if (deltaTime < 0.016f) {
            SDL_Delay((Uint32)((0.016f - deltaTime) * 1000));
        }
    }

    cleanUp(&player, &bg);
    return 0;
}
