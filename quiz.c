#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

TTF_Font *chargerPolice(int taille)
{
    TTF_Font *font = NULL;
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", taille);
    if (font != NULL) return font;
    font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", taille);
    if (font != NULL) return font;
    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", taille);
    return font;
}

void drawText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect pos;

    if (font == NULL || text == NULL) return;
    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (surface == NULL) return;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL)
    {
        SDL_FreeSurface(surface);
        return;
    }
    pos.x = x;
    pos.y = y;
    pos.w = surface->w;
    pos.h = surface->h;
    SDL_RenderCopy(renderer, texture, NULL, &pos);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int chargerQuestions(const char *nomFichier, char questions[][256], char rep1[][128], char rep2[][128], char rep3[][128], int bonnes[])
{
    FILE *f;
    int n;

    n = 0;
    f = fopen(nomFichier, "r");
    if (f == NULL) return 0;

    while (n < MAX_Q && fscanf(f, " %255[^|]|%127[^|]|%127[^|]|%127[^|]|%d\n", questions[n], rep1[n], rep2[n], rep3[n], &bonnes[n]) == 5)
        n++;

    fclose(f);
    return n;
}

Enigme generer(const char *nomFichier)
{
    Enigme e;
    char questions[MAX_Q][256];
    char rep1[MAX_Q][128];
    char rep2[MAX_Q][128];
    char rep3[MAX_Q][128];
    int bonnes[MAX_Q];
    int n;
    int indice;

    strcpy(e.question, "Question indisponible");
    strcpy(e.rep1, "Reponse 1");
    strcpy(e.rep2, "Reponse 2");
    strcpy(e.rep3, "Reponse 3");
    e.numQuestSelect = 1;
    e.numbr = 1;
    e.etat = 0;
    e.posQuestion.x = 110;
    e.posQuestion.y = 90;
    e.posQuestion.w = 580;
    e.posQuestion.h = 80;
    e.posRep1.x = 150;
    e.posRep1.y = 230;
    e.posRep1.w = 500;
    e.posRep1.h = 60;
    e.posRep2.x = 150;
    e.posRep2.y = 320;
    e.posRep2.w = 500;
    e.posRep2.h = 60;
    e.posRep3.x = 150;
    e.posRep3.y = 410;
    e.posRep3.w = 500;
    e.posRep3.h = 60;
    e.posStart.x = 270;
    e.posStart.y = 290;
    e.posStart.w = 260;
    e.posStart.h = 70;
    e.posQuit.x = 270;
    e.posQuit.y = 400;
    e.posQuit.w = 260;
    e.posQuit.h = 70;

    n = chargerQuestions(nomFichier, questions, rep1, rep2, rep3, bonnes);
    if (n > 0)
    {
        indice = rand() % n;
        strcpy(e.question, questions[indice]);
        strcpy(e.rep1, rep1[indice]);
        strcpy(e.rep2, rep2[indice]);
        strcpy(e.rep3, rep3[indice]);
        e.numQuestSelect = indice + 1;
        e.numbr = bonnes[indice];
    }

    return e;
}

