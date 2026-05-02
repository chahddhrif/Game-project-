#include "back.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
   BANIM HELPERS
   ============================================================ */
BAnim banimLoad(SDL_Renderer *re, const char *path,
                int cols, int rows, int delay, int loop)
{
    BAnim a = {0};
    if (re && path) {
        a.tex = IMG_LoadTexture(re, path);
        if (!a.tex)
            fprintf(stderr, "BAnim ERREUR: %s\n", path);
    }
    a.cols = cols; a.rows = rows;
    a.total = cols * rows;
    if (a.tex) {
        int tw, th;
        SDL_QueryTexture(a.tex, NULL, NULL, &tw, &th);
        a.frame_w = tw / cols;
        a.frame_h = th / rows;
    }
    a.delay = delay; a.loop = loop;
    return a;
}

void banimFree(BAnim *a) {
    if (a->tex) { SDL_DestroyTexture(a->tex); a->tex = NULL; }
}

void banimUpdate(BAnim *a) {
    if (!a->tex || a->finished) return;
    Uint32 now = SDL_GetTicks();
    if (now - a->last_time >= (Uint32)a->delay) {
        a->last_time = now;
        a->current++;
        if (a->current >= a->total) {
            if (a->loop) a->current = 0;
            else { a->current = a->total - 1; a->finished = 1; }
        }
    }
}

void banimRender(BAnim *a, SDL_Renderer *re, SDL_Rect *dst) {
    if (!a->tex) return;
    int col = a->current % a->cols;
    int row = a->current / a->cols;
    SDL_Rect src = { col * a->frame_w, row * a->frame_h,
                     a->frame_w, a->frame_h };
    SDL_RenderCopy(re, a->tex, &src, dst);
}

/* ============================================================
   DRAWLINE
   ============================================================ */
int drawLine(SDL_Renderer *re, TTF_Font *font,
             SDL_Color col, char *txt, int x, int y)
{
    if (!txt || !txt[0] || !font) return 0;
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, col);
    if (!s) return 0;
    SDL_Texture *t = SDL_CreateTextureFromSurface(re, s);
    SDL_Rect dst = {x, y, s->w, s->h};
    SDL_RenderCopy(re, t, NULL, &dst);
    int h = s->h;
    SDL_FreeSurface(s); SDL_DestroyTexture(t);
    return h;
}

/* ============================================================
   SCORES
   ============================================================ */
void saveScores(Background *b) {
    FILE *f = fopen("scores.txt", "w");
    if (!f) return;
    for (int i = 0; i < b->scoreCount; i++)
        fprintf(f, "%d %s\n", b->scores[i].score, b->scores[i].name);
    fclose(f);
}

void loadScores(Background *b) {
    b->scoreCount = 0;
    FILE *f = fopen("scores.txt", "r");
    if (!f) return;
    while (b->scoreCount < 100) {
        ScoreEntry *e = &b->scores[b->scoreCount];
        if (fscanf(f, "%d ", &e->score) != 1) break;
        if (!fgets(e->name, 64, f)) break;
        int len = (int)strlen(e->name);
        if (len > 0 && e->name[len-1] == '\n') e->name[len-1] = '\0';
        b->scoreCount++;
    }
    fclose(f);
}

void trierScores(Background *b) {
    for (int i = 0; i < b->scoreCount - 1; i++)
        for (int j = 0; j < b->scoreCount - 1 - i; j++)
            if (b->scores[j].score < b->scores[j+1].score) {
                ScoreEntry tmp = b->scores[j];
                b->scores[j] = b->scores[j+1];
                b->scores[j+1] = tmp;
            }
}

/* ============================================================
   INIT BACKGROUND
   ============================================================ */
void initBack(Background *b, SDL_Renderer *re)
{
    /* Ciel */
    b->sky = IMG_LoadTexture(re, "assets/back1/ciel.png");

    /* 4 bâtiments */
    b->bat[0] = IMG_LoadTexture(re, "assets/back1/bat.png");
    b->bat[1] = IMG_LoadTexture(re, "assets/back1/batiment2.png");
    b->bat[2] = IMG_LoadTexture(re, "assets/back1/batiment3.png");
    b->bat[3] = IMG_LoadTexture(re, "assets/back1/batiment4.png");

    b->ground1    = IMG_LoadTexture(re, "assets/back1/sol1.png");
    b->ground2    = IMG_LoadTexture(re, "assets/back1/sol2.png");
    b->rain       = IMG_LoadTexture(re, "assets/back1/pluie.png");
    b->territoire = IMG_LoadTexture(re, "assets/back1/territoire1.png");

    /* Son pluie */
    b->rainSound = Mix_LoadMUS("assets/sounds/rain_loop.mp3");
    if (b->rainSound) Mix_PlayMusic(b->rainSound, -1);

    b->mode  = 0;
    b->rainY = 0;
    b->cam1  = (SDL_Rect){0,0,1920,1080};
    b->cam2  = (SDL_Rect){0,0,1920,1080};

    /* Polices */
    b->font         = TTF_OpenFont("arial(1).ttf", 36);
    b->fontHelp     = TTF_OpenFont("arial(1).ttf", 52);
    b->fontHelpText = TTF_OpenFont("arial(1).ttf", 32);
    b->fontBig      = TTF_OpenFont("arial(1).ttf", 160);
    b->fontHUD      = TTF_OpenFont("arial(1).ttf", 28);

    b->color     = (SDL_Color){255,255,255,255};
    b->startTime = SDL_GetTicks();
    b->showHelp  = 0;
    b->pauseTime = 0;

    /* Messages */
    b->msg_active       = 0;
    b->score_anim_active = 0;
}

/* ============================================================
   HUD INIT
   ============================================================ */
