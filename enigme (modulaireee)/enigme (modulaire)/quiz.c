#include "header.h"

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

int main() {
    SDL_Surface *screen = NULL;
    TTF_Font *font = NULL;
    
    if (!initGame(&screen, &font)) {
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

    cleanupGame(menuBg, quizButton, quizButtonHover, puzzleButton, puzzleButtonHover, font);
    return 0;
}