void afficherEcran(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *background, Enigme e, int mode, int score, int vies, int tempsRestant, const char *message)
{
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color jaune = {255, 220, 80, 255};
    SDL_Color vert = {0, 220, 80, 255};
    SDL_Color rouge = {255, 60, 60, 255};
    SDL_Color bleu = {130, 220, 255, 255};
    SDL_Rect fond = {60, 50, 680, 520};
    SDL_Rect cadre = {170, 120, 460, 390};
    SDL_Rect zone = {60, 560, 680, 40};
    char texte[256];
    int i;
    double progression;
    double angle1;
    double angle2;
    int x1;
    int y1;
    int x2;
    int y2;
    int rayon;

    if (background != NULL)
        SDL_RenderCopy(renderer, background, NULL, NULL);
    else
    {
        SDL_SetRenderDrawColor(renderer, 10, 12, 18, 255);
        SDL_RenderClear(renderer);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (mode == 0)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &cadre);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &cadre);
        drawText(renderer, font, "ENIGME - DEMO", 315, 165, jaune);
        drawText(renderer, font, "Cliquez sur START pour lancer l'enigme", 205, 215, blanc);

        SDL_SetRenderDrawColor(renderer, 55, 65, 90, 255);
        SDL_RenderFillRect(renderer, &e.posStart);
        SDL_RenderFillRect(renderer, &e.posQuit);
        SDL_SetRenderDrawColor(renderer, 130, 220, 255, 255);
        SDL_RenderDrawRect(renderer, &e.posStart);
        SDL_RenderDrawRect(renderer, &e.posQuit);
        drawText(renderer, font, "START", e.posStart.x + 80, e.posStart.y + 16, blanc);
        drawText(renderer, font, "QUIT", e.posQuit.x + 92, e.posQuit.y + 16, blanc);
    }
    else if (mode == 1)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &fond);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &fond);
        SDL_RenderDrawRect(renderer, &e.posQuestion);
        SDL_RenderDrawRect(renderer, &e.posRep1);
        SDL_RenderDrawRect(renderer, &e.posRep2);
        SDL_RenderDrawRect(renderer, &e.posRep3);
        SDL_RenderDrawRect(renderer, &zone);
        drawText(renderer, font, "ENIGME", 335, 60, jaune);
        drawText(renderer, font, e.question, e.posQuestion.x + 16, e.posQuestion.y + 22, blanc);
        sprintf(texte, "1) %s", e.rep1);
        drawText(renderer, font, texte, e.posRep1.x + 18, e.posRep1.y + 18, blanc);
        sprintf(texte, "2) %s", e.rep2);
        drawText(renderer, font, texte, e.posRep2.x + 18, e.posRep2.y + 18, blanc);
        sprintf(texte, "3) %s", e.rep3);
        drawText(renderer, font, texte, e.posRep3.x + 18, e.posRep3.y + 18, blanc);
        sprintf(texte, "Score : %d", score);
        drawText(renderer, font, texte, 85, 568, blanc);
        sprintf(texte, "Vies : %d", vies);
        drawText(renderer, font, texte, 250, 568, blanc);
        sprintf(texte, "Question : %d", e.numQuestSelect);
        drawText(renderer, font, texte, 500, 568, blanc);

        progression = (double)tempsRestant / (double)TEMPS_ENIGME;
        if (progression < 0.0) progression = 0.0;
        if (progression > 1.0) progression = 1.0;
        rayon = 35;
        if (tempsRestant > 6) SDL_SetRenderDrawColor(renderer, 0, 220, 0, 255);
        else if (tempsRestant > 3) SDL_SetRenderDrawColor(renderer, 255, 180, 0, 255);
        else SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
        for (i = 0; i < 360 * progression; i++)
        {
            angle1 = (-90 + i) * M_PI / 180.0;
            angle2 = (-89 + i) * M_PI / 180.0;
            x1 = 670 + (int)(rayon * cos(angle1));
            y1 = 105 + (int)(rayon * sin(angle1));
            x2 = 670 + (int)(rayon * cos(angle2));
            y2 = 105 + (int)(rayon * sin(angle2));
            SDL_RenderDrawLine(renderer, 670, 105, x1, y1);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (i = 0; i < 360; i++)
        {
            angle1 = i * M_PI / 180.0;
            angle2 = (i + 1) * M_PI / 180.0;
            x1 = 670 + (int)(rayon * cos(angle1));
            y1 = 105 + (int)(rayon * sin(angle1));
            x2 = 670 + (int)(rayon * cos(angle2));
            y2 = 105 + (int)(rayon * sin(angle2));
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
        sprintf(texte, "%d", tempsRestant);
        drawText(renderer, font, texte, 662, 95, blanc);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &fond);
        if (mode == 2) drawText(renderer, font, message, 285, 270, vert);
        else drawText(renderer, font, message, 275, 270, rouge);
        drawText(renderer, font, "Patientez...", 320, 320, bleu);
    }
}