void initHUD(Background *b, SDL_Renderer *re)
{
    b->count_1dt    = 0;
    b->count_5dt    = 0;
    b->count_diamond = 0;
    b->total_score  = 0;
    b->hud_bullets  = MAX_HUD_BULLETS;

    b->tex_1dt_fixe      = IMG_LoadTexture(re, "assets/obstacles/1dt_image_fixe.png");
    b->tex_5dt_fixe      = IMG_LoadTexture(re, "assets/obstacles/5dt_image_fixe.png");
    b->tex_diamond_fixe  = IMG_LoadTexture(re, "assets/obstacles/diamand_image_fixe.png");
    b->tex_bullet_score  = IMG_LoadTexture(re, "assets/obstacles/bullet_score.png");

    b->tex_1dt_sheet     = IMG_LoadTexture(re, "assets/obstacles/1dinars_spritesheet.png");
    b->tex_5dt_sheet     = IMG_LoadTexture(re, "assets/obstacles/5dinars_spritesheet.png");
    b->tex_diamond_sheet = IMG_LoadTexture(re, "assets/obstacles/diamond.png");
    b->tex_blood_sheet   = IMG_LoadTexture(re, "assets/obstacles/blood_bag_spritesheet.png");
    b->tex_ammo_sheet    = IMG_LoadTexture(re, "assets/obstacles/munitions_spritesheet.png");
    b->tex_antiball_sheet= IMG_LoadTexture(re, "assets/obstacles/antiball_spritesheet.png");

    if (!b->tex_1dt_fixe)      fprintf(stderr, "ERREUR: 1dt_image_fixe.png\n");
    if (!b->tex_5dt_fixe)      fprintf(stderr, "ERREUR: 5dt_image_fixe.png\n");
    if (!b->tex_diamond_fixe)  fprintf(stderr, "ERREUR: diamand_image_fixe.png\n");
    if (!b->tex_1dt_sheet)     fprintf(stderr, "ERREUR: 1dinars_spritesheet.png\n");
    if (!b->tex_5dt_sheet)     fprintf(stderr, "ERREUR: 5dinars_spritesheet.png\n");
    if (!b->tex_diamond_sheet) fprintf(stderr, "ERREUR: diamond.png\n");
    if (!b->tex_blood_sheet)   fprintf(stderr, "ERREUR: blood_bag_spritesheet.png\n");
    if (!b->tex_ammo_sheet)    fprintf(stderr, "ERREUR: munitions_spritesheet.png\n");
    if (!b->tex_antiball_sheet)fprintf(stderr, "ERREUR: antiball_spritesheet.png\n");
}

/* ============================================================
   INIT COLLECTIBLES (placement dans le monde)
   ============================================================ */
void initCollectibles(Background *b, SDL_Renderer *re)
{
    b->nb_collectibles = 0;
    (void)re;

    /* Structure simple pour placer un collectible */
    struct { CollectibleType type; float wx; SDL_Texture *sheet; } items[] = {
        /* Page 0 */
        { COL_BLOOD,    PAGE_W*0 + 500,  b->tex_blood_sheet   },
        { COL_AMMO,     PAGE_W*0 + 1000, b->tex_ammo_sheet    },
        { COL_1DT,      PAGE_W*0 + 1400, b->tex_1dt_sheet     },
        /* Page 1 */
        { COL_1DT,      PAGE_W*1 + 300,  b->tex_1dt_sheet     },
        { COL_5DT,      PAGE_W*1 + 700,  b->tex_5dt_sheet     },
        { COL_DIAMOND,  PAGE_W*1 + 1100, b->tex_diamond_sheet },
        { COL_ANTIBALL, PAGE_W*1 + 1500, b->tex_antiball_sheet},
        /* Page 2 */
        { COL_BLOOD,    PAGE_W*2 + 400,  b->tex_blood_sheet   },
        { COL_5DT,      PAGE_W*2 + 800,  b->tex_5dt_sheet     },
        { COL_AMMO,     PAGE_W*2 + 1200, b->tex_ammo_sheet    },
        /* Page 3 */
        { COL_1DT,      PAGE_W*3 + 300,  b->tex_1dt_sheet     },
        { COL_DIAMOND,  PAGE_W*3 + 700,  b->tex_diamond_sheet },
        { COL_ANTIBALL, PAGE_W*3 + 1100, b->tex_antiball_sheet},
        { COL_BLOOD,    PAGE_W*3 + 1500, b->tex_blood_sheet   },
        /* Page 4 */
        { COL_5DT,      PAGE_W*4 + 400,  b->tex_5dt_sheet     },
        { COL_AMMO,     PAGE_W*4 + 900,  b->tex_ammo_sheet    },
        { COL_DIAMOND,  PAGE_W*4 + 1400, b->tex_diamond_sheet },
        /* Page 5 */
        { COL_BLOOD,    PAGE_W*5 + 300,  b->tex_blood_sheet   },
        { COL_1DT,      PAGE_W*5 + 700,  b->tex_1dt_sheet     },
        { COL_5DT,      PAGE_W*5 + 1100, b->tex_5dt_sheet     },
    };

    int n = (int)(sizeof(items)/sizeof(items[0]));
    for (int i = 0; i < n && b->nb_collectibles < MAX_COLLECTIBLES; i++) {
        Collectible *c = &b->collectibles[b->nb_collectibles++];
        c->type   = items[i].type;
        c->wx     = items[i].wx;
        c->wy     = (float)GROUND_Y; /* posé sur le sol */
        c->active = 1;

        /* Initialiser animation */
        memset(&c->anim, 0, sizeof(BAnim));
        c->anim.tex = items[i].sheet;
        c->anim.cols = 5; c->anim.rows = 5;
        c->anim.total = 25;
        c->anim.delay = 80;
        c->anim.loop  = 1;
        if (c->anim.tex) {
            int tw, th;
            SDL_QueryTexture(c->anim.tex, NULL, NULL, &tw, &th);
            c->anim.frame_w = tw / 5;
            c->anim.frame_h = th / 5;
        }
    }
}

/* ============================================================
   UPDATE COLLECTIBLES
   ============================================================ */
