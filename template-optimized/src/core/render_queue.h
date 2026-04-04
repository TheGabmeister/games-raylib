#ifndef RENDER_QUEUE_H
#define RENDER_QUEUE_H

#include "types.h"
#include <string.h>

#define MAX_RENDER_COMMANDS 2048

typedef enum {
    RCMD_LINE,
    RCMD_CIRCLE,
    RCMD_TEXT,
} RenderCmdType;

typedef struct {
    RenderCmdType type;
    int layer;
    Colr color;
    union {
        struct { Vec2 start, end; float thick; } line;
        struct { Vec2 center; float radius; bool filled; } circle;
        struct { Vec2 pos; char str[64]; int size; } text;
    };
} RenderCommand;

typedef struct {
    RenderCommand commands[MAX_RENDER_COMMANDS];
    int count;
} RenderQueue;

static inline void rq_clear(RenderQueue *q) { q->count = 0; }

static inline void rq_push(RenderQueue *q, RenderCommand cmd) {
    if (q->count < MAX_RENDER_COMMANDS)
        q->commands[q->count++] = cmd;
}

#endif
