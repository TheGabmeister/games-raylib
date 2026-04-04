#ifndef RENDERER_H
#define RENDERER_H

#include "core/types.h"
#include "core/config.h"

typedef struct Renderer Renderer;

enum {
    LAYER_BACKGROUND = 0,
    LAYER_ENTITIES   = 10,
    LAYER_PARTICLES  = 15,
    LAYER_UI         = 30,
};

Renderer *renderer_create(const Config *cfg, const char *title);
void renderer_destroy(Renderer *r);
bool renderer_should_close(Renderer *r);
float renderer_get_dt(Renderer *r);

void renderer_push_line(Renderer *r, int layer, Vec2 start, Vec2 end, float thick, Colr color);
void renderer_push_circle(Renderer *r, int layer, Vec2 center, float radius, bool filled, Colr color);
void renderer_push_text(Renderer *r, int layer, Vec2 pos, const char *str, int size, Colr color);

void renderer_push_neon_line(Renderer *r, int layer, Vec2 start, Vec2 end, Colr color);
void renderer_push_neon_circle(Renderer *r, int layer, Vec2 center, float radius, Colr color);

void renderer_set_camera(Renderer *r, Vec2 offset);
void renderer_flush(Renderer *r);

#endif