void updateCollectibles(Background *b, float player_wx, float player_wy,
                        int *costume_changed)
{
    *costume_changed = -1; /* -1 = pas de changement */

    for (int i = 0; i < b->nb_collectibles; i++) {
        Collectible *c = &b->collectibles[i];
        if (!c->active) continue;

        banimUpdate(&c->anim);

        /* Collision avec joueur (zone 80px) */
        float dx = fabsf(player_wx - c->wx);
        float dy = fabsf(player_wy - c->wy);
        if (dx < 80 && dy < 120) {
            c->active = 0;
            SDL_Color col = {255, 220, 0, 255};

            switch (c->type) {
            case COL_1DT:
                b->count_1dt++;
                b->total_score += 10;
                b->score_anim_val   = 10;
                b->score_anim_timer = SDL_GetTicks();
                b->score_anim_active = 1;
                break;
            case COL_5DT:
                b->count_5dt++;
                b->total_score += 50;
                b->score_anim_val   = 50;
                b->score_anim_timer = SDL_GetTicks();
                b->score_anim_active = 1;
                break;
            case COL_DIAMOND:
                b->count_diamond++;
                b->total_score += 100;
                b->score_anim_val   = 100;
                b->score_anim_timer = SDL_GetTicks();
                b->score_anim_active = 1;
                col.r=100; col.g=200; col.b=255;
                showMessage(b, "CONGRATULATION!", col);
                break;
            case COL_BLOOD:
                /* Recharge vie complète → signal via costume_changed = -2 */
                *costume_changed = -2;
                showMessage(b, "LIFE RESTORED!", col);
                break;
            case COL_AMMO:
                b->hud_bullets = MAX_HUD_BULLETS;
                showMessage(b, "AMMO RELOADED!", col);
                break;
            case COL_ANTIBALL:
                /* Changer costume antiball → signal via costume_changed = 1 */
                *costume_changed = 1;
                showMessage(b, "ANTIBALL VEST!", col);
                break;
            default: break;
            }
        }
    }
}

/* ============================================================
   AFFICHER COLLECTIBLES
   ============================================================ */
void afficherCollectibles(Background *b, SDL_Renderer *re, int cam_x)
{
    for (int i = 0; i < b->nb_collectibles; i++) {
        Collectible *c = &b->collectibles[i];
        if (!c->active) continue;
        int sx = (int)c->wx - cam_x;
        /* Afficher seulement si visible à l'écran */
        if (sx < -150 || sx > WINDOW_W + 150) continue;

        /* Taille plus grande pour être visible */
        int w = 100, h = 100;
        SDL_Rect dst = { sx - w/2, (int)c->wy - h, w, h };

        if (c->anim.tex) {
            banimRender(&c->anim, re, &dst);
        } else {
            /* Fallback : rectangle coloré si texture manque */
            switch (c->type) {
            case COL_1DT:     SDL_SetRenderDrawColor(re,255,215,0,255);   break;
            case COL_5DT:     SDL_SetRenderDrawColor(re,255,180,0,255);   break;
            case COL_DIAMOND: SDL_SetRenderDrawColor(re,100,200,255,255); break;
            case COL_BLOOD:   SDL_SetRenderDrawColor(re,200,0,0,255);     break;
            case COL_AMMO:    SDL_SetRenderDrawColor(re,150,75,0,255);    break;
            case COL_ANTIBALL:SDL_SetRenderDrawColor(re,0,150,200,255);   break;
            default:          SDL_SetRenderDrawColor(re,255,255,255,255); break;
            }
            SDL_RenderFillRect(re, &dst);
        }
    }
}

/* ============================================================
   AFFICHER HUD
   ============================================================ */
void afficherHUD(Background *b, SDL_Renderer *re)
{
    int x = 10, y = 10;
    char buf[64];
    SDL_Color white = {255,255,255,255};
    SDL_Color yellow = {255,220,0,255};

    /* --- Image 1DT + count --- */
    if (b->tex_1dt_fixe) {
        SDL_Rect r = {x, y, 40, 40};
        SDL_RenderCopy(re, b->tex_1dt_fixe, NULL, &r);
    }
    sprintf(buf, "x%d", b->count_1dt);
    drawLine(re, b->fontHUD, white, buf, x+44, y+8);
    x += 110;

    /* --- Image 5DT + count --- */
    if (b->tex_5dt_fixe) {
        SDL_Rect r = {x, y, 40, 40};
        SDL_RenderCopy(re, b->tex_5dt_fixe, NULL, &r);
    }
    sprintf(buf, "x%d", b->count_5dt);
    drawLine(re, b->fontHUD, white, buf, x+44, y+8);
    x += 110;

    /* --- Image diamond + count --- */
    if (b->tex_diamond_fixe) {
        SDL_Rect r = {x, y, 40, 40};
        SDL_RenderCopy(re, b->tex_diamond_fixe, NULL, &r);
    }
    sprintf(buf, "x%d", b->count_diamond);
    drawLine(re, b->fontHUD, white, buf, x+44, y+8);
    x += 110;

    /* --- Ligne de bullets HUD (20 bullets) --- */
    x += 20;
    for (int i = 0; i < MAX_HUD_BULLETS; i++) {
        if (b->tex_bullet_score) {
            SDL_Rect r = {x + i*28, y+2, 22, 36};
            if (i < b->hud_bullets) {
                SDL_SetTextureAlphaMod(b->tex_bullet_score, 255);
            } else {
                SDL_SetTextureAlphaMod(b->tex_bullet_score, 60);
            }
            SDL_RenderCopy(re, b->tex_bullet_score, NULL, &r);
        }
    }
    SDL_SetTextureAlphaMod(b->tex_bullet_score, 255);

    /* --- Score total à droite en jaune --- */
    sprintf(buf, "SCORE: %d", b->total_score);
    int tw, th;
    TTF_SizeText(b->fontHUD, buf, &tw, &th);
    drawLine(re, b->fontHUD, yellow, buf, 1920 - tw - 20, y+8);

    /* --- Score animation (incrémentation visible) --- */
    if (b->score_anim_active) {
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - b->score_anim_timer;
        if (elapsed < 800) {
            char sanim[32];
            sprintf(sanim, "+%d", b->score_anim_val);
            /* Position qui remonte */
            int ay = (int)(y + 50 - elapsed * 60 / 800);
            Uint8 alpha = (Uint8)(255 - elapsed * 255 / 800);
            SDL_Color col = {255, 255, 0, alpha};
            drawLine(re, b->fontHelp, col, sanim, 1920 - 120, ay);
        } else {
            b->score_anim_active = 0;
        }
    }
}

