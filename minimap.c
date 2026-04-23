#include "minimap.h"

void MAJMinimap(SDL_Rect playerPos, SDL_Rect obstacle, Minimap* m, float scaleX, float scaleY) {
    
    int centerX = playerPos.x + playerPos.w / 2;
    int centerY = playerPos.y + playerPos.h / 2;

    m->pointPosition.x = (int)(centerX * scaleX) + m->minimapPosition.x;
    m->pointPosition.y = (int)(centerY * scaleY) + m->minimapPosition.y;
    m->pointPosition.w = 6;
    m->pointPosition.h = 6;

    
    m->obstacleMini.x = (int)(obstacle.x * scaleX) + m->minimapPosition.x;
    m->obstacleMini.y = (int)(obstacle.y * scaleY) + m->minimapPosition.y;
    m->obstacleMini.w = (int)(obstacle.w * scaleX);
    m->obstacleMini.h = (int)(obstacle.h * scaleY);
}

void Liberer(Minimap* m) {
    if (m->backgroundTexture) SDL_DestroyTexture(m->backgroundTexture);
    if (m->pointTexture) SDL_DestroyTexture(m->pointTexture);
    m->backgroundTexture = NULL;
    m->pointTexture = NULL;
}
