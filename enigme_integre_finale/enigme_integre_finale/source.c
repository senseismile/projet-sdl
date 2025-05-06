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


typedef struct {
    SDL_Surface *images[4]; 
    int currentEmotion; 
    int frame; 
    int frameCount; 
    Uint32 lastFrameTime; 
    Uint32 frameDuration; 
} Animation;


SDL_Surface* scaleSurface(SDL_Surface *src, int newWidth, int newHeight) {
    SDL_Surface *dest = SDL_CreateRGBSurface(src->flags, newWidth, newHeight, 
                                            src->format->BitsPerPixel,
                                            src->format->Rmask, src->format->Gmask,
                                            src->format->Bmask, src->format->Amask);
    if (!dest) {
        printf("Error creating scaled surface: %s\n", SDL_GetError());
        return NULL;
    }


    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = (x * src->w) / newWidth;
            int srcY = (y * src->h) / newHeight;
            Uint32 pixel = ((Uint32*)src->pixels)[srcY * src->w + srcX];
            ((Uint32*)dest->pixels)[y * newWidth + x] = pixel;
        }
    }
    return dest;
}

void initAnimation(Animation *anim) {
    SDL_Surface *tempImages[4];
    tempImages[0] = loadImage("angry4.png");
    tempImages[1] = loadImage("sad4.png");
    tempImages[2] = loadImage("confused4.png");
    tempImages[3] = loadImage("happy4.png");

    for (int i = 0; i < 4; i++) {
        if (tempImages[i]) {
            anim->images[i] = scaleSurface(tempImages[i], 100, 100);
            SDL_FreeSurface(tempImages[i]);
            if (!anim->images[i]) {
                printf("Failed to scale image %d\n", i);
            }
        } else {
            anim->images[i] = NULL;
        }
    }

    anim->currentEmotion = 3; 
    anim->frame = 0;
    anim->frameCount = 1; 
    anim->lastFrameTime = SDL_GetTicks();
    anim->frameDuration = 500; 
}

void freeAnimation(Animation *anim) {
    for (int i = 0; i < 4; i++) {
        if (anim->images[i]) SDL_FreeSurface(anim->images[i]);
    }
}

void updateAnimation(Animation *anim, float progress, int selectedAnswer, int correctAnswer, int showingAnswer) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - anim->lastFrameTime >= anim->frameDuration) {
        anim->frame = (anim->frame + 1) % anim->frameCount;
        anim->lastFrameTime = currentTime;
    }

    if (showingAnswer && selectedAnswer != -1) {
        if (selectedAnswer == correctAnswer) {
            anim->currentEmotion = 3; 
        } else {
            anim->currentEmotion = 0; 
        }
    } else if (player.lives <= 1) {
        anim->currentEmotion = 1; 
    } else if (progress < 0.2) {
        anim->currentEmotion = 2; 
    } else {
        anim->currentEmotion = 3; 
    }
}

void renderAnimation(SDL_Surface *screen, Animation *anim) {
    if (anim->images[anim->currentEmotion]) {
        SDL_Rect animRect = {50, WINDOW_HEIGHT - 150, 100, 100}; 
        SDL_BlitSurface(anim->images[anim->currentEmotion], NULL, screen, &animRect);
    }
}

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
    Animation anim;
    initAnimation(&anim);
    anim.currentEmotion = 1; 

    if (!gameOverBg) {
        printf("Failed to load game over background\n");
        freeAnimation(&anim);
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
    
    renderAnimation(screen, &anim);
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
    freeAnimation(&anim);
}

