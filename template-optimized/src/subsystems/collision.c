#include "subsystems/collision.h"

void collision_world_clear(CollisionWorld *w) {
    w->count = 0;
}

void collision_world_add(CollisionWorld *w, Vec2 center, float radius,
                         uint32_t layer, uint32_t mask, int tag, int index) {
    if (w->count >= MAX_COLLISION_BODIES) return;
    w->bodies[w->count++] = (CollisionBody){
        .center = center,
        .radius = radius,
        .layer  = layer,
        .mask   = mask,
        .tag    = tag,
        .index  = index,
    };
}

int collision_world_check(const CollisionWorld *w, CollisionPair *results, int max_results) {
    int count = 0;
    for (int i = 0; i < w->count && count < max_results; i++) {
        for (int j = i + 1; j < w->count && count < max_results; j++) {
            const CollisionBody *a = &w->bodies[i];
            const CollisionBody *b = &w->bodies[j];

            // Both directions: does A want to collide with B's layer, or vice versa?
            if (!(a->layer & b->mask) && !(b->layer & a->mask))
                continue;

            float dist = vec2_dist(a->center, b->center);
            if (dist < a->radius + b->radius) {
                results[count++] = (CollisionPair){
                    .tag_a = a->tag, .index_a = a->index,
                    .tag_b = b->tag, .index_b = b->index,
                };
            }
        }
    }
    return count;
}
