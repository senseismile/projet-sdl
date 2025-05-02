#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include "game.h"

int main(int argc, char* argv[]) {
    initSDL();
    ControlConfig controls;
    int character_id;
    loadConfig(&controls, &character_id);
    loadResources(character_id);
    Player player;
    Background bg;
    Platform platforms[MAX_PLATFORMS];
    int platformCount;
    Boss boss;
    Enemy enemies[MAX_ENEMIES];
    int enemiesRemaining = MAX_ENEMIES;
    Minimap minimap;
    bool running = true;
    Uint32 lastFrameTime = SDL_GetTicks();
    initPlayer(&player, character_id);
    initBackground(&bg);
    initPlatforms(platforms, &platformCount, bg.width);
    initBoss(&boss, bg.width);
    initEnemies(enemies, bg.width);
    initMinimap(&minimap);
    Mix_PlayMusic(background_music, -1);
    if (argc > 1 && strcmp(argv[1], "--load") == 0) {
        if (!loadGame(&player, &bg, platforms, &platformCount, &boss, enemies, &enemiesRemaining, character_id)) {
            resetGame(&player, &bg, platforms, &platformCount, &boss, enemies, &enemiesRemaining, &minimap, character_id);
        }
    }
    while (running) {
        if (bg.needsReset) {
            resetGame(&player, &bg, platforms, &platformCount, &boss, enemies, &enemiesRemaining, &minimap, character_id);
        }
        handleEvents(&player, &running, &bg, platforms, &platformCount, &boss, enemies, &enemiesRemaining, &controls);
        updateGame(&player, platforms, &platformCount, &bg, &boss, enemies, &enemiesRemaining);
        renderGame(&player, &bg, platforms, platformCount, &boss, enemies, enemiesRemaining, &minimap);
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastFrameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - (currentTime - lastFrameTime));
        }
        lastFrameTime = currentTime;
    }
    cleanUp(&player, &bg, &boss, &minimap);
    return 0;
}
