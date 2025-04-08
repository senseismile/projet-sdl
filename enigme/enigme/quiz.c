#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define WINDOW_WIDTH 1297
#define WINDOW_HEIGHT 749
#define MAX_QUESTIONS 100
#define MAX_LINE_LENGTH 256
#define BUTTON_WIDTH 400
#define BUTTON_HEIGHT 80
#define BUTTON_SPACING 20
#define QUESTION_Y 150
#define ANSWERS_Y 300
#define TIMER_HEIGHT 20
#define TIMER_WIDTH 500
#define TIMER_Y 50  // Changed from 80 to move timer up
#define TIMER_RADIUS 50
#define TIMER_CENTER_X (WINDOW_WIDTH/2)
#define TIMER_CENTER_Y (TIMER_Y + TIMER_RADIUS)
#define TIMER_DURATION 30
#define BEEP_INTERVAL_START 2000
#define BEEP_INTERVAL_MIN 200
#define NEXT_BUTTON_WIDTH 200
#define NEXT_BUTTON_HEIGHT 60
#define QUESTIONS_TO_WIN 3

typedef enum {
    MENU_STATE,
    GAME_STATE,
    GAMEOVER_STATE,
    WIN_STATE
} GameState;

typedef struct {
    int x, y;
} Point;

typedef struct {
    char question[MAX_LINE_LENGTH];
    char answers[3][MAX_LINE_LENGTH];
    int correctAnswer;
} QuizQuestion;

typedef struct {
    int score;
    int lives;
    int level;
    int consecutiveCorrect;
    int questionsSurvived;
} PlayerStats;

// Global variables
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
    // Draw the clock background (outer circle)
    int outerRadius = TIMER_RADIUS;
    int innerRadius = TIMER_RADIUS - 10;
    
    // Draw the outer circle (background)
    for (int y = -outerRadius; y <= outerRadius; y++) {
        for (int x = -outerRadius; x <= outerRadius; x++) {
            if (x*x + y*y <= outerRadius*outerRadius) {
                SDL_Rect pixel = {
                    TIMER_CENTER_X + x,
                    TIMER_CENTER_Y + y,
                    1, 1
                };
                SDL_FillRect(screen, &pixel, SDL_MapRGB(screen->format, 100, 100, 100));
            }
        }
    }
    
    // Draw the inner circle (progress)
    float angle = 2 * M_PI * progress;
    Uint8 r = progress > 0.5 ? (Uint8)(255 * (1-progress)*2) : 255;
    Uint8 g = progress < 0.5 ? (Uint8)(255 * progress*2) : 255;
    Uint32 color = SDL_MapRGB(screen->format, r, g, 0);
    
    for (int y = -innerRadius; y <= innerRadius; y++) {
        for (int x = -innerRadius; x <= innerRadius; x++) {
            // Check if pixel is within inner circle
            if (x*x + y*y <= innerRadius*innerRadius) {
                // Calculate angle of this pixel
                float pixelAngle = atan2(y, x);
                // Normalize angle to 0-2π
                if (pixelAngle < 0) pixelAngle += 2 * M_PI;
                // Draw if angle is less than progress angle
                if (pixelAngle <= angle) {
                    SDL_Rect pixel = {
                        TIMER_CENTER_X + x,
                        TIMER_CENTER_Y + y,
                        1, 1
                    };
                    SDL_FillRect(screen, &pixel, color);
                }
            }
        }
    }
    
    // Draw clock marks (optional)
    for (int i = 0; i < 12; i++) {
        float markAngle = i * (2 * M_PI / 12);
        int x1 = TIMER_CENTER_X + (innerRadius-5) * cos(markAngle);
        int y1 = TIMER_CENTER_Y + (innerRadius-5) * sin(markAngle);
        int x2 = TIMER_CENTER_X + innerRadius * cos(markAngle);
        int y2 = TIMER_CENTER_Y + innerRadius * sin(markAngle);
        
        // Simple line drawing
        for (int j = 0; j <= 5; j++) {
            float t = j / 5.0f;
            int x = x1 + (x2 - x1) * t;
            int y = y1 + (y2 - y1) * t;
            SDL_Rect pixel = {x, y, 1, 1};
            SDL_FillRect(screen, &pixel, SDL_MapRGB(screen->format, 255, 255, 255));
        }
    }
    
    // Draw clock hands (optional)
    // Hour hand
    float hourAngle = 2 * M_PI * progress;
    int hourLength = innerRadius * 0.6;
    int hourX = TIMER_CENTER_X + hourLength * sin(hourAngle);
    int hourY = TIMER_CENTER_Y - hourLength * cos(hourAngle);
    
    // Simple line drawing for hour hand
    for (int j = 0; j <= hourLength; j++) {
        float t = j / (float)hourLength;
        int x = TIMER_CENTER_X + (hourX - TIMER_CENTER_X) * t;
        int y = TIMER_CENTER_Y + (hourY - TIMER_CENTER_Y) * t;
        SDL_Rect pixel = {x, y, 2, 2};  // Thicker hand
        SDL_FillRect(screen, &pixel, SDL_MapRGB(screen->format, 0, 0, 0));
    }
    
    // Center dot
    SDL_Rect centerDot = {TIMER_CENTER_X-2, TIMER_CENTER_Y-2, 4, 4};
    SDL_FillRect(screen, &centerDot, SDL_MapRGB(screen->format, 0, 0, 0));
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

