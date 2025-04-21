#include "game.h"

int main(int argc, char* argv[]) {
    Player player1, player2;
    Background background;
    Platform platforms[MAX_PLATFORMS];
    int platformCount = 0;
    Boss boss;
    Enemy enemies[MAX_ENEMIES];
    int enemiesRemaining = MAX_ENEMIES;
    bool running = true;
    
    initSDL();
    loadResources();
    initPlayer(&player1, false);
    initPlayer(&player2, true);
    initBackground(&background);
    initPlatforms(platforms, &platformCount, background.width);
    initBoss(&boss, background.width);
    initEnemies(enemies, background.width);
    
    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        handleEvents(&player1, &player2, &running, &background);
        updateGame(&player1, &player2, platforms, &platformCount, &background, &boss, enemies, &enemiesRemaining);
        renderGame(&player1, &player2, &background, platforms, platformCount, &boss, enemies, enemiesRemaining);
        
        if (deltaTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - deltaTime);
        }
    }
    
    cleanUp(&player1, &player2, &background, &boss);
    return 0;
}
