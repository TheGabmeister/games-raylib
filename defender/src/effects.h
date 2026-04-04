#ifndef EFFECTS_H
#define EFFECTS_H

#include "game_types.h"

void InitPalette(RenderPalette *palette);
void UpdateEffects(Game *game, float dt);
void UpdateGameCamera(Game *game, float dt);
void AddCameraShake(Game *game, float strength);
void SpawnParticle(Game *game, Vector2 position, Vector2 velocity, float life, float size, float sizeVelocity, Color color);
void SpawnBurst(Game *game, Vector2 position, Color color, int count, float minSpeed, float maxSpeed, float size);
void SpawnThrusterParticles(Game *game, Vector2 position, int facing, Color color);

#endif
