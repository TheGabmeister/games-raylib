#ifndef FORMATION_H
#define FORMATION_H

#include "config.h"
#include <stdbool.h>

typedef struct {
    float sway_timer;
    float sway_offset;
    float dive_timer;
    int active_divers;
    bool all_entered;
} Formation;

struct Game;

void FormationInit(Formation *f);
Vector2 FormationSlotPosition(int col, int row, float sway_offset);
void FormationUpdate(struct Game *game, float dt);
void FormationInitStage(struct Game *game);

#endif /* FORMATION_H */
