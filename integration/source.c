#include "header.h"

Mix_Chunk *beepSound = NULL;
Mix_Chunk *hoverSound = NULL;
Mix_Music *enigmeMusic = NULL;
Mix_Music *winMusic = NULL;
Uint32 lastBeepTime = 0;
Uint32 beepInterval = BEEP_INTERVAL_START;
int beepChannel = -1;
int shouldBeep = 1;
PlayerStats player = {0, 3, 1, 0, 0};
GameState currentState = MENU_STATE;

int pointInRect(Point p, SDL_Rect r) {
    return (p.x >= r.x) && (p.x < r.x + r.w) && (p.y >= r.y) && (p.y < r.y + r.h);
}

SDL_Surface* loadImage(const char *path) {
    SDL_Surface *img = IMG_Load(path);
    if (!img) printf("Error loading %s: %s\n", path, IMG_GetError());
    return img;
}

void renderCenteredText(SDL_Surface *screen, TTF_Font *font, const char *text, 
                       SDL_Rect rect, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, color);
    if (textSurface) {
        SDL_Rect dstRect = {
            rect.x + (rect.w - textSurface->w)/2,
            rect.y + (rect.h - textSurface->h)/2,
            textSurface->w,
            textSurface->h
        };
        SDL_BlitSurface(textSurface, NULL, screen, &dstRect);
        SDL_FreeSurface(textSurface);
    }
}

void updateBeepSound(float progress) {
    if (!shouldBeep || progress <= 0) {
        if (beepChannel != -1) {
            Mix_HaltChannel(beepChannel);
            beepChannel = -1;
        }
        return;
    }

    Uint32 currentTime = SDL_GetTicks();
    beepInterval = BEEP_INTERVAL_MIN + 
                  (Uint32)((BEEP_INTERVAL_START - BEEP_INTERVAL_MIN) * progress);
    
    if (currentTime - lastBeepTime > beepInterval) {
        if (beepSound) {
            if (beepChannel != -1) {
                Mix_HaltChannel(beepChannel);
            }
            beepChannel = Mix_PlayChannel(-1, beepSound, 0);
        }
        lastBeepTime = currentTime;
    }
}

void drawTimer(SDL_Surface *screen, float progress) {
    SDL_Rect bgRect = {(WINDOW_WIDTH-TIMER_WIDTH)/2, TIMER_Y, TIMER_WIDTH, TIMER_HEIGHT};
    SDL_FillRect(screen, &bgRect, SDL_MapRGB(screen->format, 100, 100, 100));
    
    int width = (int)(TIMER_WIDTH * progress);
    Uint8 r = progress > 0.5 ? (Uint8)(255 * (1-progress)*2) : 255;
    Uint8 g = progress < 0.5 ? (Uint8)(255 * progress*2) : 255;
    
    SDL_Rect timerRect = {(WINDOW_WIDTH-TIMER_WIDTH)/2, TIMER_Y, width, TIMER_HEIGHT};
    SDL_FillRect(screen, &timerRect, SDL_MapRGB(screen->format, r, g, 0));
}

void renderPlayerStats(SDL_Surface *screen, TTF_Font *font) {
    char statsText[100];
    SDL_Color white = {255, 255, 255};
    SDL_Color gold = {255, 215, 0};
    
    sprintf(statsText, "Score: %d", player.score);
    renderCenteredText(screen, font, statsText, (SDL_Rect){WINDOW_WIDTH-200, 20, 180, 30}, gold);
    
    sprintf(statsText, "Lives: %d", player.lives);
    renderCenteredText(screen, font, statsText, (SDL_Rect){WINDOW_WIDTH-200, 60, 180, 30}, white);
    
    sprintf(statsText, "Level: %d", player.level);
    renderCenteredText(screen, font, statsText, (SDL_Rect){WINDOW_WIDTH-200, 100, 180, 30}, white);
    
    sprintf(statsText, "Survived: %d/%d", player.questionsSurvived, QUESTIONS_TO_WIN);
    renderCenteredText(screen, font, statsText, (SDL_Rect){WINDOW_WIDTH-200, 140, 180, 30}, white);
}