/* ============================================================
   INIT BOMBES CIEL
   ============================================================ */
void initBombes(Background *b, SDL_Renderer *re)
{
    b->tex_bombe_ciel = IMG_LoadTexture(re, "assets/obstacles/bombe_ciel.png");
    b->tex_bombe_exp  = IMG_LoadTexture(re, "assets/obstacles/bombe_1_explosé.png");
    b->bombe_timer    = SDL_GetTicks();

    if (!b->tex_bombe_ciel) fprintf(stderr, "ERREUR: bombe_ciel.png\n");
    if (!b->tex_bombe_exp)  fprintf(stderr, "ERREUR: bombe_1_explosé.png\n");

    for (int i = 0; i < MAX_BOMBES; i++) {
        b->bombes[i].active   = 0;
        b->bombes[i].exploded = 0;
    }
}

static void spawnBombe(Background *b, int cam_x)
{
    for (int i = 0; i < MAX_BOMBES; i++) {
        if (!b->bombes[i].active) {
            BombeCiel *bm = &b->bombes[i];
            /* Spawn en haut à DROITE de l'écran visible */
            bm->wx = (float)(cam_x + PAGE_W - 100 - rand() % 300);
            bm->wy = -100.0f;
            /* Vitesse diagonale 45° : droite→gauche */
            float spd = 7.0f;
            bm->vx = -spd;   /* va vers la gauche */
            bm->vy =  spd;   /* descend */
            bm->active   = 1;
            bm->exploded = 0;
            bm->spawn_time = SDL_GetTicks();

            memset(&bm->anim, 0, sizeof(BAnim));
            bm->anim.tex   = b->tex_bombe_ciel;
            bm->anim.cols  = 7; bm->anim.rows = 7;
            bm->anim.total = 49;
            bm->anim.delay = 30;
            bm->anim.loop  = 1;
            if (b->tex_bombe_ciel) {
                int tw, th;
                SDL_QueryTexture(b->tex_bombe_ciel, NULL, NULL, &tw, &th);
                bm->anim.frame_w = tw / 7;
                bm->anim.frame_h = th / 7;
            }

            memset(&bm->anim_exp, 0, sizeof(BAnim));
            bm->anim_exp.tex   = b->tex_bombe_exp;
            bm->anim_exp.cols  = 5; bm->anim_exp.rows = 5;
            bm->anim_exp.total = 25;
            bm->anim_exp.delay = 40;
            bm->anim_exp.loop  = 0;
            if (b->tex_bombe_exp) {
                int tw, th;
                SDL_QueryTexture(b->tex_bombe_exp, NULL, NULL, &tw, &th);
                bm->anim_exp.frame_w = tw / 5;
                bm->anim_exp.frame_h = th / 5;
            }
            break;
        }
    }
}

void updateBombes(Background *b, float player_wx, float player_wy,
                  int *player_dead)
{
    Uint32 now = SDL_GetTicks();

    /* Spawn une bombe toutes les 5-8 secondes */
    if (now - b->bombe_timer > 5000 + (Uint32)(rand() % 3000)) {
        b->bombe_timer = now;
        spawnBombe(b, (int)player_wx - PAGE_W/2);
    }

    for (int i = 0; i < MAX_BOMBES; i++) {
        BombeCiel *bm = &b->bombes[i];
        if (!bm->active) continue;

        if (!bm->exploded) {
            /* Mouvement diagonale 45° */
            bm->wx += bm->vx;
            bm->wy += bm->vy;

            /* Calculer la frame selon la progression vers le sol */
            float progress = bm->wy / (float)GROUND_Y;
            int target_frame = (int)(progress * bm->anim.total);
            if (target_frame >= bm->anim.total)
                target_frame = bm->anim.total - 1;
            bm->anim.current = target_frame;

            /* Collision avec joueur */
            float dx = fabsf(player_wx - bm->wx);
            float dy = fabsf(player_wy - bm->wy);
            if (dx < 100 && dy < 150) {
                *player_dead = 1;
                bm->exploded = 1;
                continue;
            }

            /* Touche le sol → explosion + son */
            if (bm->wy >= (float)GROUND_Y) {
                bm->exploded = 1;
                bm->wy = (float)GROUND_Y;
                /* Son bombardement */
                Mix_Chunk *snd = Mix_LoadWAV("assets/sounds/gun.wav");
                if (snd) { Mix_PlayChannel(-1, snd, 0); }
            }
        } else {
            /* Animation explosion */
            banimUpdate(&bm->anim_exp);
            if (bm->anim_exp.finished)
                bm->active = 0;
        }
    }
}

void afficherBombes(Background *b, SDL_Renderer *re, int cam_x)
{
    for (int i = 0; i < MAX_BOMBES; i++) {
        BombeCiel *bm = &b->bombes[i];
        if (!bm->active) continue;
        int sx = (int)bm->wx - cam_x;
        SDL_Rect dst = {sx - 50, (int)bm->wy - 80, 100, 100};
        if (!bm->exploded) {
            banimRender(&bm->anim, re, &dst);
        } else {
            dst.w = 150; dst.h = 150;
            dst.x = sx - 75; dst.y = (int)bm->wy - 120;
            banimRender(&bm->anim_exp, re, &dst);
        }
    }
}

void libererBombes(Background *b)
{
    if (b->tex_bombe_ciel) SDL_DestroyTexture(b->tex_bombe_ciel);
    if (b->tex_bombe_exp)  SDL_DestroyTexture(b->tex_bombe_exp);
}

