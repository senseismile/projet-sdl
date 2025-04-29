#include "menu.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    initSDL();
    loadResources();

    Player player;
    Background background;
    Platform platforms[MAX_PLATFORMS];
    int platformCount = 0;
    Boss boss;
    Enemy enemies[MAX_ENEMIES];
    int enemiesRemaining = MAX_ENEMIES;
    Minimap minimap;

    initPlayer(&player);
    initBackground(&background);
    initPlatforms(platforms, &platformCount, background.width);
    initBoss(&boss, background.width);
    initEnemies(enemies, background.width);
    initMinimap(&minimap);

    int result;
    do {
        result = mainMenu(&player, &background, platforms, &platformCount, &boss, enemies, &enemiesRemaining, &minimap);
    } while (result == 0);

    cleanUp(&player, &background, &boss, &minimap);
    return 0;
}
