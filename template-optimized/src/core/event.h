#ifndef EVENT_H
#define EVENT_H

#include "types.h"

#define MAX_EVENTS 256

// Generic event: `type` is game-defined, `pos` + `a`/`b` carry payload.
typedef struct {
    int type;
    Vec2 pos;
    int a, b;
} Event;

typedef struct {
    Event events[MAX_EVENTS];
    int count;
} EventQueue;

static inline void event_queue_clear(EventQueue *q) { q->count = 0; }

static inline void event_queue_push(EventQueue *q, Event e) {
    if (q->count < MAX_EVENTS)
        q->events[q->count++] = e;
}

#endif