/* ============================================================
   MINES SOL
   ============================================================ */
void initMines(Background *b, SDL_Renderer *re)
{
    b->tex_mine_normal = IMG_LoadTexture(re, "assets/obstacles/bombe_2_normale.png");
    b->tex_mine_exp    = IMG_LoadTexture(re, "assets/obstacles/bombe_2_exoplosé.png");

    /* Placer mines aléatoirement dans le monde */
    float positions[] = {
        PAGE_W*0 + 600,
        PAGE_W*1 + 400,
        PAGE_W*1 + 1200,
        PAGE_W*2 + 700,
        PAGE_W*3 + 300,
        PAGE_W*3 + 1400,
        PAGE_W*4 + 600,
        PAGE_W*4 + 1100,
        PAGE_W*5 + 400,
        PAGE_W*5 + 1000
    };

    for (int i = 0; i < MAX_MINES && i < 10; i++) {
        MineSol *m = &b->mines[i];
        m->wx = positions[i];
        m->wy = (float)GROUND_Y;
        m->active   = 1;
        m->exploded = 0;

        m->anim_normal.tex = b->tex_mine_normal;
        m->anim_normal.cols = 7; m->anim_normal.rows = 7;
        m->anim_normal.total = 49; m->anim_normal.delay = 120;
        m->anim_normal.loop = 1;
        if (b->tex_mine_normal) {
            int tw, th;
            SDL_QueryTexture(b->tex_mine_normal, NULL, NULL, &tw, &th);
            m->anim_normal.frame_w = tw/7;
            m->anim_normal.frame_h = th/7;
        }

        m->anim_exp.tex = b->tex_mine_exp;
        m->anim_exp.cols = 5; m->anim_exp.rows = 5;
        m->anim_exp.total = 25; m->anim_exp.delay = 50;
        m->anim_exp.loop = 0;
        if (b->tex_mine_exp) {
            int tw, th;
            SDL_QueryTexture(b->tex_mine_exp, NULL, NULL, &tw, &th);
            m->anim_exp.frame_w = tw/5;
            m->anim_exp.frame_h = th/5;
        }
    }
}

void updateMines(Background *b, float dog_wx, float dog_wy,
                 int *dog_dead)
{
    Uint32 now = SDL_GetTicks();

    for (int i = 0; i < MAX_MINES; i++) {
        MineSol *m = &b->mines[i];
        if (!m->active) continue;

        if (!m->exploded) {
            banimUpdate(&m->anim_normal);

            /* Collision chien - seulement après 3 secondes de jeu */
            if (now > 3000) {
                float dx = fabsf(dog_wx - m->wx);
                float dy = fabsf(dog_wy - m->wy);
                if (dx < 60 && dy < 80) {
                    m->exploded = 1;
                    *dog_dead   = 1;
                }
            }
        } else {
            banimUpdate(&m->anim_exp);
            if (m->anim_exp.finished)
                m->active = 0;
        }
    }
}

void afficherMines(Background *b, SDL_Renderer *re, int cam_x)
{
    for (int i = 0; i < MAX_MINES; i++) {
        MineSol *m = &b->mines[i];
        if (!m->active) continue;
        int sx = (int)m->wx - cam_x;
        if (sx < -100 || sx > 2100) continue;
        SDL_Rect dst = {sx - 40, (int)m->wy - 70, 80, 70};
        if (!m->exploded)
            banimRender(&m->anim_normal, re, &dst);
        else {
            dst.w = 120; dst.h = 120;
            dst.x = sx - 60; dst.y = (int)m->wy - 100;
            banimRender(&m->anim_exp, re, &dst);
        }
    }
}

void libererMines(Background *b)
{
    if (b->tex_mine_normal) SDL_DestroyTexture(b->tex_mine_normal);
    if (b->tex_mine_exp)    SDL_DestroyTexture(b->tex_mine_exp);
}

/* ============================================================
   MESSAGES FLOTTANTS
   ============================================================ */
void showMessage(Background *b, const char *txt, SDL_Color col)
{
    strncpy(b->msg_text, txt, 63);
    b->msg_timer  = SDL_GetTicks();
    b->msg_active = 1;
    b->msg_color  = col;
}

void afficherMessage(Background *b, SDL_Renderer *re)
{
    if (!b->msg_active || !b->fontHelp) return;
    Uint32 elapsed = SDL_GetTicks() - b->msg_timer;
    if (elapsed > 2000) { b->msg_active = 0; return; }
    Uint8 alpha = (elapsed < 1500) ? 255 : (Uint8)(255 - (elapsed-1500)*255/500);
    int ay = 200 - (int)(elapsed * 50 / 2000);
    SDL_Color col = b->msg_color;
    col.a = alpha;
    int tw, th;
    TTF_SizeText(b->fontHelp, b->msg_text, &tw, &th);
    drawLine(re, b->fontHelp, col, b->msg_text, (1920-tw)/2, ay);
}

/* ============================================================
   SCROLLING - suit la position du joueur
   ============================================================ */
void scrolling(Background *b, const Uint8 *keystate)
{
    (void)keystate; /* Le scrolling est géré par setCamFromPlayer */
    b->rainY += 8.0f;
    if (b->rainY >= 1080) b->rainY = 0;
}

/* Appelé depuis main.c avec la position du joueur */
void setCamFromPlayer(Background *b, float player_wx)
{
    int cam = (int)player_wx - WINDOW_W / 2;
    if (cam < 0) cam = 0;
    if (cam > WORLD_W - WINDOW_W) cam = WORLD_W - WINDOW_W;
    b->cam1.x = cam;
    b->cam1.y = 0;
}

/* ============================================================
   AFFICHER BACKGROUND
   ============================================================ */
