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

    bool loadFromSave = false;
    if (argc > 1 && strcmp(argv[1], "--load") == 0) {
        loadFromSave = loadGame(&player, &background, platforms, &platformCount, &boss, enemies, &enemiesRemaining);
        if (!loadFromSave) {
            printf("Failed to load saved game!\n");
        }
    }

    if (!loadFromSave) {
        initPlayer(&player);
        initBackground(&background);
        initPlatforms(platforms, &platformCount, background.width);
        initBoss(&boss, background.width);
        initEnemies(enemies, background.width);
    }

    initMinimap(&minimap);

    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        handleEvents(&player, &running, &background, platforms, &platformCount, &boss, enemies, &enemiesRemaining);
        updateGame(&player, platforms, &platformCount, &background, &boss, enemies, &enemiesRemaining);
        renderGame(&player, &background, platforms, platformCount, &boss, enemies, enemiesRemaining, &minimap);

        if (deltaTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - deltaTime);
        }
    }

    cleanUp(&player, &background, &boss, &minimap);
    return 0;
}
