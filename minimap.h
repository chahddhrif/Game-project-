#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Texture* backgroundTexture;
    SDL_Rect minimapPosition;

    SDL_Texture* pointTexture;
    SDL_Rect pointPosition;

    SDL_Rect obstacleMini;
} Minimap;

void MAJMinimap(SDL_Rect playerPos, SDL_Rect obstacle, Minimap* m, float scaleX, float scaleY);
void afficherMinimap(SDL_Renderer* renderer, Minimap* m, SDL_Rect animPos, SDL_Rect playerAnim, SDL_Rect obsAnim);
void Liberer(Minimap* m);

#endif
