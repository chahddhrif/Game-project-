#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "back.h"
#include "joueur.h"

static void afficherGrandTexte(SDL_Renderer *re, TTF_Font *font,
                                const char *txt, SDL_Color col)
{
    if (!font) return;
    SDL_Surface *s = TTF_RenderText_Blended(font, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(re, s);
    SDL_Rect dst = {WINDOW_W/2-s->w/2, WINDOW_H/2-s->h/2, s->w, s->h};
    SDL_RenderCopy(re, t, NULL, &dst);
    SDL_FreeSurface(s); SDL_DestroyTexture(t);
}

static void jouerVideo(void) {
    system("mpv --fs --no-terminal assets/video/scene.mp4 2>/dev/null || "
           "vlc --fullscreen --play-and-exit assets/video/scene.mp4 2>/dev/null");
}

static void placeEnemies(Ennemi *enemies, int *nb, SDL_Renderer *re)
{
    *nb = 0;
    ennemiInit(&enemies[(*nb)++], re, ENEMY_TYPE_1,
               PAGE_W*1+300+rand()%(PAGE_W-600), GROUND_Y); SDL_Delay(80);
    ennemiInit(&enemies[(*nb)++], re, ENEMY_TYPE_2,
               PAGE_W*2+300+rand()%(PAGE_W-600), GROUND_Y); SDL_Delay(80);
    ennemiInit(&enemies[(*nb)++], re, ENEMY_TYPE_3,
               PAGE_W*3+300+rand()%(PAGE_W-600), GROUND_Y); SDL_Delay(80);
    ennemiInit(&enemies[(*nb)++], re, ENEMY_LOSEF,
               PAGE_W*4+PAGE_W/2, GROUND_Y); SDL_Delay(80);
    ennemiInit(&enemies[(*nb)++], re, ENEMY_VIGGO,
               PAGE_W*5+PAGE_W/2, GROUND_Y);
}

static void reloadCostume(Joueur *j, SDL_Renderer *re)
{
    CostumeType  c = j->costume;
    PersonnageType p = j->perso;
    float wx = j->wx, wy = j->wy;
    int   hp = j->hp;
    joueurFree(j);
    joueurInit(j, re, p, c);
    j->wx = wx; j->wy = wy; j->hp = hp;
}

int main(void)
{
    srand((unsigned)time(NULL));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_AllocateChannels(64);

    SDL_Window *win = SDL_CreateWindow("John Wick - Empty Revenge",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *re = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Background back;
    initBack(&back, re);
    initMiniMap(&back);
    initScore(&back, re);
    initHUD(&back, re);
    initCollectibles(&back, re);
    initBombes(&back, re);
    initMines(&back, re);
    initHealthBar(&back.hp1, WINDOW_W/2-150, 10, 300, 28, 100);

    Joueur joueur;
    joueurInit(&joueur, re, PERSO_JOHN, COSTUME_JOHN);

    Chien chien;
    chienInit(&chien, re, joueur.wx - DOG_OFFSET, GROUND_Y);

    Ennemi enemies[MAX_ENEMIES];
    int nb_enemies = 0;
    placeEnemies(enemies, &nb_enemies, re);
    int viggo_idx = nb_enemies - 1;

    int running=1, game_over=0, dog_dead_flag=0, video_played=0, prev_shoot=0;
    SDL_Event event;

    while (running) {
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                SDL_Scancode sc = event.key.keysym.scancode;
                if (sc == SDL_SCANCODE_ESCAPE) running = 0;
                if (sc == SDL_SCANCODE_H) toggleHelp(&back);
                if (sc == SDL_SCANCODE_N) {
                    back.mode  = (back.mode==0) ? 1 : 0;
                    back.hp1.x = (back.mode==1) ? 380 : WINDOW_W/2-150;
                }
            }
            if (!game_over && !back.showHelp)
                joueurHandleInput(&joueur, keys, &event);
            handleScoreEvent(&back, &event);
        }

        /* Pause */
        if (back.showHelp) {
            SDL_SetRenderDrawColor(re,0,0,0,255);
            SDL_RenderClear(re);
            setCamFromPlayer(&back, joueur.wx);
            afficherBack(&back, re, 1);
            afficherHelp(&back, re);
            SDL_RenderPresent(re);
            SDL_Delay(FPS_DELAY);
            continue;
        }

        /* Caméra suit le joueur */
        setCamFromPlayer(&back, joueur.wx);
        scrolling(&back, keys);
        int cam_x = back.cam1.x;

        joueurUpdate(&joueur);
        chienUpdate(&chien, &joueur);

        back.hp1.hp    = joueur.hp;
        back.hp1.hpMax = joueur.hp_max;

        if (!game_over) {
            for (int i=0; i<nb_enemies; i++)
                ennemiUpdate(&enemies[i], joueur.wx, cam_x);

            checkBulletsVsEnemies(&joueur, enemies, nb_enemies);
            checkPunchKickVsEnemies(&joueur, enemies, nb_enemies);
            checkEnemyBulletsVsPlayer(enemies, nb_enemies, &joueur);

            int costume_signal = -1;
            updateCollectibles(&back, joueur.wx, joueur.wy, &costume_signal);
            if (costume_signal == -2) {
                joueur.hp = joueur.hp_max;
            } else if (costume_signal == 1) {
                joueur.costume = (joueur.perso==PERSO_JOHN) ?
                    COSTUME_JOHN_ANTIBALL : COSTUME_HELEN_ANTIBALL;
                reloadCostume(&joueur, re);
            }

            int curr_shoot = (joueur.current_anim==ANIM_SHOOT) ? 1 : 0;
            if (curr_shoot && !prev_shoot && back.hud_bullets>0)
                back.hud_bullets--;
            prev_shoot = curr_shoot;

            int bombe_killed = 0;
            updateBombes(&back, joueur.wx, joueur.wy, &bombe_killed);
            if (bombe_killed && joueur.alive) {
                joueur.alive=0; joueur.hp=0;
                joueur.current_anim=ANIM_DEATH;
                resetAnim(&joueur.anims[ANIM_DEATH]);
                if (joueur.snd_death) Mix_PlayChannel(-1, joueur.snd_death, 0);
            }

            int mine_dog=0;
            updateMines(&back, chien.wx, chien.wy, &mine_dog);
            if (mine_dog && chien.alive && !dog_dead_flag) {
                dog_dead_flag=1; chien.alive=0; chien.current_anim=2;
                if (!video_played) { video_played=1; jouerVideo(); }
            }

            if (!joueur.alive && joueur.anims[ANIM_DEATH].finished)
                game_over=1;

            if (joueur.wx>=PAGE_W*5 && !enemies[viggo_idx].alive &&
                enemies[viggo_idx].anims[ANIM_DEATH].finished) {
                joueur.victory=1; game_over=2;
                resetAnim(&joueur.anims[ANIM_VICTORY]);
            }
        } else {
            joueurUpdate(&joueur);
        }

        /* RENDU */
        SDL_SetRenderDrawColor(re,0,0,0,255);
        SDL_RenderClear(re);

        afficherBack(&back, re, 1);
        afficherMines(&back, re, cam_x);
        afficherCollectibles(&back, re, cam_x);

        for (int i=0; i<nb_enemies; i++) {
            ennemiRender(&enemies[i], re, cam_x);
            ennemiRenderHealthBar(&enemies[i], re, cam_x);
            ennemiRenderBullets(&enemies[i], re, cam_x);
        }

        chienRender(&chien, re, cam_x);
        joueurRender(&joueur, re, cam_x);
        joueurRenderBullets(&joueur, re, cam_x);
        afficherBombes(&back, re, cam_x);

        for (int i=0; i<nb_enemies; i++)
            ennemiRenderGreat(&enemies[i], re, back.fontBig);

        afficherMessage(&back, re);
        afficherHUD(&back, re);
        afficherHealthBar(&back.hp1, re);
        afficherMiniMap(&back, re, joueur.wx, joueur.wy, joueur.wx, joueur.wy);
        afficherTemps(&back, re, 1);

        if (game_over==1) {
            SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(re,0,0,0,160);
            SDL_Rect f={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(re,&f);
            SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
            SDL_Color rouge={220,0,0,255};
            afficherGrandTexte(re, back.fontBig, "DEFEAT", rouge);
        }
        if (game_over==2) {
            SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(re,0,0,0,140);
            SDL_Rect f={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(re,&f);
            SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
            SDL_Color bleu={30,100,255,255};
            afficherGrandTexte(re, back.fontBig, "VICTORY!", bleu);
        }

        afficherScore(&back, re);
        SDL_RenderPresent(re);
        SDL_Delay(FPS_DELAY);
    }

    joueurFree(&joueur);
    chienFree(&chien);
    for (int i=0; i<nb_enemies; i++) ennemiFree(&enemies[i]);
    libererBack(&back);
    libererScore(&back);
    Mix_CloseAudio();
    TTF_Quit(); IMG_Quit();
    SDL_DestroyRenderer(re); SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
