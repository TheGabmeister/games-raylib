#ifndef WORLD_H
#define WORLD_H

#include "game_types.h"

void WorldInit(WorldState *world);
float WorldWrapX(const WorldState *world, float x);
float WorldWrapDeltaX(const WorldState *world, float delta);
float WorldGetTerrainHeight(const WorldState *world, float x);
Vector2 WorldFindSafePosition(const WorldState *world, float desiredX, float desiredY);

#endif
