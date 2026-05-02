#include "joueur.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
   ANIMATION
   ============================================================ */
Animation loadAnim(SDL_Renderer *re, const char *path,
                   int cols, int rows, int frame_delay, int loop)
{
    Animation a = {0};
    a.texture = IMG_LoadTexture(re, path);
    if (!a.texture)
        fprintf(stderr, "ERREUR texture: %s -> %s\n", path, IMG_GetError());
    a.info.cols         = cols;
    a.info.rows         = rows;
    a.info.total_frames = cols * rows;
    if (a.texture) {
        int tw, th;
        SDL_QueryTexture(a.texture, NULL, NULL, &tw, &th);
        a.info.frame_w = tw / cols;
        a.info.frame_h = th / rows;
    }
    a.frame_delay = frame_delay;
    a.loop        = loop;
    return a;
}

void freeAnim(Animation *a) {
    if (a->texture) { SDL_DestroyTexture(a->texture); a->texture = NULL; }
}

void updateAnim(Animation *a) {
    if (!a->texture || a->finished) return;
    Uint32 now = SDL_GetTicks();
    if (now - a->last_time >= (Uint32)a->frame_delay) {
        a->last_time = now;
        a->current_frame++;
        if (a->current_frame >= a->info.total_frames) {
            if (a->loop) a->current_frame = 0;
            else { a->current_frame = a->info.total_frames - 1; a->finished = 1; }
        }
    }
}

void renderAnim(Animation *a, SDL_Renderer *re,
                SDL_Rect *dst, SDL_RendererFlip flip)
{
    if (!a->texture) return;
    int col = a->current_frame % a->info.cols;
    int row = a->current_frame / a->info.cols;
    SDL_Rect src = { col * a->info.frame_w, row * a->info.frame_h,
                     a->info.frame_w, a->info.frame_h };
    SDL_RenderCopyEx(re, a->texture, &src, dst, 0, NULL, flip);
}

void resetAnim(Animation *a) {
    a->current_frame = 0;
    a->last_time     = SDL_GetTicks();
    a->finished      = 0;
}

/* ============================================================
   CHARGEMENT ANIMS JOUEUR
   ============================================================ */
static void buildPath(char *out, size_t sz,
                      PersonnageType perso, CostumeType costume,
                      const char *filename)
{
    static const char *john_dirs[] = {
        "assets/john/costume",
        "assets/john/costume_avec_antiball",
        "assets/john/sans_chemise",
        "assets/john/sans_chemise_avec_antiball",
        "assets/john/sans_costume",
        "assets/john/sans_costume_avec_antiball"
    };
    static const char *helen_dirs[] = {
        "assets/helen/costume",
        "assets/helen/costume_avec_antiball",
        "assets/helen/chemise",
        "assets/helen/chemise_avec_antiball"
    };
    const char *dir = "assets";
    if (perso == PERSO_JOHN) {
        int idx = (int)costume;
        if (idx >= 0 && idx < 6) dir = john_dirs[idx];
    } else {
        int idx = (int)costume - (int)COSTUME_HELEN;
        if (idx >= 0 && idx < 4) dir = helen_dirs[idx];
    }
    snprintf(out, sz, "%s/%s", dir, filename);
}

static void getSheetDims(PersonnageType perso, CostumeType costume,
                         AnimType anim, int *cols, int *rows)
{
    *cols = 7; *rows = 7;
    if (perso == PERSO_JOHN && costume == COSTUME_JOHN) {
        /* john/costume : 8x8 sauf exceptions 7x7 */
        if (anim == ANIM_IDLE || anim == ANIM_WALK || anim == ANIM_RUN) {
            *cols = 8; *rows = 8;
        }
        /* john_jump.png est 8x8 aussi */
        if (anim == ANIM_JUMP) { *cols = 8; *rows = 8; }
    } else if (perso == PERSO_HELEN && costume == COSTUME_HELEN) {
        if (anim == ANIM_JUMP || anim == ANIM_SHOOT) { *cols = 5; *rows = 5; }
    }
}

