#include "enigme.h"

void resetGame(int *nb, int t[]) {
    *nb = 0;
    memset(t, 0, sizeof(int) * 6);
}

void choicehint(char* hintText, int pattern[], int difficulty) {
    if (difficulty == 0) { // Facile 
        strcpy(hintText, "les 4 premiers clicks sont croissants");
    }
    else if (difficulty == 1) { // Moyen
        strcpy(hintText, "Indice: 3 clicks successifs se repetent");
    }
    else if (difficulty == 2) { // Difficile
        strcpy(hintText, "Indice: 5 -> 4 -> 6 et 2 se repete");
    }
}
