#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

#define MAX_BULLETS    30
#define MAX_ASTEROIDS  64
#define MAX_PARTICLES  256

#define PI 3.14159265358979323846f
#define DEG2RAD_F (PI / 180.0f)

typedef struct { float x, y; } Vec2;
typedef struct { unsigned char r, g, b, a; } Colr;

#define CLR_WHITE  ((Colr){255, 255, 255, 255})
#define CLR_BLACK  ((Colr){0,   0,   0,   255})
#define CLR_GREEN  ((Colr){0,   228, 48,  255})
#define CLR_RED    ((Colr){230, 41,  55,  255})
#define CLR_ORANGE ((Colr){255, 161, 0,   255})
#define CLR_YELLOW ((Colr){253, 249, 0,   255})
#define CLR_LGRAY  ((Colr){200, 200, 200, 255})
#define CLR_CYAN   ((Colr){102, 191, 255, 255})

static inline Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }
static inline Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }
static inline Vec2 vec2_scale(Vec2 v, float s) { return (Vec2){v.x * s, v.y * s}; }
static inline float vec2_length(Vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }
static inline float vec2_dist(Vec2 a, Vec2 b) { return vec2_length(vec2_sub(a, b)); }

static inline Vec2 wrap_position(Vec2 pos) {
    if (pos.x < 0) pos.x += SCREEN_WIDTH;
    if (pos.x > SCREEN_WIDTH) pos.x -= SCREEN_WIDTH;
    if (pos.y < 0) pos.y += SCREEN_HEIGHT;
    if (pos.y > SCREEN_HEIGHT) pos.y -= SCREEN_HEIGHT;
    return pos;
}

static inline float randf(float min, float max) {
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

static inline Colr colr_alpha(Colr c, float alpha) {
    c.a = (unsigned char)(alpha * 255.0f);
    return c;
}

#endif