static void getAnimFilename(PersonnageType perso, CostumeType costume,
                            AnimType anim, char *out, size_t sz)
{
    if (perso == PERSO_JOHN) {
        switch (costume) {
        case COSTUME_JOHN: {
            const char *n[] = {
                "john_idle.png","john_walk.png","john_run.png",
                "john_jump.png","john_kick.png","john_punch.png",
                "john_shoot.png","john_death.png","john_victory.png","john_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case COSTUME_JOHN_ANTIBALL: {
            const char *n[] = {
                "john_wick_avec_antiball_idle.png","john_wick_avec_antiball_walk.png",
                "john_wick_avec_antiball_run.png","john_wick_avec_antiball_jump.png",
                "john_wick_avec_antiball_kick.png","john_wick_avec_antiball_punch.png",
                "john_wick_avec_antiball_tir.png","john_wick_avec_antiball_mort.png",
                "john_wick_avec_antiball_idle.png","john_wick_avec_antiball_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case JOHN_SANS_CHEMISE: {
            const char *n[] = {
                "john sans veste-idle-v1.png","john sans veste-walk-v1.png",
                "john sans veste-run-v1.png","john sans veste-jump-v1.png",
                "john sans veste-kick-v1.png","john sans veste-punch-v1.png",
                "john sans veste-gun_shoot-v1.png","john sans veste-death-v1.png",
                "john sans veste-idle-v1.png","john sans veste-idle-v1.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case JOHN_SANS_CHEMISE_ANTIBALL: {
            const char *n[] = {
                "john_sans_veste_avec_jilet_idle.png","john_sans_veste_avec_jilet_walk.png",
                "john_san_sveste_avec_jilet_run.png","john_sans_veste_avec_jilet_jump.png",
                "john_sans_veste_avec_jilet_kick.png","john_sans_veste_avec_jilet_punch.png",
                "john_sans_vest_avec_jilet_gun_shoot.png","john_sans_veste_avec_jilet_death.png",
                "john_sans_veste_avec_jilet_idle.png","john_sans_veste_avec_jilet_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case JOHN_SANS_COSTUME: {
            const char *n[] = {
                "john_sans_vetements_idle.png","john_sans_vetements_walk.png",
                "john_sans_vetements_run.png","john_sans_vetements_jump.png",
                "john_sans_vetements_kick.png","john_sans_vetements_punch.png",
                "john_sans_vetements_gun_shoot.png","john_sans_vetements_death.png",
                "john_sans_vetements_idle.png","john_sans_vetements_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case JOHN_SANS_COSTUME_ANTIBALL: {
            const char *n[] = {
                "john_wick_sans_vetements_avec_antiball_idle.png",
                "john_wick_sans_vetements_avec_antiball_walk.png",
                "john_wick_sans_vetements_avec_antiball_run.png",
                "john_wick_sans_vetements_avec_antiball_jump.png",
                "john_wick_sans_vetements_avec_antiball_kick.png",
                "john_wick_sans_vetements_avec_antiball_punch.png",
                "john_wick_sans_vetements_avec_antiball_gun_shoo.png",
                "john_wick_sans_vetements_avec_antiball_death.png",
                "john_wick_sans_vetements_avec_antiball_idle.png",
                "john_wick_sans_vetements_avec_antiball_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        default: snprintf(out, sz, "john_idle.png");
        }
    } else {
        switch (costume) {
        case COSTUME_HELEN: {
            const char *n[] = {
                "helen_idle.png","helen_walk.png","helen_run.png",
                "helen_jump.png","helen_kick.png","helen_punch.png",
                "helen_shoot.png","helen_idle.png","helen_victory.png","helen_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case COSTUME_HELEN_ANTIBALL: {
            const char *n[] = {
                "helen_avec_jilet_idle.png","helen_avec_jilet_walk.png",
                "helen_avec_jilet_run.png","helen_avec_jilet_jump_v1.png",
                "helen_avec_jilet_kick.png","helen_avec_jilet_punch.png",
                "helen_avec_jilet_idle.png","helen_avec_jilet_death.png",
                "helen_avec_jilet_idle.png","helen_avec_jilet_attack.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case HELEN_CHEMISE: {
            const char *n[] = {
                "helen_sans_veste_idle.png","helen_sans_veste_walk.png",
                "helen_sans_veste_run.png","helen_sans_veste_jump.png",
                "helen_sans_veste_kick.png","helen_sans_veste_punch.png",
                "helen_sans_veste_gun_shoot.png","helen_sans_veste_death.png",
                "helen_sans_veste_idle.png","helen_sans_veste_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        case HELEN_CHEMISE_ANTIBALL: {
            const char *n[] = {
                "helen_sans_veste_avec_jilet_antiball_idle.png",
                "helen_sans_veste_avec_jilet_antiball_walk.png",
                "helen_sans_veste_avec_jilet_antiball_run.png",
                "helen_sans_veste_avec_jilet_anti_ball_jump.png",
                "helen_sans_veste_avec_jilet_antiball_kick.png",
                "helen_sans_veste_avec_jilet_antiball_punch.png",
                "helen_sans_veste_avec_jilet_antiball_gun_shoot.png",
                "helen_sans_veste_avec_jilet_antiball_death.png",
                "helen_sans_veste_avec_jilet_antiball_idle.png",
                "helen_sans_veste_avec_jilet_antiball_idle.png"
            };
            snprintf(out, sz, "%s", n[anim]);
        } break;
        default: snprintf(out, sz, "helen_idle.png");
        }
    }
}

static void loadAllAnims(Joueur *j, SDL_Renderer *re)
{
    char path[512], fname[256];
    int cols, rows;
    /*           idle walk run  jump kick punch shoot death victory attack */
    int delays[] = { 120, 100,  70,  80,  22,   22,   22,  150,   80,   22 };
    int loops[]  = {   1,   1,   1,   0,   0,    0,    0,    0,    1,    0 };

    for (int a = 0; a < ANIM_COUNT; a++) {
        getSheetDims(j->perso, j->costume, (AnimType)a, &cols, &rows);
        getAnimFilename(j->perso, j->costume, (AnimType)a, fname, sizeof(fname));
        buildPath(path, sizeof(path), j->perso, j->costume, fname);
        j->anims[a] = loadAnim(re, path, cols, rows, delays[a], loops[a]);
    }
}

/* ============================================================
   INIT JOUEUR
   ============================================================ */
void joueurInit(Joueur *j, SDL_Renderer *re,
                PersonnageType perso, CostumeType costume)
{
    memset(j, 0, sizeof(Joueur));
    j->perso        = perso;
    j->costume      = costume;
    j->wx           = 200.0f;
    j->wy           = (float)GROUND_Y;
    j->on_ground    = 1;
    j->facing_right = 1;
    j->hp           = 100;
    j->hp_max       = 100;
    j->alive        = 1;
    j->walk_channel  = -1;
    j->run_channel   = -1;
    j->punch_channel = -1;
    j->kick_channel  = -1;
    j->gun_channel   = -1;
    j->key_punch = 0; j->key_kick = 0; j->key_shoot = 0;

    loadAllAnims(j, re);

    j->bullet_tex = IMG_LoadTexture(re, "assets/back1/bullet.png");

    j->snd_walk   = Mix_LoadWAV("assets/sounds/walk.wav");
    j->snd_run    = Mix_LoadWAV("assets/sounds/run.wav");
    j->snd_punch  = Mix_LoadWAV("assets/sounds/punch1.wav");
    j->snd_punch2 = Mix_LoadWAV("assets/sounds/punch2.wav");
    j->snd_kick   = Mix_LoadWAV("assets/sounds/kick.wav");
    j->snd_gun    = Mix_LoadWAV("assets/sounds/gun.wav");
    j->snd_death  = Mix_LoadWAV("assets/sounds/death.wav");
}

void joueurSetAnim(Joueur *j, AnimType anim) {
    if (j->current_anim == anim) return;
    j->current_anim = anim;
    resetAnim(&j->anims[anim]);
}

/* ============================================================
   BULLET JOUEUR
   ============================================================ */
void joueurFireBullet(Joueur *j)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!j->bullets[i].active) {
            j->bullets[i].active  = 1;
            j->bullets[i].x       = j->wx + (j->facing_right ? 100 : -100);
            j->bullets[i].y       = j->wy - 200;
            j->bullets[i].vx      = j->facing_right ? BULLET_SPEED : -BULLET_SPEED;
            j->bullets[i].w       = 60;
            j->bullets[i].h       = 25;
            j->bullets[i].texture = j->bullet_tex;
            /* Son tir */
            if (j->snd_gun) Mix_PlayChannel(-1, j->snd_gun, 0);
            break;
        }
    }
}

void joueurUpdateBullets(Joueur *j) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!j->bullets[i].active) continue;
        j->bullets[i].x += j->bullets[i].vx;
        if (j->bullets[i].x < 0 || j->bullets[i].x > WORLD_W)
            j->bullets[i].active = 0;
    }
}

void joueurRenderBullets(Joueur *j, SDL_Renderer *re, int cam_x) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!j->bullets[i].active) continue;
        SDL_Rect r = { (int)j->bullets[i].x - cam_x,
                       (int)j->bullets[i].y,
                       j->bullets[i].w, j->bullets[i].h };
        if (j->bullets[i].texture) {
            SDL_RendererFlip flip = (j->bullets[i].vx < 0) ?
                SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            SDL_RenderCopyEx(re, j->bullets[i].texture, NULL, &r, 0, NULL, flip);
        } else {
            SDL_SetRenderDrawColor(re, 255, 220, 50, 255);
            SDL_RenderFillRect(re, &r);
        }
    }
}

/* ============================================================
   INPUT
   ============================================================ */
void joueurHandleInput(Joueur *j, const Uint8 *keys, SDL_Event *ev)
{
    (void)keys;
    if (!j->alive) return;
    if (!ev) return;

    if (ev->type == SDL_KEYDOWN && !ev->key.repeat) {
        SDL_Scancode sc = ev->key.keysym.scancode;

        if (sc == SDL_SCANCODE_SPACE && j->on_ground) {
            j->vy        = JUMP_FORCE;
            j->on_ground = 0;
            joueurSetAnim(j, ANIM_JUMP);
        }
        /* M = Punch */
        if (sc == SDL_SCANCODE_M) {
            j->key_punch = 1;
            Mix_Chunk *snd = (rand()%2) ? j->snd_punch : j->snd_punch2;
            if (snd) Mix_PlayChannel(-1, snd, 0);
        }
        /* P = Kick */
        if (sc == SDL_SCANCODE_P) {
            j->key_kick = 1;
            if (j->snd_kick) Mix_PlayChannel(-1, j->snd_kick, 0);
        }
        /* T = Shoot */
        if (sc == SDL_SCANCODE_T) {
            j->key_shoot = 1;
            joueurFireBullet(j);
            if (j->snd_gun) Mix_PlayChannel(-1, j->snd_gun, 0);
        }
    }

    if (ev->type == SDL_KEYUP) {
        SDL_Scancode sc = ev->key.keysym.scancode;
        if (sc == SDL_SCANCODE_M) {
            j->key_punch = 0;
            if (j->current_anim == ANIM_PUNCH) joueurSetAnim(j, ANIM_IDLE);
        }
        if (sc == SDL_SCANCODE_P) {
            j->key_kick = 0;
            if (j->current_anim == ANIM_KICK) joueurSetAnim(j, ANIM_IDLE);
        }
        if (sc == SDL_SCANCODE_T) {
            j->key_shoot = 0;
            if (j->current_anim == ANIM_SHOOT) joueurSetAnim(j, ANIM_IDLE);
        }
    }
}
/* ============================================================
   UPDATE JOUEUR
   ============================================================ */
void joueurUpdate(Joueur *j)
{
    /* --- MORT --- */
    if (!j->alive) {
        if (!j->anims[ANIM_DEATH].finished)
            updateAnim(&j->anims[ANIM_DEATH]);
        if (j->walk_channel  != -1) { Mix_HaltChannel(j->walk_channel);  j->walk_channel  = -1; }
        if (j->run_channel   != -1) { Mix_HaltChannel(j->run_channel);   j->run_channel   = -1; }
        if (j->punch_channel != -1) { Mix_HaltChannel(j->punch_channel); j->punch_channel = -1; }
        if (j->kick_channel  != -1) { Mix_HaltChannel(j->kick_channel);  j->kick_channel  = -1; }
        if (j->gun_channel   != -1) { Mix_HaltChannel(j->gun_channel);   j->gun_channel   = -1; }
        joueurUpdateBullets(j);
        return;
    }

    /* --- VICTOIRE : sprite en boucle infinie --- */
    if (j->victory) {
        updateAnim(&j->anims[ANIM_VICTORY]);
        joueurUpdateBullets(j);
        return;
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    /* --- Gravité --- */
    j->vy += GRAVITY;
    j->wy += j->vy;
    if (j->wy >= (float)GROUND_Y) {
        j->wy        = (float)GROUND_Y;
        j->vy        = 0;
        j->on_ground = 1;
    }

    /* --- Mouvement horizontal --- */
    int moving  = 0;
    int running = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];

    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        j->wx -= running ? SPEED_RUN : SPEED_WALK;
        j->facing_right = 0;
        moving = 1;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        j->wx += running ? SPEED_RUN : SPEED_WALK;
        j->facing_right = 1;
        moving = 1;
    }
    if (j->wx < 0) j->wx = 0;
    if (j->wx > WORLD_W - 100) j->wx = WORLD_W - 100;

    joueurUpdateBullets(j);

    /* --- Saut --- */
    if (!j->on_ground) {
        if (j->current_anim != ANIM_JUMP) joueurSetAnim(j, ANIM_JUMP);
        updateAnim(&j->anims[ANIM_JUMP]);
        return;
    }

    /* --- Touche maintenue → sprite en boucle --- */
    /* Priorité : shoot > kick > punch */
    if (j->key_shoot) {
        if (j->current_anim != ANIM_SHOOT) {
            joueurSetAnim(j, ANIM_SHOOT);
        }
        updateAnim(&j->anims[ANIM_SHOOT]);
        /* Tirer au frame 3 de chaque cycle */
        if (j->anims[ANIM_SHOOT].current_frame == 3 &&
            j->anims[ANIM_SHOOT].finished == 0) {
            joueurFireBullet(j);
        }
        /* Reboucler l'anim si finie */
        if (j->anims[ANIM_SHOOT].finished) resetAnim(&j->anims[ANIM_SHOOT]);
        return;
    }
    if (j->key_kick) {
        if (j->current_anim != ANIM_KICK) joueurSetAnim(j, ANIM_KICK);
        updateAnim(&j->anims[ANIM_KICK]);
        if (j->anims[ANIM_KICK].finished) resetAnim(&j->anims[ANIM_KICK]);
        return;
    }
    if (j->key_punch) {
        if (j->current_anim != ANIM_PUNCH) joueurSetAnim(j, ANIM_PUNCH);
        updateAnim(&j->anims[ANIM_PUNCH]);
        if (j->anims[ANIM_PUNCH].finished) resetAnim(&j->anims[ANIM_PUNCH]);
        return;
    }

    /* --- Sons déplacement --- */
    if (moving && running) {
        if (j->run_channel == -1 && j->snd_run)
            j->run_channel = Mix_PlayChannel(-1, j->snd_run, -1);
        if (j->walk_channel != -1) { Mix_HaltChannel(j->walk_channel); j->walk_channel = -1; }
    } else if (moving) {
        if (j->walk_channel == -1 && j->snd_walk)
            j->walk_channel = Mix_PlayChannel(-1, j->snd_walk, -1);
        if (j->run_channel != -1) { Mix_HaltChannel(j->run_channel); j->run_channel = -1; }
    } else {
        if (j->walk_channel != -1) { Mix_HaltChannel(j->walk_channel); j->walk_channel = -1; }
        if (j->run_channel  != -1) { Mix_HaltChannel(j->run_channel);  j->run_channel  = -1; }
    }

    /* --- Animation de base --- */
    AnimType wanted = moving ? (running ? ANIM_RUN : ANIM_WALK) : ANIM_IDLE;
    joueurSetAnim(j, wanted);
    updateAnim(&j->anims[j->current_anim]);
}


