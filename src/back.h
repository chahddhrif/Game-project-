#ifndef BACK_H
#define BACK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

/* Dimensions globales */
#define WINDOW_W   1920
#define WINDOW_H   1080
#define WORLD_W    11520
#define PAGE_W     1920
#define GROUND_Y   960

/* ============================================================
   SCORE
   ============================================================ */
typedef struct {
    char name[64];
    int  score;
} ScoreEntry;

/* ============================================================
   BARRE DE VIE
   ============================================================ */
typedef struct {
    int hp, hpMax;
    int x, y, w, h;
} HealthBar;

/* ============================================================
   ANIMATION GENERIQUE
   ============================================================ */
typedef struct {
    SDL_Texture *tex;
    int cols, rows, total;
    int frame_w, frame_h;
    int current;
    Uint32 last_time;
    int delay;
    int loop;
    int finished;
} BAnim;

/* ============================================================
   COLLECTIBLE
   ============================================================ */
#define MAX_COLLECTIBLES 30

typedef enum {
    COL_1DT = 0,
    COL_5DT,
    COL_DIAMOND,
    COL_BLOOD,
    COL_AMMO,
    COL_ANTIBALL,
    COL_TYPE_COUNT
} CollectibleType;

typedef struct {
    CollectibleType type;
    float wx, wy;
    int   active;
    BAnim anim;
} Collectible;

/* ============================================================
   BOMBE CIEL (tombe en diagonale 45°)
   ============================================================ */
#define MAX_BOMBES 5

typedef struct {
    float wx, wy;
    float vx, vy;
    int   active;
    int   exploded;
    BAnim anim;
    BAnim anim_exp;
    Uint32 spawn_time;
} BombeCiel;

/* ============================================================
   MINE SOL
   ============================================================ */
#define MAX_MINES 10

typedef struct {
    float wx, wy;
    int   active;
    int   exploded;
    BAnim anim_normal;
    BAnim anim_exp;
} MineSol;

/* ============================================================
   HUD BULLETS (ligne de 20 en haut)
   ============================================================ */
#define MAX_HUD_BULLETS 20

/* ============================================================
   BACKGROUND PRINCIPAL
   ============================================================ */
typedef struct {
    /* Textures décor */
    SDL_Texture *sky;
    SDL_Texture *bat[4];
    SDL_Texture *ground1, *ground2;
    SDL_Texture *rain;
    SDL_Texture *territoire;

    /* Son */
    Mix_Music   *rainSound;

    /* Caméras */
    SDL_Rect cam1, cam2;
    int      mode;
    float    rainY;

    /* Polices */
    TTF_Font *font;
    TTF_Font *fontHelp;
    TTF_Font *fontHelpText;
    TTF_Font *fontScore;
    TTF_Font *fontScoreText;
    TTF_Font *fontBig;      /* pour DEFEAT/VICTORY/GREAT/CONGRATULATION */
    TTF_Font *fontHUD;      /* pour score HUD */

    SDL_Color color;
    Uint32    startTime;

    /* Minimap */
    SDL_Rect miniRect;
    float    scaleX, scaleY;
    int      mapLargeur, mapHauteur;

    /* Help/Pause */
    int    showHelp;
    Uint32 pauseTime;

    /* Scores */
    ScoreEntry scores[100];
    int        scoreCount;
    char       inputName1[64]; int inputLen1; int showScore1; int validated1;
    char       inputName2[64]; int inputLen2; int showScore2; int validated2;

    /* Barres de vie joueur */
    HealthBar hp1, hp2;

    /* ---- HUD collectibles ---- */
    int count_1dt;
    int count_5dt;
    int count_diamond;
    int total_score;
    int hud_bullets;        /* nombre de bullets restantes (max 20) */

    /* Textures HUD fixes */
    SDL_Texture *tex_1dt_fixe;
    SDL_Texture *tex_5dt_fixe;
    SDL_Texture *tex_diamond_fixe;
    SDL_Texture *tex_bullet_score;

    /* Collectibles dans le monde */
    Collectible collectibles[MAX_COLLECTIBLES];
    int         nb_collectibles;

    /* Textures spritesheets collectibles */
    SDL_Texture *tex_1dt_sheet;
    SDL_Texture *tex_5dt_sheet;
    SDL_Texture *tex_diamond_sheet;
    SDL_Texture *tex_blood_sheet;
    SDL_Texture *tex_ammo_sheet;
    SDL_Texture *tex_antiball_sheet;

    /* Bombes ciel */
    BombeCiel bombes[MAX_BOMBES];
    Uint32    bombe_timer;
    SDL_Texture *tex_bombe_ciel;
    SDL_Texture *tex_bombe_exp;

    /* Mines sol */
    MineSol mines[MAX_MINES];
    SDL_Texture *tex_mine_normal;
    SDL_Texture *tex_mine_exp;

    /* Messages flottants */
    char    msg_text[64];
    Uint32  msg_timer;
    int     msg_active;
    SDL_Color msg_color;

    /* Score animation */
    int    score_anim_val;
    Uint32 score_anim_timer;
    int    score_anim_active;

} Background;

