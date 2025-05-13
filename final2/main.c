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
    Pit pits[MAX_PITS];
    int pitCount = 0;
    PitEnemy pitEnemies[MAX_PITS];
    Rock rocks[MAX_ROCKS];
    bool running = true;

    initSDL();
    loadResources();

    bool loadFromSave = false;
    if (argc > 1 && strcmp(argv[1], "--load") == 0) {
        loadFromSave = loadGame(&player, &background, platforms, &platformCount, &boss, enemies, &enemiesRemaining);
        if (!loadFromSave) printf("Failed to load saved game!\n");
    }

    if (!loadFromSave) {
        initPlayer(&player);
        initBackground(&background);
        initPlatforms(platforms, &platformCount, background.width);
        initBoss(&boss, background.width);
        initEnemies(enemies, background.width);
    }

    // Always initialize pits, pit enemies, and rocks
    initPits(pits, &pitCount, background.width);
    initPitEnemies(pitEnemies, pits, pitCount);
    initRocks(rocks, background.width);

    initMinimap(&minimap);

    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        handleEvents(&player, &running, &background, platforms, &platformCount, &boss, enemies, &enemiesRemaining);
        updateGame(&player, platforms, &platformCount, &background, &boss, enemies, &enemiesRemaining, pits, pitCount, pitEnemies, rocks);
        renderGame(&player, &background, platforms, platformCount, &boss, enemies, enemiesRemaining, &minimap, pits, pitCount, pitEnemies, rocks);

        if (deltaTime < FRAME_DELAY) SDL_Delay(FRAME_DELAY - deltaTime);
    }

    cleanUp(&player, &background, &boss, &minimap);
    return 0;
}