/* ============================================================
   RENDER JOUEUR
   ============================================================ */
void joueurRender(Joueur *j, SDL_Renderer *re, int cam_x)
{
    /* Taille normale */
    int w = 260, h = 360;

    /* Avec jilet antiball : légèrement plus petit */
    if (j->costume == COSTUME_JOHN_ANTIBALL ||
        j->costume == JOHN_SANS_CHEMISE_ANTIBALL ||
        j->costume == JOHN_SANS_COSTUME_ANTIBALL ||
        j->costume == COSTUME_HELEN_ANTIBALL ||
        j->costume == HELEN_CHEMISE_ANTIBALL) {
        w = 230; h = 320;
    }

    /* Décaler vers le bas pour poser sur le sol */
    SDL_Rect dst = {
        (int)j->wx - cam_x - w / 2,
        (int)j->wy - h + 20,   /* +20 = descendre un peu */
        w, h
    };
    SDL_RendererFlip flip = j->facing_right ?
        SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    AnimType ar = (!j->alive) ? ANIM_DEATH :
                  (j->victory) ? ANIM_VICTORY : j->current_anim;
    renderAnim(&j->anims[ar], re, &dst, flip);

    if (j->alive && !j->victory) {
        SDL_Rect bg = { dst.x, dst.y - 16, HEALTH_BAR_W, HEALTH_BAR_H };
        SDL_Rect fg = { dst.x, dst.y - 16,
                        (int)(HEALTH_BAR_W * j->hp / (float)j->hp_max),
                        HEALTH_BAR_H };
        SDL_SetRenderDrawColor(re, 60, 0, 0, 255);
        SDL_RenderFillRect(re, &bg);
        SDL_SetRenderDrawColor(re, 0, 230, 0, 255);
        SDL_RenderFillRect(re, &fg);
    }
}

