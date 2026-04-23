#ifndef QUIZ_H
#define QUIZ_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_Q 20
#define TEMPS_ENIGME 10

typedef struct
{
    char question[256];
    char rep1[128];
    char rep2[128];
    char rep3[128];
    int numQuestSelect;
    int numbr;
    int etat;
    SDL_Rect posQuestion;
    SDL_Rect posRep1;
    SDL_Rect posRep2;
    SDL_Rect posRep3;
    SDL_Rect posStart;
    SDL_Rect posQuit;
} Enigme;

TTF_Font *chargerPolice(int taille);
void drawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);
int chargerQuestions(const char *nomFichier, char questions[][256], char rep1[][128], char rep2[][128], char rep3[][128], int bonnes[]);
Enigme generer(const char *nomFichier);
void afficherEcran(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *background, Enigme e, int mode, int score, int vies, int tempsRestant, const char *message);

#endif
