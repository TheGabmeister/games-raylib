#ifndef BULLET_H
#define BULLET_H

#include "core/types.h"

#define BULLET_SPEED    500.0f
#define BULLET_RADIUS   2.0f
#define BULLET_LIFETIME 1.5f

typedef struct Renderer Renderer;

typedef struct {
    Vec2 position;
    Vec2 velocity;
    float lifetime;
    bool active;
} Bullet;

void bullet_fire(Bullet *pool, int max, Vec2 pos, float rotation_deg);
void bullets_update(Bullet *pool, int max, float dt);
void bullets_draw(const Bullet *pool, int max, Renderer *r);

#endif