void joueurFree(Joueur *j)
{
    for (int a = 0; a < ANIM_COUNT; a++) freeAnim(&j->anims[a]);
    if (j->bullet_tex) SDL_DestroyTexture(j->bullet_tex);
    if (j->snd_walk)   Mix_FreeChunk(j->snd_walk);
    if (j->snd_run)    Mix_FreeChunk(j->snd_run);
    if (j->snd_punch)  Mix_FreeChunk(j->snd_punch);
    if (j->snd_punch2) Mix_FreeChunk(j->snd_punch2);
    if (j->snd_kick)   Mix_FreeChunk(j->snd_kick);
    if (j->snd_gun)    Mix_FreeChunk(j->snd_gun);
    if (j->snd_death)  Mix_FreeChunk(j->snd_death);
}

/* ============================================================
   ENNEMI – chemins
   ============================================================ */
typedef struct { const char *idle,*walk,*run,*jump,*kick,
                             *punch,*shoot,*death,*attack; } EnemyPaths;

static EnemyPaths getEnemyPaths(EnemyType t)
{
    EnemyPaths p = {0};
    switch (t) {
    case ENEMY_TYPE_1:
        p.idle  = "assets/enemie1/enemi1_idle.png";
        p.walk  = "assets/enemie1/enemi1_walk.png";
        p.shoot = "assets/enemie1/enemi1_shootgun.png";
        p.death = "assets/enemie1/enemi1_death.png";
        break;
    case ENEMY_TYPE_2:
        p.idle  = "assets/enemie2/enemie2_idle.png";
        p.walk  = "assets/enemie2/enemie2_walk.png";
        p.shoot = "assets/enemie2/enemie2_shootgun.png";
        p.death = "assets/enemie2/enemie2_death.png";
        break;
    case ENEMY_TYPE_3:
        p.idle  = "assets/enemie3/enemie3_idle.png";
        p.walk  = "assets/enemie3/enemie3_walk.png";
        p.shoot = "assets/enemie3/enemie3_shootgun.png";
        p.death = "assets/enemie3/enemie3_death.png";
        break;
    case ENEMY_LOSEF:
        p.idle  = "assets/losef_tarassov/losef tarasov-idle-v1.png";
        p.walk  = "assets/losef_tarassov/losef tarasov-walk-v1.png";
        p.run   = "assets/losef_tarassov/losef tarasov-run-v1.png";
        p.jump  = "assets/losef_tarassov/losef tarasov-jump-v1.png";
        p.kick  = "assets/losef_tarassov/losef tarasov-kick-v1.png";
        p.punch = "assets/losef_tarassov/losef tarasov-punch-v1.png";
        p.shoot = "assets/losef_tarassov/losef tarasov-gun_shoot-v1.png";
        p.death = "assets/losef_tarassov/losef tarasov-death-v3.png";
        break;
    case ENEMY_VIGGO:
        p.idle   = "assets/viggos_tarasov/viggo_tarasov_idle.png";
        p.walk   = "assets/viggos_tarasov/viggo_tarasov_walk.png";
        p.run    = "assets/viggos_tarasov/viggo_tarasov_run.png";
        p.jump   = "assets/viggos_tarasov/viggo_tarasov_jump.png";
        p.kick   = "assets/viggos_tarasov/viggo_tarasov_kick.png";
        p.punch  = "assets/viggos_tarasov/viggo_tarasov_punch.png";
        p.attack = "assets/viggos_tarasov/viggo_tarasov_attack.png";
        p.death  = "assets/viggos_tarasov/viggo_tarasov_death.png";
        break;
    default: break;
    }
    return p;
}

