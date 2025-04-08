#include "game.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    initSDL();
    loadResources();

    Player player1;
    Player player2;
    Background bg;
    Platform platforms[MAX_PLATFORMS];
    int platformCount;
    Boss boss;
    Enemy enemies[MAX_ENEMIES];
    int enemiesRemaining = MAX_ENEMIES;

    initPlayer(&player1, false);
    initPlayer(&player2, true);
    initBackground(&bg);
    initPlatforms(platforms, &platformCount, bg.width);
    initBoss(&boss, bg.width);
    initEnemies(enemies, bg.width);

    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    Uint32 frameStart;
    int frameTime;

    while (running) {
        frameStart = SDL_GetTicks();

        handleEvents(&player1, &player2, &running, &bg);
        updateGame(&player1, &player2, platforms, &platformCount, &bg, &boss, enemies, &enemiesRemaining);
        renderGame(&player1, &player2, &bg, platforms, platformCount, &boss, enemies, enemiesRemaining);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    cleanUp(&player1, &player2, &bg, &boss);
    return 0;
}
