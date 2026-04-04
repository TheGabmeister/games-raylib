#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Vector2 position;
    bool alive;
    float respawn_timer;
    float invincible_timer;
    float thrust_timer;
} Player;

void PlayerInit(Player *p);
void PlayerUpdate(Player *p, float dt);
void PlayerDraw(Player *p);

#endif /* PLAYER_H */