/* ============================================================
   INIT ENNEMI
   ============================================================ */
void ennemiInit(Ennemi *e, SDL_Renderer *re,
                EnemyType type, float wx, float wy)
{
    memset(e, 0, sizeof(Ennemi));
    e->type         = type;
    e->wx           = wx;
    e->wy           = wy;
    e->alive        = 1;
    e->facing_right = 0;
    e->on_ground    = 1;
    e->ai_timer     = SDL_GetTicks();
    e->hp = e->hp_max = 100; /* 5 tirs/coups pour mourir */
    e->bullet_tex   = IMG_LoadTexture(re, "assets/back1/bullet.png");

    EnemyPaths p = getEnemyPaths(type);
    int dc = (type == ENEMY_LOSEF) ? 5 : 7;
    int dr = (type == ENEMY_LOSEF) ? 5 : 7;

    if (p.idle)   e->anims[ANIM_IDLE]   = loadAnim(re, p.idle,  7,7,120,1);
    if (p.walk)   e->anims[ANIM_WALK]   = loadAnim(re, p.walk,  7,7,100,1);
    if (p.run)    e->anims[ANIM_RUN]    = loadAnim(re, p.run,   7,7, 70,1);
    if (p.jump)   e->anims[ANIM_JUMP]   = loadAnim(re, p.jump,  7,7,100,0);
    if (p.kick)   e->anims[ANIM_KICK]   = loadAnim(re, p.kick,  7,7, 80,0);
    if (p.punch)  e->anims[ANIM_PUNCH]  = loadAnim(re, p.punch, 7,7, 80,0);
    if (p.shoot)  e->anims[ANIM_SHOOT]  = loadAnim(re, p.shoot, 7,7, 80,1);
    if (p.attack) e->anims[ANIM_ATTACK] = loadAnim(re, p.attack,7,7, 80,0);
    if (p.death)  e->anims[ANIM_DEATH]  = loadAnim(re, p.death, dc,dr,150,0);
}