void afficherBack(Background *b, SDL_Renderer *re, int playerNum)
{
    SDL_Rect *cam;
    SDL_Rect  view;

    if (b->mode == 0) {
        cam  = &b->cam1;
        view = (SDL_Rect){0,0,1920,1080};
    } else {
        if (playerNum == 1) { cam = &b->cam2; view = (SDL_Rect){0,0,960,1080}; }
        else                { cam = &b->cam1; view = (SDL_Rect){960,0,960,1080}; }
    }

    SDL_RenderSetViewport(re, &view);

    /* Ciel */
    SDL_RenderCopy(re, b->sky, NULL, NULL);

    /* 4 bâtiments alternés */
    for (int i = 0; i < 12; i++) {
        SDL_Texture *img = b->bat[i % 4];
        SDL_Rect r = {(i*1920) - (int)(cam->x*0.6f), -cam->y, 1920, 1080};
        SDL_RenderCopy(re, img, NULL, &r);
    }

    /* Sol 1 ×5 */
    for (int i = 0; i < 5; i++) {
        SDL_Rect r = {(i*1920) - cam->x, -cam->y, 1920, 1080};
        SDL_RenderCopy(re, b->ground1, NULL, &r);
    }

    /* Sol 2 (fin) */
    SDL_Rect s2 = {(5*1920) - cam->x, -cam->y, 1920, 1080};
    SDL_RenderCopy(re, b->ground2, NULL, &s2);

    /* Pluie */
    SDL_Rect r1 = {0, (int)b->rainY,        1920, 1080};
    SDL_Rect r2 = {0, (int)b->rainY - 1080, 1920, 1080};
    SDL_RenderCopy(re, b->rain, NULL, &r1);
    SDL_RenderCopy(re, b->rain, NULL, &r2);

    SDL_RenderSetViewport(re, NULL);
}

/* ============================================================
   AFFICHER TEMPS
   ============================================================ */
void afficherTemps(Background *b, SDL_Renderer *re, int playerNum)
{
    if (!b->font) return;
    Uint32 sec = (SDL_GetTicks() - b->startTime) / 1000;
    char buf[12];
    sprintf(buf, "%02d:%02d", sec/60, sec%60);
    SDL_Surface *s = TTF_RenderText_Blended(b->font, buf, b->color);
    SDL_Texture *t = SDL_CreateTextureFromSurface(re, s);
    int px = (b->mode == 0) ? 1880 - s->w :
             (playerNum == 1) ? 920 - s->w : 1880 - s->w;
    SDL_Rect dst = {px, 30, s->w, s->h};
    SDL_RenderCopy(re, t, NULL, &dst);
    SDL_FreeSurface(s); SDL_DestroyTexture(t);
}

/* ============================================================
   MINIMAP
   ============================================================ */
void initMiniMap(Background *b)
{
    int mW = 300;
    int mH = 170;
    b->mapLargeur = WORLD_W;
    b->mapHauteur = WINDOW_H;
    /* Position : haut gauche, sous la ligne des jetons (y=60) */
    b->miniRect   = (SDL_Rect){10, 65, mW, mH};
    b->scaleX     = (float)mW / (float)WINDOW_W;
    b->scaleY     = (float)mH / (float)WINDOW_H;
}

static void afficherMiniMapPourCam(Background *b, SDL_Renderer *re,
                                    SDL_Rect *cam, int offsetX,
                                    float px, float py)
{
    (void)py;
    int mW = b->miniRect.w, mH = b->miniRect.h;

    /* Viewport de la minimap */
    SDL_Rect miniView = {offsetX, b->miniRect.y, mW, mH};
    SDL_RenderSetViewport(re, &miniView);

    /* Échelle pour réduire le background */
    float sx = (float)mW / (float)WINDOW_W;
    float sy = (float)mH / (float)WINDOW_H;
    SDL_RenderSetScale(re, sx, sy);

    /* Rendre exactement le même background qu'à l'écran */
    SDL_RenderCopy(re, b->sky, NULL, NULL);

    for (int i = 0; i < 12; i++) {
        SDL_Texture *img = b->bat[i % 4];
        SDL_Rect r = {(i*1920) - (int)(cam->x*0.6f), -cam->y, 1920, 1080};
        SDL_RenderCopy(re, img, NULL, &r);
    }

    for (int i = 0; i < 5; i++) {
        SDL_Rect r = {(i*1920) - cam->x, -cam->y, 1920, 1080};
        SDL_RenderCopy(re, b->ground1, NULL, &r);
    }

    SDL_Rect s2 = {(5*1920) - cam->x, -cam->y, 1920, 1080};
    SDL_RenderCopy(re, b->ground2, NULL, &s2);

    SDL_Rect r1 = {0, (int)b->rainY,        1920, 1080};
    SDL_Rect r2 = {0, (int)b->rainY - 1080, 1920, 1080};
    SDL_RenderCopy(re, b->rain, NULL, &r1);
    SDL_RenderCopy(re, b->rain, NULL, &r2);

    /* Reset scale */
    SDL_RenderSetScale(re, 1.0f, 1.0f);

    /* Point joueur (rouge) par dessus */
    int dotX = (int)((px - cam->x) * sx);
    int dotY = (int)(GROUND_Y * sy) - 4;
    if (dotX > 0 && dotX < mW) {
        SDL_SetRenderDrawColor(re, 255, 50, 50, 255);
        SDL_Rect dot = {dotX-5, dotY-5, 10, 10};
        SDL_RenderFillRect(re, &dot);
    }

    /* Bordure */
    SDL_SetRenderDrawColor(re, 200, 200, 255, 255);
    SDL_Rect border = {0, 0, mW, mH};
    SDL_RenderDrawRect(re, &border);

    SDL_RenderSetViewport(re, NULL);
}

void afficherMiniMap(Background *b, SDL_Renderer *re,
                     float p1x, float p1y, float p2x, float p2y)
{
    if (b->mode == 0) {
        afficherMiniMapPourCam(b, re, &b->cam1, 10, p1x, p1y);
    } else {
        SDL_Rect saved = b->miniRect;
        float savedSX  = b->scaleX;
        b->miniRect.w  = 960/4;
        b->miniRect.h  = 1080/4;
        b->scaleX      = (float)b->miniRect.w / 1920.0f;
        afficherMiniMapPourCam(b, re, &b->cam2,  10, p1x, p1y);
        afficherMiniMapPourCam(b, re, &b->cam1, 970, p2x, p2y);
        b->miniRect = saved;
        b->scaleX   = savedSX;
    }
}

