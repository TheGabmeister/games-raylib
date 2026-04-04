#include "entities/particle.h"
#include "subsystems/renderer.h"
#include "core/pool.h"
#include <math.h>
#include <stdlib.h>

void particle_spawn_burst(Particle *pool, int max, Vec2 pos, int count,
                           float speed, float lifetime, Colr color) {
    for (int s = 0; s < count; s++) {
        Particle *p;
        POOL_ALLOC(pool, max, p);
        if (!p) return;
        float angle = randf(0, 2.0f * PI);
        float spd = randf(speed * 0.3f, speed);
        p->position     = pos;
        p->velocity     = (Vec2){ cosf(angle) * spd, sinf(angle) * spd };
        p->lifetime     = randf(lifetime * 0.5f, lifetime);
        p->max_lifetime = lifetime;
        p->color        = color;
        p->size         = randf(1.0f, 3.0f);
    }
}

void particles_update(Particle *pool, int max, float dt) {
    POOL_EACH(pool, max, i) {
        pool[i].position = vec2_add(pool[i].position, vec2_scale(pool[i].velocity, dt));
        pool[i].velocity = vec2_scale(pool[i].velocity, powf(0.98f, dt * 60.0f));
        pool[i].lifetime -= dt;
        if (pool[i].lifetime <= 0) POOL_FREE(pool, i);
    }
}

void particles_draw(const Particle *pool, int max, Renderer *r) {
    POOL_EACH(pool, max, i) {
        float alpha = pool[i].lifetime / pool[i].max_lifetime;
        Colr c = pool[i].color;
        c.a = (unsigned char)(alpha * 255.0f);
        renderer_push_circle(r, LAYER_PARTICLES, pool[i].position, pool[i].size, true, c);
    }
}