void showWinScreen(SDL_Surface *screen, TTF_Font *font) {
    SDL_Surface *winBg = loadImage("win.png");
    Animation anim;
    initAnimation(&anim);
    anim.currentEmotion = 3; 

    if (!winBg) {
        printf("Failed to load win background\n");
        freeAnimation(&anim);
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
    
    renderAnimation(screen, &anim);
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
    freeAnimation(&anim);
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
    SDL_Surface *bg = loadImage("2D.png");
    SDL_Surface *questionButton = loadImage("button.png");
    SDL_Surface *answerButtons[3] = {NULL};
    SDL_Surface *nextButton = loadImage("next.png");
    answerButtons[0] = loadImage("a.png");
    answerButtons[1] = loadImage("B.png");
    answerButtons[2] = loadImage("c.png");
    
    Animation anim;
    initAnimation(&anim);

    if (!bg || !questionButton || !answerButtons[0] || !answerButtons[1] || !answerButtons[2] || !nextButton) {
        printf("Failed to load quiz game assets\n");
        if (bg) SDL_FreeSurface(bg);
        if (questionButton) SDL_FreeSurface(questionButton);
        for (int i = 0; i < 3; i++) if (answerButtons[i]) SDL_FreeSurface(answerButtons[i]);
        if (nextButton) SDL_FreeSurface(nextButton);
        freeAnimation(&anim);
        currentState = MENU_STATE;
        return;
    }

    SDL_Rect questionRect = {(WINDOW_WIDTH-BUTTON_WIDTH)/2, QUESTION_Y, BUTTON_WIDTH, BUTTON_HEIGHT};
    int totalWidth = 3*BUTTON_WIDTH + 2*BUTTON_SPACING;
    int startX = (WINDOW_WIDTH - totalWidth)/2;
    SDL_Rect answerRects[3];
    for (int i = 0; i < 3; i++) {
        answerRects[i] = (SDL_Rect){startX + i*(BUTTON_WIDTH+BUTTON_SPACING), ANSWERS_Y, BUTTON_WIDTH, BUTTON_HEIGHT};
    }

    SDL_Rect nextButtonRect = {
        (WINDOW_WIDTH - NEXT_BUTTON_WIDTH)/2,
        ANSWERS_Y + BUTTON_HEIGHT + 50,
        NEXT_BUTTON_WIDTH,
        NEXT_BUTTON_HEIGHT
    };

    SDL_Color black = {0,0,0}, white = {255,255,255};
    SDL_Color green = {0,255,0}, red = {255,0,0};
    SDL_Color gold = {255, 215, 0};

    srand(time(NULL));
    int currentQuestion = rand() % questionCount;
    int selectedAnswer = -1;
    int showingAnswer = 0;
    int running = 1;
    int showNextButton = 0;
   
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime;
    float timeLeft;
    float progress;

    while (running) {
        currentTime = SDL_GetTicks();
        timeLeft = TIMER_DURATION - (currentTime - startTime)/500.0f;
        progress = timeLeft/TIMER_DURATION;
        
        if (timeLeft <= 0 && !showingAnswer) {
            showingAnswer = 1;
            selectedAnswer = -1;
            shouldBeep = 0;
            player.lives--;
            player.consecutiveCorrect = 0;
            showNextButton = 1;
            
            if (player.lives <= 0) {
                currentState = GAMEOVER_STATE;
                running = 0;
            }
        }
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                currentState = MENU_STATE;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                    currentState = MENU_STATE;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                Point mouse = {event.button.x, event.button.y};
                if (!showingAnswer) {
                    for (int i = 0; i < 3; i++) {
                        if (pointInRect(mouse, answerRects[i])) {
                            selectedAnswer = i;
                            showingAnswer = 1;
                            shouldBeep = 0;
                            showNextButton = 1;
                            
                            if (selectedAnswer == questions[currentQuestion].correctAnswer) {
                                int timeBonus = (int)(timeLeft * 2);
                                player.score += 100 * player.level + timeBonus;
                                player.consecutiveCorrect++;
                                player.questionsSurvived++;
                                
                                if (player.consecutiveCorrect >= 3) {
                                    player.score += 50 * player.level;
                                }
                                
                                if (player.score >= player.level * 500) {
                                    player.level++;
                                    player.lives++;
                                }

                                if (player.questionsSurvived >= QUESTIONS_TO_WIN) {
                                    currentState = WIN_STATE;
                                    running = 0;
                                    break;
                                }
                            } else {
                                player.score -= 50 * player.level;
                                if (player.score < 0) player.score = 0;
                                player.lives--;
                                player.consecutiveCorrect = 0;
                                
                                if (player.lives <= 0) {
                                    currentState = GAMEOVER_STATE;
                                    running = 0;
                                }
                            }
                            break;
                        }
                    }
                } else if (showNextButton && pointInRect(mouse, nextButtonRect)) {
                    int channel = Mix_PlayChannel(-1, hoverSound, 0);
                    Uint32 soundStart = SDL_GetTicks();
                    while (SDL_GetTicks() - soundStart < 1000) {
                        SDL_PumpEvents();
                    }
                    Mix_HaltChannel(channel);
                    
                    currentQuestion = rand() % questionCount;
                    selectedAnswer = -1;
                    showingAnswer = 0;
                    showNextButton = 0;
                    
                    startTime = SDL_GetTicks();
                    shouldBeep = 1;
                    lastBeepTime = SDL_GetTicks();
                }
            }
        }
        
        updateBeepSound(progress);
        updateAnimation(&anim, progress, selectedAnswer, questions[currentQuestion].correctAnswer, showingAnswer);

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0));
        SDL_BlitSurface(bg, NULL, screen, NULL);
        
        if (!showingAnswer) {
            drawTimer(screen, progress);
        }
        
        SDL_BlitSurface(questionButton, NULL, screen, &questionRect);
        renderCenteredText(screen, font, questions[currentQuestion].question, questionRect, black);
        
        for (int i = 0; i < 3; i++) {
            SDL_BlitSurface(answerButtons[i], NULL, screen, &answerRects[i]);
            
            SDL_Color textColor = black;
            if (showingAnswer) {
                if (i == questions[currentQuestion].correctAnswer) {
                    textColor = green;
                    renderCenteredText(screen, font, "", 
                        (SDL_Rect){answerRects[i].x + BUTTON_WIDTH - 30, 
                                  answerRects[i].y + 10, 20, 20}, green);
                } else if (i == selectedAnswer) {
                    textColor = red;
                    renderCenteredText(screen, font, "", 
                        (SDL_Rect){answerRects[i].x + BUTTON_WIDTH - 30, 
                                  answerRects[i].y + 10, 20, 20}, red);
                }
            }
            renderCenteredText(screen, font, questions[currentQuestion].answers[i], answerRects[i], textColor);
        }

        if (showNextButton && nextButton) {
            SDL_BlitSurface(nextButton, NULL, screen, &nextButtonRect);
            renderCenteredText(screen, font, "Next", nextButtonRect, gold);
        }

        renderPlayerStats(screen, font);
        renderCenteredText(screen, font, "Exiled Gods Quiz", (SDL_Rect){0,20,WINDOW_WIDTH,40}, white);
        renderAnimation(screen, &anim);
        
        SDL_Flip(screen);
        SDL_Delay(10);
    }

    SDL_FreeSurface(bg);
    SDL_FreeSurface(questionButton);
    for (int i = 0; i < 3; i++) if (answerButtons[i]) SDL_FreeSurface(answerButtons[i]);
    if (nextButton) SDL_FreeSurface(nextButton);
    freeAnimation(&anim);
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
