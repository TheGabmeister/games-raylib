#include "subsystems/debug.h"
#include "subsystems/renderer.h"
#include "subsystems/collision.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

void debug_init(Debug *d) {
    memset(d, 0, sizeof(Debug));
}

void debug_update(Debug *d, float dt) {
    if (IsKeyPressed(KEY_F3)) d->active = !d->active;
    if (IsKeyPressed(KEY_F4)) d->show_collisions = !d->show_collisions;

    d->frame_count++;
    d->fps_timer += dt;
    if (d->fps_timer >= 0.5f) {
        d->fps = (float)d->frame_count / d->fps_timer;
        d->frame_count = 0;
        d->fps_timer = 0;
    }

    d->stat_count = 0;
}

void debug_stat(Debug *d, const char *label, int value) {
    if (!d || !d->active || d->stat_count >= DEBUG_MAX_STATS) return;
    d->stats[d->stat_count].label = label;
    d->stats[d->stat_count].value = value;
    d->stat_count++;
}

void debug_draw(const Debug *d, Renderer *r) {
    if (!d || !d->active) return;

    char buf[64];
    float x = SCREEN_WIDTH - 160.0f;

    snprintf(buf, sizeof(buf), "FPS: %.0f", d->fps);
    renderer_push_text(r, LAYER_UI, (Vec2){x, 10}, buf, 16, CLR_YELLOW);

    for (int i = 0; i < d->stat_count; i++) {
        snprintf(buf, sizeof(buf), "%s: %d", d->stats[i].label, d->stats[i].value);
        renderer_push_text(r, LAYER_UI, (Vec2){x, 30.0f + i * 18.0f}, buf, 16, CLR_YELLOW);
    }

    renderer_push_text(r, LAYER_UI,
                       (Vec2){x, (float)(30 + d->stat_count * 18 + 10)},
                       "F3:debug F4:col", 12, colr_alpha(CLR_YELLOW, 0.6f));
}

void debug_draw_collisions(const Debug *d, Renderer *r, const struct CollisionWorld *w) {
    if (!d || !d->active || !d->show_collisions) return;

    for (int i = 0; i < w->count; i++) {
        renderer_push_circle(r, LAYER_ENTITIES + 1,
                             w->bodies[i].center, w->bodies[i].radius,
                             false, colr_alpha(CLR_CYAN, 0.4f));
    }
}