int loadQuestions(const char *filename, QuizQuestion *questions) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open %s\n", filename);
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int count = 0;
    
    while (count < MAX_QUESTIONS && fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        strncpy(questions[count].question, line, MAX_LINE_LENGTH);
        
        for (int i = 0; i < 3; i++) {
            if (!fgets(line, sizeof(line), file)) break;
            line[strcspn(line, "\n")] = 0;
            strncpy(questions[count].answers[i], line, MAX_LINE_LENGTH);
        }
        
        if (fgets(line, sizeof(line), file)) {
            questions[count].correctAnswer = atoi(line);
        }
        
        count++;
    }
    
    fclose(file);
    return count;
}

void showGameOver(SDL_Surface *screen, TTF_Font *font) {
    SDL_Surface *gameOverBg = loadImage("over.png");
    if (!gameOverBg) {
        printf("Failed to load game over background\n");
        return;
    }

    SDL_BlitSurface(gameOverBg, NULL, screen, NULL);
    
    char gameOverText[100];
    sprintf(gameOverText, "Final Score: %d", player.score);
    
    SDL_Color red = {255, 0, 0}; 
    SDL_Color white = {255, 255, 255};
    
    renderCenteredText(screen, font, "Game Over!", 
                     (SDL_Rect){0, WINDOW_HEIGHT/2 - 100, WINDOW_WIDTH, 50}, red);
    renderCenteredText(screen, font, gameOverText, 
                     (SDL_Rect){0, WINDOW_HEIGHT/2 - 30, WINDOW_WIDTH, 50}, white);
    renderCenteredText(screen, font, "Click to return to menu", 
                     (SDL_Rect){0, WINDOW_HEIGHT/2 + 40, WINDOW_WIDTH, 50}, white);
    
    SDL_Flip(screen);
    
    SDL_Event event;
    int waiting = 1;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_MOUSEBUTTONDOWN) {
                waiting = 0;
            }
        }
        SDL_Delay(10);
    }

    SDL_FreeSurface(gameOverBg);
}

void showWinScreen(SDL_Surface *screen, TTF_Font *font) {
    SDL_Surface *winBg = loadImage("win.png");
    if (!winBg) {
        printf("Failed to load win background\n");
        return;
    }

    if (enigmeMusic) Mix_HaltMusic();
    if (winMusic) Mix_PlayMusic(winMusic, 0);

    SDL_BlitSurface(winBg, NULL, screen, NULL);
    
    char winText[100];
    sprintf(winText, "Final Score: %d", player.score);
    
    SDL_Color gold = {255, 215, 0};
    SDL_Color white = {255, 255, 255};
    
    renderCenteredText(screen, font, "You Win!", 
                     (SDL_Rect){0, WINDOW_HEIGHT/2 - 100, WINDOW_WIDTH, 50}, gold);
    renderCenteredText(screen, font, winText, 
                     (SDL_Rect){0, WINDOW_HEIGHT/2 - 30, WINDOW_WIDTH, 50}, white);
    renderCenteredText(screen, font, "Click to return to menu", 
                     (SDL_Rect){0, WINDOW_HEIGHT/2 + 40, WINDOW_WIDTH, 50}, white);
    
    SDL_Flip(screen);
    
    SDL_Event event;
    int waiting = 1;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_MOUSEBUTTONDOWN) {
                waiting = 0;
            }
        }
        SDL_Delay(10);
    }

    if (winMusic) Mix_HaltMusic();
    if (enigmeMusic) Mix_PlayMusic(enigmeMusic, -1);

    SDL_FreeSurface(winBg);
}

void initializeGame() {
    player.score = 0;
    player.lives = 3;
    player.level = 1;
    player.consecutiveCorrect = 0;
    player.questionsSurvived = 0;
    currentState = MENU_STATE;
    lastBeepTime = 0;
    beepInterval = BEEP_INTERVAL_START;
    beepChannel = -1;
    shouldBeep = 1;
}

