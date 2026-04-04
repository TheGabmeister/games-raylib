#include "entities/asteroid.h"
#include "subsystems/renderer.h"
#include "core/pool.h"
#include <math.h>
#include <stdlib.h>

static float size_to_radius(AsteroidSize size) {
    switch (size) {
        case SIZE_LARGE:  return 40.0f;
        case SIZE_MEDIUM: return 20.0f;
        case SIZE_SMALL:  return 10.0f;
    }
    return 10.0f;
}

static void generate_shape(Asteroid *a) {
    a->vert_count = 8 + rand() % 4;
    for (int i = 0; i < a->vert_count; i++)
        a->vert_distances[i] = a->radius * (0.7f + randf(0, 0.3f));
}

void asteroid_spawn(Asteroid *pool, int max, AsteroidSize size, Vec2 pos, Vec2 vel) {
    Asteroid *a;
    POOL_ALLOC(pool, max, a);
    if (!a) return;
    a->position  = pos;
    a->velocity  = vel;
    a->rotation  = randf(0, 360);
    a->rot_speed = randf(-90, 90);
    a->radius    = size_to_radius(size);
    a->size      = size;
    generate_shape(a);
}

void asteroid_spawn_wave(Asteroid *pool, int max, int wave) {
    int count = 2 + wave;
    if (count > 12) count = 12;
    for (int i = 0; i < count; i++) {
        Vec2 pos;
        if (rand() % 2) {
            pos.x = (rand() % 2) ? 0.0f : (float)SCREEN_WIDTH;
            pos.y = randf(0, (float)SCREEN_HEIGHT);
        } else {
            pos.x = randf(0, (float)SCREEN_WIDTH);
            pos.y = (rand() % 2) ? 0.0f : (float)SCREEN_HEIGHT;
        }
        float angle = randf(0, 2.0f * PI);
        float speed = ASTEROID_SPEED_MIN * (1.0f + wave * 0.1f);
        Vec2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        asteroid_spawn(pool, max, SIZE_LARGE, pos, vel);
    }
}

void asteroids_update(Asteroid *pool, int max, float dt) {
    POOL_EACH(pool, max, i) {
        pool[i].position = vec2_add(pool[i].position, vec2_scale(pool[i].velocity, dt));
        pool[i].position = wrap_position(pool[i].position);
        pool[i].rotation += pool[i].rot_speed * dt;
    }
}

void asteroids_draw(const Asteroid *pool, int max, Renderer *r) {
    POOL_EACH(pool, max, i) {
        const Asteroid *a = &pool[i];

        for (int v = 0; v < a->vert_count; v++) {
            int next = (v + 1) % a->vert_count;
            float ang1 = (360.0f / a->vert_count * v + a->rotation) * DEG2RAD_F;
            float ang2 = (360.0f / a->vert_count * next + a->rotation) * DEG2RAD_F;
            Vec2 p1 = { a->position.x + cosf(ang1) * a->vert_distances[v],
                         a->position.y + sinf(ang1) * a->vert_distances[v] };
            Vec2 p2 = { a->position.x + cosf(ang2) * a->vert_distances[next],
                         a->position.y + sinf(ang2) * a->vert_distances[next] };
            renderer_push_neon_line(r, LAYER_ENTITIES, p1, p2, CLR_LGRAY);
        }
    }
}

void asteroid_destroy(Asteroid *pool, int max, int index) {
    Asteroid *a = &pool[index];
    Vec2 pos = a->position;
    Vec2 vel = a->velocity;
    AsteroidSize size = a->size;
    POOL_FREE(pool, index);

    if (size == SIZE_LARGE || size == SIZE_MEDIUM) {
        AsteroidSize new_size = (size == SIZE_LARGE) ? SIZE_MEDIUM : SIZE_SMALL;
        for (int i = 0; i < 2; i++) {
            float angle_off = randf(30, 60) * (i == 0 ? 1.0f : -1.0f) * DEG2RAD_F;
            float base = atan2f(vel.y, vel.x) + angle_off;
            float speed = vec2_length(vel) * 1.5f;
            if (speed < ASTEROID_SPEED_MIN) speed = ASTEROID_SPEED_MIN;
            Vec2 new_vel = { cosf(base) * speed, sinf(base) * speed };
            asteroid_spawn(pool, max, new_size, pos, new_vel);
        }
    }
}

int asteroids_count_active(const Asteroid *pool, int max) {
    int count = 0;
    POOL_EACH(pool, max, i) { count++; }
    return count;
}