void runQuizGame(SDL_Surface *screen, TTF_Font *font, QuizQuestion *questions, int questionCount) {
    SDL_Surface *bg = loadImage("2D.png");
    SDL_Surface *questionButton = loadImage("button.png");
    SDL_Surface *answerButtons[3] = {NULL};
    SDL_Surface *nextButton = loadImage("next.png");
    answerButtons[0] = loadImage("a.png");
    answerButtons[1] = loadImage("b.png");
    answerButtons[2] = loadImage("c.png");
    
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
        timeLeft = TIMER_DURATION - (currentTime - startTime)/1000.0f;
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
                    renderCenteredText(screen, font, "✓", 
                        (SDL_Rect){answerRects[i].x + BUTTON_WIDTH - 30, 
                                  answerRects[i].y + 10, 20, 20}, green);
                } else if (i == selectedAnswer) {
                    textColor = red;
                    renderCenteredText(screen, font, "✗", 
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
        
        SDL_Flip(screen);
        SDL_Delay(10);
    }

    SDL_FreeSurface(bg);
    SDL_FreeSurface(questionButton);
    for (int i = 0; i < 3; i++) SDL_FreeSurface(answerButtons[i]);
    if (nextButton) SDL_FreeSurface(nextButton);
}

int main() {
    SDL_Surface *screen = NULL;
    TTF_Font *font = NULL;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_SWSURFACE);
    if (!screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return 1;
    }
    
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return 1;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return 1;
    }
    
    Mix_AllocateChannels(16);
    beepSound = Mix_LoadWAV("beep.wav");
    hoverSound = Mix_LoadWAV("hover.wav");
    enigmeMusic = Mix_LoadMUS("enigme.wav");
    winMusic = Mix_LoadMUS("win.wav");
    
    if (!beepSound || !hoverSound) {
        printf("Warning: Could not load sound effects\n");
    }
    if (!enigmeMusic) {
        printf("Warning: Could not load enigme music\n");
    }
    if (!winMusic) {
        printf("Warning: Could not load win music\n");
    }
    
    font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        return 1;
    }

    QuizQuestion questions[MAX_QUESTIONS];
    int questionCount = loadQuestions("fichier.txt", questions);
    if (questionCount == 0) {
        return 1;
    }

    SDL_Surface *menuBg = loadImage("2D.png");
    SDL_Surface *quizButton = loadImage("quiz.png");
    SDL_Surface *quizButtonHover = loadImage("quiz2.png");
    SDL_Surface *puzzleButton = loadImage("puzzle.png");
    SDL_Surface *puzzleButtonHover = loadImage("puzzle2.png");
    
    if (!menuBg || !quizButton || !quizButtonHover || !puzzleButton || !puzzleButtonHover) {
        printf("Failed to load menu assets\n");
        return 1;
    }

    SDL_Rect quizButtonRect = {
        (WINDOW_WIDTH - quizButton->w) / 2,
        (WINDOW_HEIGHT/2 - quizButton->h - BUTTON_SPACING/2),
        quizButton->w,
        quizButton->h
    };
    
    SDL_Rect puzzleButtonRect = {
        (WINDOW_WIDTH - puzzleButton->w) / 2,
        (WINDOW_HEIGHT/2 + BUTTON_SPACING/2),
        puzzleButton->w,
        puzzleButton->h
    };

    if (enigmeMusic) {
        Mix_PlayMusic(enigmeMusic, -1);
    }

    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && currentState == MENU_STATE) {
                Point mouse = {event.button.x, event.button.y};
                if (pointInRect(mouse, quizButtonRect)) {
                    Mix_PlayChannel(-1, hoverSound, 0);
                    if (enigmeMusic) Mix_HaltMusic();
                    currentState = GAME_STATE;
                    player = (PlayerStats){0, 3, 1, 0, 0};
                }
                else if (pointInRect(mouse, puzzleButtonRect)) {
                    Mix_PlayChannel(-1, hoverSound, 0);
                }
            }
        }

        if (currentState == MENU_STATE) {
            Point mouse;
            SDL_GetMouseState(&mouse.x, &mouse.y);
            int isHoveringQuiz = pointInRect(mouse, quizButtonRect);
            int isHoveringPuzzle = pointInRect(mouse, puzzleButtonRect);

            SDL_BlitSurface(menuBg, NULL, screen, NULL);
            
            SDL_BlitSurface(isHoveringQuiz ? quizButtonHover : quizButton, NULL, screen, &quizButtonRect);
            SDL_BlitSurface(isHoveringPuzzle ? puzzleButtonHover : puzzleButton, NULL, screen, &puzzleButtonRect);
            
            SDL_Flip(screen);
        }
        else if (currentState == GAME_STATE) {
            runQuizGame(screen, font, questions, questionCount);
            if (currentState == GAMEOVER_STATE) {
                showGameOver(screen, font);
                currentState = MENU_STATE;
                if (enigmeMusic) Mix_PlayMusic(enigmeMusic, -1);
            }
            else if (currentState == WIN_STATE) {
                showWinScreen(screen, font);
                currentState = MENU_STATE;
            }
        }

        SDL_Delay(10);
    }

    if (beepSound) Mix_FreeChunk(beepSound);
    if (hoverSound) Mix_FreeChunk(hoverSound);
    if (enigmeMusic) Mix_FreeMusic(enigmeMusic);
    if (winMusic) Mix_FreeMusic(winMusic);
    Mix_CloseAudio();
    SDL_FreeSurface(menuBg);
    SDL_FreeSurface(quizButton);
    SDL_FreeSurface(quizButtonHover);
    SDL_FreeSurface(puzzleButton);
    SDL_FreeSurface(puzzleButtonHover);
    TTF_CloseFont(font);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