static void ennemiFireBullet(Ennemi *e)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!e->bullets[i].active) {
            e->bullets[i].active  = 1;
            e->bullets[i].x       = e->wx + (e->facing_right ? 100 : -100);
            e->bullets[i].y       = e->wy - 200;
            e->bullets[i].vx      = e->facing_right ? BULLET_SPEED : -BULLET_SPEED;
            e->bullets[i].w       = 50;
            e->bullets[i].h       = 20;
            e->bullets[i].texture = e->bullet_tex;
            break;
        }
    }
}

/* ============================================================
   UPDATE ENNEMI
   ============================================================ */
void ennemiUpdate(Ennemi *e, float player_wx, int cam_x)
{
    /* Vérifier si joueur visible à l'écran */
    int ex_screen = (int)e->wx - cam_x;
    e->ai_active = (ex_screen > -200 && ex_screen < WINDOW_W + 200);

    if (!e->alive) {
        if (!e->anims[ANIM_DEATH].finished)
            updateAnim(&e->anims[ANIM_DEATH]);
        /* Update bullets encore actives */
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (e->bullets[i].active) {
                e->bullets[i].x += e->bullets[i].vx;
                if (e->bullets[i].x < 0 || e->bullets[i].x > WORLD_W)
                    e->bullets[i].active = 0;
            }
        }
        return;
    }

    /* Si pas visible → rester idle, ne pas tirer */
    if (!e->ai_active) {
        e->current_anim = ANIM_IDLE;
        updateAnim(&e->anims[ANIM_IDLE]);
        return;
    }

    /* Gravité */
    e->vy += GRAVITY;
    e->wy += e->vy;
    if (e->wy >= (float)GROUND_Y) {
        e->wy = (float)GROUND_Y;
        e->vy = 0;
        e->on_ground = 1;
    }

    e->facing_right = (player_wx > e->wx);

    Uint32 now = SDL_GetTicks();

    /* Fréquence IA selon type - boss plus agressifs */
    Uint32 ai_delay;
    switch (e->type) {
    case ENEMY_TYPE_1: ai_delay = 800 + rand()%600;  break;
    case ENEMY_TYPE_2: ai_delay = 700 + rand()%500;  break;
    case ENEMY_TYPE_3: ai_delay = 500 + rand()%400;  break;
    case ENEMY_LOSEF:  ai_delay = 400 + rand()%300;  break;
    case ENEMY_VIGGO:  ai_delay = 300 + rand()%200;  break;
    default:           ai_delay = 800 + rand()%600;  break;
    }

    if (now - e->ai_timer > ai_delay) {
        e->ai_timer = now;
        /* Boss tirent plus souvent (état 2 = shoot plus probable) */
        if (e->type == ENEMY_LOSEF || e->type == ENEMY_VIGGO)
            e->ai_state = rand() % 4 < 3 ? 2 : 1; /* 75% shoot */
        else if (e->type == ENEMY_TYPE_3)
            e->ai_state = rand() % 3 < 2 ? 2 : 1; /* 66% shoot */
        else
            e->ai_state = rand() % 3; /* 33% shoot */
    }

    /* Vitesse selon type */
    float spd;
    switch (e->type) {
    case ENEMY_TYPE_3: spd = 8.0f;  break;
    case ENEMY_LOSEF:  spd = 10.0f; break;
    case ENEMY_VIGGO:  spd = 12.0f; break;
    default:           spd = 6.0f;  break;
    }

    AnimType prev = e->current_anim;
    switch (e->ai_state) {
    case 0:
        e->current_anim = ANIM_IDLE;
        break;
    case 1:
        e->wx += e->facing_right ? spd : -spd;
        e->current_anim = ANIM_WALK;
        break;
    case 2:
        e->current_anim = ANIM_SHOOT;
        if (prev == ANIM_SHOOT && e->anims[ANIM_SHOOT].current_frame == 3)
            ennemiFireBullet(e);
        break;
    case 3: /* Losef/Viggo uniquement : avancer ET tirer */
        e->wx += e->facing_right ? spd*0.5f : -spd*0.5f;
        e->current_anim = ANIM_SHOOT;
        if (prev == ANIM_SHOOT && e->anims[ANIM_SHOOT].current_frame == 3)
            ennemiFireBullet(e);
        break;
    }

    if (e->wx < 0) e->wx = 0;
    if (e->wx > WORLD_W - 100) e->wx = WORLD_W - 100;

    updateAnim(&e->anims[e->current_anim]);

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (e->bullets[i].active) {
            e->bullets[i].x += e->bullets[i].vx;
            if (e->bullets[i].x < 0 || e->bullets[i].x > WORLD_W)
                e->bullets[i].active = 0;
        }
    }
}

