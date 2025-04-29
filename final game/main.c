#include "game.h"

int main(int argc, char* argv[]) {
    Player player;
    Background background;
    Platform platforms[MAX_PLATFORMS];
    int platformCount = 0;
    Boss boss;
    Enemy enemies[MAX_ENEMIES];
    int enemiesRemaining = MAX_ENEMIES;
    Minimap minimap;
    bool running = true;

    initSDL();
    loadResources();
    initPlayer(&player);
    initBackground(&background);
    initPlatforms(platforms, &platformCount, background.width);
    initBoss(&boss, background.width);
    initEnemies(enemies, background.width);
    initMinimap(&minimap);

    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        handleEvents(&player, &running, &background);
        updateGame(&player, platforms, &platformCount, &background, &boss, enemies, &enemiesRemaining);
        renderGame(&player, &background, platforms, platformCount, &boss, enemies, enemiesRemaining, &minimap);

        if (deltaTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - deltaTime);
        }
    }

    cleanUp(&player, &background, &boss, &minimap);
    return 0;
}
