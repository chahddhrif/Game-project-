#ifndef JOUEUR_H
#define JOUEUR_H

#include "back.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
   CONSTANTES JOUEUR (non dupliquées avec back.h)
   ============================================================ */
#define FPS_DELAY       22
#define GRAVITY         0.7f
#define JUMP_FORCE     -20.0f
#define SPEED_WALK      8
#define SPEED_RUN       16
#define BULLET_SPEED    20
#define MAX_BULLETS     20
#define MAX_ENEMIES     5
#define HEALTH_BAR_W    100
#define HEALTH_BAR_H    12
#define DOG_OFFSET      80
#define DAMAGE_BULLET   10
#define DAMAGE_MELEE    10
#define WORLD_W         11520
#define PAGE_W          1920
/* Délai frame actions à 45fps = ~22ms par frame */
#define ACTION_FRAME_DELAY  22

/* ============================================================
   ENUMS
   ============================================================ */
typedef enum {
    ANIM_IDLE=0, ANIM_WALK, ANIM_RUN, ANIM_JUMP,
    ANIM_KICK, ANIM_PUNCH, ANIM_SHOOT, ANIM_DEATH,
    ANIM_VICTORY, ANIM_ATTACK, ANIM_COUNT
} AnimType;

typedef enum {
    COSTUME_JOHN=0, COSTUME_JOHN_ANTIBALL,
    JOHN_SANS_CHEMISE, JOHN_SANS_CHEMISE_ANTIBALL,
    JOHN_SANS_COSTUME, JOHN_SANS_COSTUME_ANTIBALL,
    COSTUME_HELEN, COSTUME_HELEN_ANTIBALL,
    HELEN_CHEMISE, HELEN_CHEMISE_ANTIBALL,
    COSTUME_COUNT
} CostumeType;

typedef enum { PERSO_JOHN=0, PERSO_HELEN, PERSO_COUNT } PersonnageType;

typedef enum {
    ENEMY_TYPE_1=0, ENEMY_TYPE_2, ENEMY_TYPE_3,
    ENEMY_LOSEF, ENEMY_VIGGO, ENEMY_TYPE_COUNT
} EnemyType;

/* ============================================================
   STRUCTURES
   ============================================================ */
typedef struct {
    int cols, rows, frame_w, frame_h, total_frames;
} SheetInfo;

typedef struct {
    SDL_Texture *texture;
    SheetInfo    info;
    int          current_frame;
    Uint32       last_time;
    int          frame_delay;
    int          loop;
    int          finished;
} Animation;

typedef struct {
    float x, y, vx;
    int   active;
    SDL_Texture *texture;
    int   w, h;
} Bullet;

typedef struct {
    float          wx, wy, vy;
    int            on_ground, facing_right;
    PersonnageType perso;
    CostumeType    costume;
    Animation      anims[ANIM_COUNT];
    AnimType       current_anim;
    int            action_queue;
    AnimType       queued_action;
    int            hp, hp_max, alive;
    Mix_Chunk     *snd_walk, *snd_run, *snd_punch, *snd_punch2;
    Mix_Chunk     *snd_kick, *snd_gun, *snd_death;
    int            walk_channel, run_channel;
    int            punch_channel, kick_channel, gun_channel;
    /* Touches maintenues (1=appuyée, 0=relâchée) */
    int            key_punch, key_kick, key_shoot;
    Bullet         bullets[MAX_BULLETS];
    SDL_Texture   *bullet_tex;
    /* victoire */
    int            victory;
} Joueur;

typedef struct {
    EnemyType    type;
    float        wx, wy, vx, vy;
    int          on_ground, facing_right, alive;
    int          hp, hp_max;
    Animation    anims[ANIM_COUNT];
    AnimType     current_anim;
    Uint32       ai_timer;
    int          ai_state;       /* 0=idle,1=walk,2=shoot */
    int          ai_active;      /* 1 si joueur visible à l'écran */
    Bullet       bullets[MAX_BULLETS];
    SDL_Texture *bullet_tex;
    Uint32       great_timer;
    int          show_great;
} Ennemi;

typedef struct {
    float     wx, wy;
    int       facing_right, alive;
    Animation anims[3];
    int       current_anim;
} Chien;

/* ============================================================
   PROTOTYPES
   ============================================================ */
Animation loadAnim(SDL_Renderer *re, const char *path,
                   int cols, int rows, int frame_delay, int loop);
void freeAnim(Animation *a);
void updateAnim(Animation *a);
void resetAnim(Animation *a);
void renderAnim(Animation *a, SDL_Renderer *re,
                SDL_Rect *dst, SDL_RendererFlip flip);

void joueurInit(Joueur *j, SDL_Renderer *re,
                PersonnageType perso, CostumeType costume);
void joueurHandleInput(Joueur *j, const Uint8 *keys, SDL_Event *ev);
void joueurUpdate(Joueur *j);
void joueurRender(Joueur *j, SDL_Renderer *re, int cam_x);
void joueurFree(Joueur *j);
void joueurSetAnim(Joueur *j, AnimType anim);
void joueurFireBullet(Joueur *j);
void joueurUpdateBullets(Joueur *j);
void joueurRenderBullets(Joueur *j, SDL_Renderer *re, int cam_x);

void ennemiInit(Ennemi *e, SDL_Renderer *re,
                EnemyType type, float wx, float wy);
void ennemiUpdate(Ennemi *e, float player_wx, int cam_x);
void ennemiRender(Ennemi *e, SDL_Renderer *re, int cam_x);
void ennemiFree(Ennemi *e);
void ennemiRenderHealthBar(Ennemi *e, SDL_Renderer *re, int cam_x);
void ennemiRenderBullets(Ennemi *e, SDL_Renderer *re, int cam_x);
void ennemiRenderGreat(Ennemi *e, SDL_Renderer *re, TTF_Font *font);

void chienInit(Chien *c, SDL_Renderer *re, float wx, float wy);
void chienUpdate(Chien *c, Joueur *j);
void chienRender(Chien *c, SDL_Renderer *re, int cam_x);
void chienFree(Chien *c);

void checkBulletsVsEnemies(Joueur *j, Ennemi *enemies, int nb);
void checkPunchKickVsEnemies(Joueur *j, Ennemi *enemies, int nb);
void checkEnemyBulletsVsPlayer(Ennemi *enemies, int nb, Joueur *j);

#endif