/* ============================================================
   RENDER ENNEMI
   ============================================================ */
void ennemiRender(Ennemi *e, SDL_Renderer *re, int cam_x)
{
    if (!e->alive && e->anims[ANIM_DEATH].finished) return;

    int w, h;
    switch (e->type) {
    case ENEMY_TYPE_1: w=200; h=280; break;
    case ENEMY_TYPE_2: w=200; h=280; break;
    case ENEMY_TYPE_3: w=260; h=340; break; /* plus grand */
    case ENEMY_LOSEF:  w=280; h=380; break;
    case ENEMY_VIGGO:  w=280; h=380; break;
    default:           w=200; h=280; break;
    }

    SDL_Rect dst = {
        (int)e->wx - cam_x - w / 2,
        (int)e->wy - h + 20,   /* +20 décaler vers le bas */
        w, h
    };
    SDL_RendererFlip flip = e->facing_right ?
        SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    AnimType ar = e->alive ? e->current_anim : ANIM_DEATH;
    renderAnim(&e->anims[ar], re, &dst, flip);
}

void ennemiRenderHealthBar(Ennemi *e, SDL_Renderer *re, int cam_x)
{
    if (!e->alive) return;
    int h;
    switch (e->type) {
    case ENEMY_TYPE_3: h=340; break;
    case ENEMY_LOSEF:
    case ENEMY_VIGGO:  h=380; break;
    default:           h=280; break;
    }
    int bx = (int)e->wx - cam_x - HEALTH_BAR_W/2;
    int by = (int)e->wy - h + 20 - 18;
    SDL_Rect bg = { bx, by, HEALTH_BAR_W, HEALTH_BAR_H };
    SDL_Rect fg = { bx, by,
                    (int)(HEALTH_BAR_W * e->hp / (float)e->hp_max),
                    HEALTH_BAR_H };
    SDL_SetRenderDrawColor(re, 60, 0, 0, 255);
    SDL_RenderFillRect(re, &bg);
    SDL_SetRenderDrawColor(re, 230, 30, 30, 255);
    SDL_RenderFillRect(re, &fg);
}

void ennemiRenderBullets(Ennemi *e, SDL_Renderer *re, int cam_x)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!e->bullets[i].active) continue;
        SDL_Rect r = { (int)e->bullets[i].x - cam_x,
                       (int)e->bullets[i].y,
                       e->bullets[i].w, e->bullets[i].h };
        if (e->bullets[i].texture) {
            SDL_RendererFlip flip = (e->bullets[i].vx < 0) ?
                SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            SDL_RenderCopyEx(re, e->bullets[i].texture, NULL, &r, 0, NULL, flip);
        } else {
            SDL_SetRenderDrawColor(re, 255, 80, 0, 255);
            SDL_RenderFillRect(re, &r);
        }
    }
}

