#ifndef DEBUG_H
#define DEBUG_H

#include "core/types.h"

#define DEBUG_MAX_STATS 16

typedef struct Renderer Renderer;
struct CollisionWorld;

typedef struct {
    bool active;
    bool show_collisions;
    float fps;
    float fps_timer;
    int frame_count;
    struct { const char *label; int value; } stats[DEBUG_MAX_STATS];
    int stat_count;
} Debug;

void debug_init(Debug *d);
void debug_update(Debug *d, float dt);
void debug_stat(Debug *d, const char *label, int value);
void debug_draw(const Debug *d, Renderer *r);
void debug_draw_collisions(const Debug *d, Renderer *r, const struct CollisionWorld *w);

#endif
