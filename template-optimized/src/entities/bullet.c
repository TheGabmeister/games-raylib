#include "entities/bullet.h"
#include "subsystems/renderer.h"
#include "core/pool.h"
#include <math.h>

void bullet_fire(Bullet *pool, int max, Vec2 pos, float rotation_deg) {
    Bullet *b;
    POOL_ALLOC(pool, max, b);
    if (!b) return;
    float rad = rotation_deg * DEG2RAD_F;
    b->position = pos;
    b->velocity = (Vec2){ sinf(rad) * BULLET_SPEED, -cosf(rad) * BULLET_SPEED };
    b->lifetime = BULLET_LIFETIME;
}

void bullets_update(Bullet *pool, int max, float dt) {
    POOL_EACH(pool, max, i) {
        pool[i].position = vec2_add(pool[i].position, vec2_scale(pool[i].velocity, dt));
        pool[i].position = wrap_position(pool[i].position);
        pool[i].lifetime -= dt;
        if (pool[i].lifetime <= 0) POOL_FREE(pool, i);
    }
}

void bullets_draw(const Bullet *pool, int max, Renderer *r) {
    POOL_EACH(pool, max, i) {
        renderer_push_neon_circle(r, LAYER_ENTITIES, pool[i].position, BULLET_RADIUS, CLR_WHITE);

        float len = vec2_length(pool[i].velocity);
        if (len > 0) {
            Vec2 trail = {
                pool[i].position.x - (pool[i].velocity.x / len) * 8.0f,
                pool[i].position.y - (pool[i].velocity.y / len) * 8.0f,
            };
            renderer_push_line(r, LAYER_ENTITIES, pool[i].position, trail,
                               1.0f, colr_alpha(CLR_WHITE, 0.5f));
        }
    }
}
