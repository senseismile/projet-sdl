#include "game.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    initSDL();
    loadResources();

    Player player;
    Background bg;
    Platform platforms[MAX_PLATFORMS];
    int platformCount;
    Boss boss;
    Enemy enemies[MAX_ENEMIES];
    int enemiesRemaining = MAX_ENEMIES;

    initPlayer(&player);
    initBackground(&bg);
    initPlatforms(platforms, &platformCount, bg.width);
    initBoss(&boss, bg.width);
    initEnemies(enemies, bg.width);

    bool running = true;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleEvents(&player, &running, &bg);
        updateGame(&player, platforms, &platformCount, &bg, &boss, enemies, &enemiesRemaining);
        renderGame(&player, &bg, platforms, platformCount, &boss, enemies, enemiesRemaining);

        if (deltaTime < 0.016f) {
            SDL_Delay((Uint32)((0.016f - deltaTime) * 1000));
        }
    }

    cleanUp(&player, &bg, &boss);
    return 0;
}
