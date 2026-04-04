#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "player.h"
#include "enemy.h"
#include "formation.h"
#include "bullet.h"
#include "particle.h"
#include "starfield.h"
#include "effects.h"

typedef enum {
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_GAMEOVER
} ScreenState;

typedef struct Game {
    ScreenState screen;

    Player player;
    Enemy enemies[MAX_ENEMIES];
    Formation formation;
    BulletPool bullets;

    Particle particles[MAX_PARTICLES];
    int particle_count;

    Starfield starfield;
    Effects effects;

    int score;
    int lives;
    int stage;
    float difficulty;
    bool bonus_life_awarded;

    float stage_clear_timer;
    bool stage_clearing;
    float gameover_timer;
} Game;

void GameInit(Game *game);
void GameUpdate(Game *game, float dt);
void GameDraw(Game *game);
void GameStartStage(Game *game);
float GameDifficulty(int stage);

#endif /* GAME_H */
