#ifndef PLAYER_H
#define PLAYER_H

#include "game_types.h"

void ResetPlayerForNewRun(Game *game);
void RespawnPlayer(Game *game, float desiredX);
void UpdatePlayer(Game *game, float dt);

#endif
