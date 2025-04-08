#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Constants
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
#define TIMER_Y 50
#define TIMER_RADIUS 50
#define TIMER_CENTER_X (WINDOW_WIDTH/2)
#define TIMER_CENTER_Y (TIMER_Y + TIMER_RADIUS)
#define TIMER_DURATION 30
#define BEEP_INTERVAL_START 2000
#define BEEP_INTERVAL_MIN 200
#define NEXT_BUTTON_WIDTH 200
#define NEXT_BUTTON_HEIGHT 60
#define QUESTIONS_TO_WIN 3

// Game state enum
typedef enum {
    MENU_STATE,
    GAME_STATE,
    GAMEOVER_STATE,
    WIN_STATE
} GameState;

// Point structure
typedef struct {
    int x, y;
} Point;

// Quiz question structure
typedef struct {
    char question[MAX_LINE_LENGTH];
    char answers[3][MAX_LINE_LENGTH];
    int correctAnswer;
} QuizQuestion;

// Player stats structure
typedef struct {
    int score;
    int lives;
    int level;
    int consecutiveCorrect;
    int questionsSurvived;
} PlayerStats;

// Function declarations
int pointInRect(Point p, SDL_Rect r);
SDL_Surface* loadImage(const char *path);
void renderCenteredText(SDL_Surface *screen, TTF_Font *font, const char *text, 
                       SDL_Rect rect, SDL_Color color);
void updateBeepSound(float progress);
void drawTimer(SDL_Surface *screen, float progress);
void renderPlayerStats(SDL_Surface *screen, TTF_Font *font);
int loadQuestions(const char *filename, QuizQuestion *questions);
void showGameOver(SDL_Surface *screen, TTF_Font *font);
void showWinScreen(SDL_Surface *screen, TTF_Font *font);
void runQuizGame(SDL_Surface *screen, TTF_Font *font, QuizQuestion *questions, int questionCount);
int initGame(SDL_Surface **screen, TTF_Font **font);
void cleanupGame(SDL_Surface *menuBg, SDL_Surface *quizButton, SDL_Surface *quizButtonHover,
                SDL_Surface *puzzleButton, SDL_Surface *puzzleButtonHover, TTF_Font *font);

#endif