/* ============================================================
   HELP
   ============================================================ */
void toggleHelp(Background *b) {
    b->showHelp = !b->showHelp;
    if (b->showHelp) b->pauseTime = SDL_GetTicks();
    else { b->startTime += SDL_GetTicks() - b->pauseTime; b->pauseTime = 0; }
}

void afficherHelp(Background *b, SDL_Renderer *re)
{
    if (!b->showHelp) return;

    int winW=1920, winH=1080;
    int panW=winW-200, panH=winH-160;
    int panX=(winW-panW)/2, panY=(winH-panH)/2;
    int colG, colD, y, yG, yD, lh;
    SDL_Color cT, cS, cL, cP;
    SDL_Rect fs, panel, p2;

    cT.r=255; cT.g=220; cT.b= 80; cT.a=255;
    cS.r=120; cS.g=200; cS.b=255; cS.a=255;
    cL.r=255; cL.g=255; cL.b=255; cL.a=255;
    cP.r=255; cP.g= 80; cP.b= 80; cP.a=255;

    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    fs.x=0; fs.y=0; fs.w=winW; fs.h=winH;
    SDL_SetRenderDrawColor(re,0,0,0,120);
    SDL_RenderFillRect(re,&fs);

    panel.x=panX; panel.y=panY; panel.w=panW; panel.h=panH;
    SDL_SetRenderDrawColor(re,10,10,40,210);
    SDL_RenderFillRect(re,&panel);
    SDL_SetRenderDrawColor(re,180,180,255,255);
    SDL_RenderDrawRect(re,&panel);
    p2.x=panX+1; p2.y=panY+1; p2.w=panW-2; p2.h=panH-2;
    SDL_SetRenderDrawColor(re,100,100,200,180);
    SDL_RenderDrawRect(re,&p2);
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);

    colG = panX+60;
    colD = panX+panW/2+30;
    y    = panY+40;

    {
        char titre[] = "GUIDE DES COMMANDES";
        int tw, th;
        TTF_SizeUTF8(b->fontHelp, titre, &tw, &th);
        drawLine(re, b->fontHelp, cT, titre, panX+(panW-tw)/2, y);
        y += th + 30;
    }

    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(re,180,180,255,120);
    SDL_RenderDrawLine(re, panX+40, y, panX+panW-40, y);
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
    y += 20;

    yG = y; yD = y;

    /* Colonne gauche - SOLO */
    { char s[] = "MODE SOLO";
      lh = drawLine(re, b->fontHelp, cS, s, colG, yG); yG += lh+18; }

    { char l[64];
      sprintf(l, "\xe2\x86\x92  : DEPLACEMENT DROITE");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "\xe2\x86\x90  : DEPLACEMENT GAUCHE");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "SHIFT  : COURIR");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "ESPACE : SAUTER");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "M      : PUNCH");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "P      : KICK");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "T      : TIR");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
      sprintf(l, "H      : AIDE / PAUSE");
      lh=drawLine(re,b->fontHelpText,cL,l,colG+20,yG); yG+=lh+10;
    }

    /* Colonne droite - COLLECTIBLES */
    { char s[] = "COLLECTIBLES";
      lh = drawLine(re, b->fontHelp, cS, s, colD, yD); yD += lh+18; }

    { char l[64];
      sprintf(l, "Piece 1DT  : +10 score");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Piece 5DT  : +50 score");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Diamant    : +100 score");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Sac sang   : Vie complete");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Box ammo   : 20 bullets");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Gilet anti : Costume change");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Bombe ciel : MORT instant");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
      sprintf(l, "Mine sol   : Chien mort");
      lh=drawLine(re,b->fontHelpText,cL,l,colD+20,yD); yD+=lh+10;
    }

    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(re,180,180,255,100);
    SDL_RenderDrawLine(re, panX+panW/2, panY+130, panX+panW/2, panY+panH-60);
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);

    { char pm[] = "[ JEU EN PAUSE  -  Appuyez sur H pour reprendre ]";
      int tw, th, py2;
      TTF_SizeUTF8(b->fontHelpText, pm, &tw, &th);
      py2 = panY + panH - th - 25;
      SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(re,120,0,0,160);
      SDL_Rect pauseBg = {panX+(panW-tw)/2-16, py2-8, tw+32, th+16};
      SDL_RenderFillRect(re, &pauseBg);
      SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
      drawLine(re, b->fontHelpText, cP, pm, panX+(panW-tw)/2, py2);
    }
}

/* ============================================================
   SCORE FENETRE
   ============================================================ */
void initScore(Background *b, SDL_Renderer *re)
{
    (void)re;
    memset(b->inputName1,0,64); b->inputLen1=0; b->showScore1=0; b->validated1=0;
    memset(b->inputName2,0,64); b->inputLen2=0; b->showScore2=0; b->validated2=0;
    b->fontScore     = TTF_OpenFont("arial(1).ttf", 48);
    b->fontScoreText = TTF_OpenFont("arial(1).ttf", 30);
    loadScores(b);
}

void triggerScore(Background *b, int playerNum) {
    if (playerNum==1 && !b->showScore1) {
        memset(b->inputName1,0,64); b->inputLen1=0;
        b->showScore1=1; b->validated1=0; SDL_StartTextInput();
    }
    if (playerNum==2 && !b->showScore2) {
        memset(b->inputName2,0,64); b->inputLen2=0;
        b->showScore2=1; b->validated2=0; SDL_StartTextInput();
    }
}

