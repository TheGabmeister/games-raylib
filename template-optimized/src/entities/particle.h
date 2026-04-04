#ifndef PARTICLE_H
#define PARTICLE_H

#include "core/types.h"

typedef struct Renderer Renderer;

typedef struct {
    Vec2 position;
    Vec2 velocity;
    float lifetime;
    float max_lifetime;
    Colr color;
    float size;
    bool active;
} Particle;

void particle_spawn_burst(Particle *pool, int max, Vec2 pos, int count,
                           float speed, float lifetime, Colr color);
void particles_update(Particle *pool, int max, float dt);
void particles_draw(const Particle *pool, int max, Renderer *r);

#endif
