#ifndef ASTEROID_H
#define ASTEROID_H

#include "core/types.h"

#define MAX_ASTEROID_VERTS 12
#define ASTEROID_SPEED_MIN 40.0f

typedef struct Renderer Renderer;

typedef enum { SIZE_LARGE, SIZE_MEDIUM, SIZE_SMALL } AsteroidSize;

typedef struct {
    Vec2 position;
    Vec2 velocity;
    float rotation;
    float rot_speed;
    float radius;
    int vert_count;
    float vert_distances[MAX_ASTEROID_VERTS];
    AsteroidSize size;
    bool active;
} Asteroid;

void asteroid_spawn(Asteroid *pool, int max, AsteroidSize size, Vec2 pos, Vec2 vel);
void asteroid_spawn_wave(Asteroid *pool, int max, int wave);
void asteroids_update(Asteroid *pool, int max, float dt);
void asteroids_draw(const Asteroid *pool, int max, Renderer *r);
void asteroid_destroy(Asteroid *pool, int max, int index);
int asteroids_count_active(const Asteroid *pool, int max);

#endif
