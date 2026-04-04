#ifndef ENEMY_H
#define ENEMY_H

#include "config.h"
#include "path.h"

typedef enum {
    ENEMY_BLUE,
    ENEMY_PURPLE,
    ENEMY_RED,
    ENEMY_FLAGSHIP
} EnemyType;

typedef enum {
    ENEMY_INACTIVE,
    ENEMY_ENTERING,
    ENEMY_IN_FORMATION,
    ENEMY_DIVING,
    ENEMY_RETURNING
} EnemyState;

typedef struct {
    EnemyType type;
    EnemyState state;
    Vector2 position;
    int slot_col, slot_row;
    float rotation;
    bool alive;

    /* Entry animation */
    float enter_delay;
    float enter_timer;
    Vector2 enter_start;

    /* Dive */
    DivePath dive_path;
    float dive_time;
    float dive_speed;
    float fire_timer;

    /* Return */
    BezierSegment return_seg;
    float return_t;

    /* Escort tracking (flagships) */
    int escort_indices[2];
    int escort_count;
    int escorts_killed;
    bool is_escort;
    int escort_of;
} Enemy;

void EnemyDraw(Enemy *e);
float EnemyCollisionRadius(EnemyType type);
Color EnemyColor(EnemyType type);
int EnemyScoreFormation(EnemyType type);
int EnemyScoreDiving(Enemy *e);

#endif /* ENEMY_H */