void handleScoreEvent(Background *b, SDL_Event *e)
{
    int p1 = b->showScore1 && !b->validated1;
    int p2 = b->showScore2 && !b->validated2;
    if (e->type == SDL_TEXTINPUT) {
        if (p1 && b->inputLen1<20) { strcat(b->inputName1,e->text.text); b->inputLen1=(int)strlen(b->inputName1); }
        else if (p2 && b->inputLen2<20) { strcat(b->inputName2,e->text.text); b->inputLen2=(int)strlen(b->inputName2); }
    }
    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.scancode==SDL_SCANCODE_BACKSPACE) {
            if (p1 && b->inputLen1>0) b->inputName1[--b->inputLen1]='\0';
            else if (p2 && b->inputLen2>0) b->inputName2[--b->inputLen2]='\0';
        }
        if (e->key.keysym.scancode==SDL_SCANCODE_RETURN) {
            if (p1 && b->inputLen1>0) {
                ScoreEntry *ne = (b->scoreCount<100)?&b->scores[b->scoreCount++]:&b->scores[99];
                Uint32 sec=(SDL_GetTicks()-b->startTime)/1000;
                strncpy(ne->name,b->inputName1,63); ne->score=(int)sec;
                trierScores(b); saveScores(b); b->validated1=1;
            }
            if (p2 && b->inputLen2>0) {
                ScoreEntry *ne = (b->scoreCount<100)?&b->scores[b->scoreCount++]:&b->scores[99];
                Uint32 sec=(SDL_GetTicks()-b->startTime)/1000;
                strncpy(ne->name,b->inputName2,63); ne->score=(int)sec;
                trierScores(b); saveScores(b); b->validated2=1;
            }
        }
    }
}

void afficherScore(Background *b, SDL_Renderer *re)
{
    if (!b->showScore1 && !b->showScore2) return;
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(re,0,0,0,180);
    SDL_Rect fs={0,0,1920,1080};
    SDL_RenderFillRect(re,&fs);
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
    SDL_Color col={255,215,0,255};
    char buf[64];
    sprintf(buf,"Score enregistré !");
    drawLine(re,b->fontScore,col,buf,800,500);
}

void libererScore(Background *b) {
    if (b->fontScore)     TTF_CloseFont(b->fontScore);
    if (b->fontScoreText) TTF_CloseFont(b->fontScoreText);
}

/* ============================================================
   HEALTH BAR
   ============================================================ */
void initHealthBar(HealthBar *h, int x, int y, int w, int hgt, int hpMax) {
    h->hp=hpMax; h->hpMax=hpMax; h->x=x; h->y=y; h->w=w; h->h=hgt;
}

void takeDamage(HealthBar *h, int dmg) {
    h->hp -= dmg;
    if (h->hp < 0) h->hp = 0;
}

void afficherHealthBar(HealthBar *h, SDL_Renderer *re)
{
    float ratio = (float)h->hp / (float)h->hpMax;
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_BLEND);
    SDL_Rect bg = {h->x, h->y, h->w, h->h};
    SDL_SetRenderDrawColor(re,40,40,40,220);
    SDL_RenderFillRect(re,&bg);
    int fw = (int)(h->w * ratio);
    for (int col = 0; col < fw; col++) {
        float t = (float)col/(float)(h->w>1?h->w-1:1);
        Uint8 r,g;
        if (t<0.5f){r=220;g=(Uint8)(t*2.0f*220);}
        else{r=(Uint8)((1.0f-(t-0.5f)*2.0f)*220);g=220;}
        SDL_SetRenderDrawColor(re,r,g,30,255);
        SDL_RenderDrawLine(re,h->x+col,h->y,h->x+col,h->y+h->h-1);
    }
    SDL_SetRenderDrawColor(re,200,200,200,255);
    SDL_RenderDrawRect(re,&bg);
    SDL_SetRenderDrawBlendMode(re, SDL_BLENDMODE_NONE);
}

/* ============================================================
   LIBERER HUD
   ============================================================ */
void libererHUD(Background *b)
{
    if (b->tex_1dt_fixe)      SDL_DestroyTexture(b->tex_1dt_fixe);
    if (b->tex_5dt_fixe)      SDL_DestroyTexture(b->tex_5dt_fixe);
    if (b->tex_diamond_fixe)  SDL_DestroyTexture(b->tex_diamond_fixe);
    if (b->tex_bullet_score)  SDL_DestroyTexture(b->tex_bullet_score);
    if (b->tex_1dt_sheet)     SDL_DestroyTexture(b->tex_1dt_sheet);
    if (b->tex_5dt_sheet)     SDL_DestroyTexture(b->tex_5dt_sheet);
    if (b->tex_diamond_sheet) SDL_DestroyTexture(b->tex_diamond_sheet);
    if (b->tex_blood_sheet)   SDL_DestroyTexture(b->tex_blood_sheet);
    if (b->tex_ammo_sheet)    SDL_DestroyTexture(b->tex_ammo_sheet);
    if (b->tex_antiball_sheet)SDL_DestroyTexture(b->tex_antiball_sheet);
}

/* ============================================================
   LIBERER BACKGROUND
   ============================================================ */
void libererBack(Background *b)
{
    if (b->sky)       SDL_DestroyTexture(b->sky);
    for (int i=0;i<4;i++) if(b->bat[i]) SDL_DestroyTexture(b->bat[i]);
    if (b->ground1)   SDL_DestroyTexture(b->ground1);
    if (b->ground2)   SDL_DestroyTexture(b->ground2);
    if (b->rain)      SDL_DestroyTexture(b->rain);
    if (b->territoire)SDL_DestroyTexture(b->territoire);
    if (b->rainSound) Mix_FreeMusic(b->rainSound);
    if (b->font)      TTF_CloseFont(b->font);
    if (b->fontHelp)  TTF_CloseFont(b->fontHelp);
    if (b->fontHelpText) TTF_CloseFont(b->fontHelpText);
    if (b->fontBig)   TTF_CloseFont(b->fontBig);
    if (b->fontHUD)   TTF_CloseFont(b->fontHUD);
    libererHUD(b);
    libererBombes(b);
    libererMines(b);
}