void runQuizGame(SDL_Surface *screen, TTF_Font *font, QuizQuestion *questions, int questionCount) {
    SDL_Surface *background = loadImage("2D.png");
    SDL_Surface *button = loadImage("button.png");
    SDL_Surface *buttonHover = loadImage("button_hover.png");
    
    if (!background || !button || !buttonHover) {
        printf("Failed to load quiz game assets\n");
        if (background) SDL_FreeSurface(background);
        if (button) SDL_FreeSurface(button);
        if (buttonHover) SDL_FreeSurface(buttonHover);
        currentState = MENU_STATE;
        return;
    }

    int currentQuestion = 0;
    int timeLeft = TIMER_DURATION;
    Uint32 startTime = SDL_GetTicks();
    SDL_Event event;
    int running = 1;
    Point mouse;
    int hoveredButton = -1;

    while (running && currentQuestion < questionCount) {
        SDL_BlitSurface(background, NULL, screen, NULL);

        SDL_Color white = {255, 255, 255};
        renderCenteredText(screen, font, questions[currentQuestion].question,
                          (SDL_Rect){0, QUESTION_Y, WINDOW_WIDTH, 50}, white);

        SDL_GetMouseState(&mouse.x, &mouse.y);
        hoveredButton = -1;
        for (int i = 0; i < 3; i++) {
            SDL_Rect answerRect = {
                (WINDOW_WIDTH - BUTTON_WIDTH) / 2,
                ANSWERS_Y + i * (BUTTON_HEIGHT + BUTTON_SPACING),
                BUTTON_WIDTH, BUTTON_HEIGHT
            };
            if (pointInRect(mouse, answerRect)) {
                hoveredButton = i;
                SDL_BlitSurface(buttonHover, NULL, screen, &answerRect);
            } else {
                SDL_BlitSurface(button, NULL, screen, &answerRect);
            }
            renderCenteredText(screen, font, questions[currentQuestion].answers[i],
                              answerRect, white);
        }

        float progress = (float)(TIMER_DURATION - (SDL_GetTicks() - startTime) / 1000.0) / TIMER_DURATION;
        if (progress < 0) {
            player.lives--;
            player.consecutiveCorrect = 0;
            if (player.lives <= 0) {
                currentState = GAMEOVER_STATE;
                running = 0;
            }
            currentQuestion++;
            startTime = SDL_GetTicks();
            progress = 1.0;
        }
        drawTimer(screen, progress);
        updateBeepSound(progress);

        renderPlayerStats(screen, font);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                currentState = MENU_STATE;
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                mouse.x = event.button.x;
                mouse.y = event.button.y;
                for (int i = 0; i < 3; i++) {
                    SDL_Rect answerRect = {
                        (WINDOW_WIDTH - BUTTON_WIDTH) / 2,
                        ANSWERS_Y + i * (BUTTON_HEIGHT + BUTTON_SPACING),
                        BUTTON_WIDTH, BUTTON_HEIGHT
                    };
                    if (pointInRect(mouse, answerRect)) {
                        if (i == questions[currentQuestion].correctAnswer) {
                            player.score += 10;
                            player.consecutiveCorrect++;
                            player.questionsSurvived++;
                            if (player.questionsSurvived >= QUESTIONS_TO_WIN) {
                                currentState = WIN_STATE;
                                running = 0;
                            }
                        } else {
                            player.lives--;
                            player.consecutiveCorrect = 0;
                            if (player.lives <= 0) {
                                currentState = GAMEOVER_STATE;
                                running = 0;
                            }
                        }
                        currentQuestion++;
                        startTime = SDL_GetTicks();
                        break;
                    }
                }
            }
        }

        SDL_Flip(screen);
        SDL_Delay(10);
    }

    SDL_FreeSurface(background);
    SDL_FreeSurface(button);
    SDL_FreeSurface(buttonHover);
}

void cleanupGame() {
    if (beepSound) Mix_FreeChunk(beepSound);
    if (hoverSound) Mix_FreeChunk(hoverSound);
    if (enigmeMusic) Mix_FreeMusic(enigmeMusic);
    if (winMusic) Mix_FreeMusic(winMusic);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
