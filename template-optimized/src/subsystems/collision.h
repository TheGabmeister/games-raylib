#ifndef COLLISION_H
#define COLLISION_H

#include "core/types.h"
#include <stdint.h>

#define MAX_COLLISION_BODIES 256
#define MAX_COLLISION_PAIRS  128

typedef struct CollisionBody {
    Vec2 center;
    float radius;
    uint32_t layer;   // what this body IS (bitmask)
    uint32_t mask;    // what layers this body COLLIDES WITH (bitmask)
    int tag;          // game-defined type identifier
    int index;        // game-defined index (e.g. pool slot)
} CollisionBody;

typedef struct CollisionPair {
    int tag_a, index_a;
    int tag_b, index_b;
} CollisionPair;

typedef struct CollisionWorld {
    CollisionBody bodies[MAX_COLLISION_BODIES];
    int count;
} CollisionWorld;

void collision_world_clear(CollisionWorld *w);
void collision_world_add(CollisionWorld *w, Vec2 center, float radius,
                         uint32_t layer, uint32_t mask, int tag, int index);
int collision_world_check(const CollisionWorld *w, CollisionPair *results, int max_results);

#endif