void ennemiRenderGreat(Ennemi *e, SDL_Renderer *re, TTF_Font *font)
{
    if (!e->show_great || !font) return;
    Uint32 now = SDL_GetTicks();
    if (now - e->great_timer > 1500) { e->show_great = 0; return; }

    SDL_Color col = {255, 220, 0, 255};
    SDL_Surface *s = TTF_RenderText_Blended(font, "GREAT!", col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(re, s);
    /* Fade out */
    Uint8 alpha = (Uint8)(255 * (1.0f - (now - e->great_timer) / 1500.0f));
    SDL_SetTextureAlphaMod(t, alpha);
    SDL_Rect dst = { WINDOW_W / 2 - s->w / 2,
                     WINDOW_H / 2 - s->h / 2 - 50, s->w, s->h };
    SDL_RenderCopy(re, t, NULL, &dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

void ennemiFree(Ennemi *e)
{
    for (int a = 0; a < ANIM_COUNT; a++) freeAnim(&e->anims[a]);
    if (e->bullet_tex) { SDL_DestroyTexture(e->bullet_tex); e->bullet_tex = NULL; }
}

/* ============================================================
   CHIEN
   ============================================================ */
void chienInit(Chien *c, SDL_Renderer *re, float wx, float wy)
{
    memset(c, 0, sizeof(Chien));
    c->wx = wx; c->wy = wy; c->alive = 1; c->facing_right = 1;
    c->anims[0] = loadAnim(re, "assets/chien/puppy_idle.png",  7,7,120,1);
    c->anims[1] = loadAnim(re, "assets/chien/puppy_run.png",   7,7, 80,1);
    c->anims[2] = loadAnim(re, "assets/chien/puppy_death.png", 7,7,150,0);
}

void chienUpdate(Chien *c, Joueur *j)
{
    if (!c->alive) { updateAnim(&c->anims[2]); return; }
    if (!j->alive) { c->alive = 0; c->current_anim = 2; return; }

    float target = j->wx - (j->facing_right ? DOG_OFFSET : -DOG_OFFSET);
    float dx = target - c->wx;
    float dist = fabsf(dx);

    /* Chien suit position joueur : idle si proche, run si loin */
    if (dist > 10.0f) {
        float spd = (dist > 200.0f) ? (float)SPEED_RUN : (float)SPEED_WALK;
        c->wx += (dx > 0) ? spd : -spd;
        c->facing_right = (dx > 0);
        c->current_anim = 1; /* run */
    } else {
        c->current_anim = 0; /* idle */
    }

    c->wy = (float)GROUND_Y;
    if (c->wx < 0) c->wx = 0;
    if (c->wx > WORLD_W - 60) c->wx = WORLD_W - 60;
    updateAnim(&c->anims[c->current_anim]);
}

void chienRender(Chien *c, SDL_Renderer *re, int cam_x)
{
    int w = 170, h = 130;
    SDL_Rect dst = {
        (int)c->wx - cam_x - w / 2,
        (int)c->wy - h,
        w, h
    };
    SDL_RendererFlip flip = c->facing_right ?
        SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    renderAnim(&c->anims[c->current_anim], re, &dst, flip);
}

void chienFree(Chien *c) {
    for (int i = 0; i < 3; i++) freeAnim(&c->anims[i]);
}

/* ============================================================
   COLLISIONS
   ============================================================ */
void checkBulletsVsEnemies(Joueur *j, Ennemi *enemies, int nb)
{
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!j->bullets[b].active) continue;
        SDL_Rect br = { (int)j->bullets[b].x, (int)j->bullets[b].y,
                        j->bullets[b].w, j->bullets[b].h };
        for (int e = 0; e < nb; e++) {
            if (!enemies[e].alive) continue;
            int ew = (enemies[e].type >= ENEMY_LOSEF) ? 260 : 220;
            int eh = (enemies[e].type >= ENEMY_LOSEF) ? 360 : 300;
            SDL_Rect er = { (int)enemies[e].wx - ew/2,
                            (int)enemies[e].wy - eh, ew, eh };
            if (SDL_HasIntersection(&br, &er)) {
                j->bullets[b].active = 0;
                enemies[e].hp -= DAMAGE_BULLET;
                if (enemies[e].hp <= 0) {
                    enemies[e].hp = 0;
                    enemies[e].alive = 0;
                    enemies[e].current_anim = ANIM_DEATH;
                    resetAnim(&enemies[e].anims[ANIM_DEATH]);
                    enemies[e].show_great  = 1;
                    enemies[e].great_timer = SDL_GetTicks();
                }
                break;
            }
        }
    }
}

void checkPunchKickVsEnemies(Joueur *j, Ennemi *enemies, int nb)
{
    if (j->current_anim != ANIM_PUNCH && j->current_anim != ANIM_KICK) return;
    /* Zone de frappe devant le joueur */
    int reach = 150;
    SDL_Rect pr = {
        j->facing_right ? (int)j->wx : (int)j->wx - reach,
        (int)j->wy - 350,
        reach, 300
    };
    for (int e = 0; e < nb; e++) {
        if (!enemies[e].alive) continue;
        int ew = (enemies[e].type >= ENEMY_LOSEF) ? 260 : 220;
        int eh = (enemies[e].type >= ENEMY_LOSEF) ? 360 : 300;
        SDL_Rect er = { (int)enemies[e].wx - ew/2,
                        (int)enemies[e].wy - eh, ew, eh };
        if (SDL_HasIntersection(&pr, &er)) {
            enemies[e].hp -= DAMAGE_MELEE;
            if (enemies[e].hp <= 0) {
                enemies[e].hp = 0;
                enemies[e].alive = 0;
                enemies[e].current_anim = ANIM_DEATH;
                resetAnim(&enemies[e].anims[ANIM_DEATH]);
                enemies[e].show_great  = 1;
                enemies[e].great_timer = SDL_GetTicks();
            }
        }
    }
}

void checkEnemyBulletsVsPlayer(Ennemi *enemies, int nb, Joueur *j)
{
    if (!j->alive) return;
    /* Pas de collision si joueur en saut */
    if (!j->on_ground) return;

    SDL_Rect pr = { (int)j->wx - 100, (int)j->wy - 380, 200, 380 };
    for (int e = 0; e < nb; e++) {
        for (int b = 0; b < MAX_BULLETS; b++) {
            if (!enemies[e].bullets[b].active) continue;
            SDL_Rect br = { (int)enemies[e].bullets[b].x,
                            (int)enemies[e].bullets[b].y,
                            enemies[e].bullets[b].w,
                            enemies[e].bullets[b].h };
            if (SDL_HasIntersection(&br, &pr)) {
                enemies[e].bullets[b].active = 0;
                j->hp -= DAMAGE_BULLET;
                if (j->hp <= 0) {
                    j->hp = 0;
                    j->alive = 0;
                    j->current_anim = ANIM_DEATH;
                    resetAnim(&j->anims[ANIM_DEATH]);
                    if (j->snd_death) Mix_PlayChannel(-1, j->snd_death, 0);
                }
            }
        }
    }
}