/* ============================================================
   PROTOTYPES
   ============================================================ */

/* Back de base */
void initBack(Background *b, SDL_Renderer *re);
void scrolling(Background *b, const Uint8 *keystate);
void setCamFromPlayer(Background *b, float player_wx);
void afficherBack(Background *b, SDL_Renderer *re, int playerNum);
void afficherTemps(Background *b, SDL_Renderer *re, int playerNum);
void libererBack(Background *b);

/* Minimap */
void initMiniMap(Background *b);
void afficherMiniMap(Background *b, SDL_Renderer *re,
                     float p1x, float p1y, float p2x, float p2y);

/* Help */
void toggleHelp(Background *b);
void afficherHelp(Background *b, SDL_Renderer *re);

/* Score */
void initScore(Background *b, SDL_Renderer *re);
void triggerScore(Background *b, int playerNum);
void handleScoreEvent(Background *b, SDL_Event *e);
void afficherScore(Background *b, SDL_Renderer *re);
void libererScore(Background *b);
void saveScores(Background *b);
void loadScores(Background *b);
void trierScores(Background *b);

/* Santé */
void initHealthBar(HealthBar *h, int x, int y, int w, int hgt, int hpMax);
void takeDamage(HealthBar *h, int dmg);
void afficherHealthBar(HealthBar *h, SDL_Renderer *re);

/* HUD & collectibles */
void initHUD(Background *b, SDL_Renderer *re);
void initCollectibles(Background *b, SDL_Renderer *re);
void updateCollectibles(Background *b, float player_wx, float player_wy,
                        int *costume_changed);
void afficherHUD(Background *b, SDL_Renderer *re);
void afficherCollectibles(Background *b, SDL_Renderer *re, int cam_x);
void libererHUD(Background *b);

/* Bombes */
void initBombes(Background *b, SDL_Renderer *re);
void updateBombes(Background *b, float player_wx, float player_wy,
                  int *player_dead);
void afficherBombes(Background *b, SDL_Renderer *re, int cam_x);
void libererBombes(Background *b);

/* Mines */
void initMines(Background *b, SDL_Renderer *re);
void updateMines(Background *b, float dog_wx, float dog_wy,
                 int *dog_dead);
void afficherMines(Background *b, SDL_Renderer *re, int cam_x);
void libererMines(Background *b);

/* Messages */
void afficherMessage(Background *b, SDL_Renderer *re);
void showMessage(Background *b, const char *txt, SDL_Color col);

/* BAnim helpers */
BAnim banimLoad(SDL_Renderer *re, const char *path,
                int cols, int rows, int delay, int loop);
void  banimFree(BAnim *a);
void  banimUpdate(BAnim *a);
void  banimRender(BAnim *a, SDL_Renderer *re, SDL_Rect *dst);

/* drawLine helper */
int drawLine(SDL_Renderer *re, TTF_Font *font,
             SDL_Color col, char *txt, int x, int y);

#endif /* BACK_H */
