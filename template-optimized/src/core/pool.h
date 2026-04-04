#ifndef POOL_H
#define POOL_H

#include <stdbool.h>
#include <string.h>

// Iterate active items. Items must have a bool `active` field.
// Usage: POOL_EACH(bullets, MAX_BULLETS, i) { update(&bullets[i]); }
#define POOL_EACH(arr, max, i) \
    for (int i = 0; i < (max); i++) if ((arr)[i].active)

// Find first inactive slot, zero-initialize it, mark active, assign to `out`.
// Sets `out` to NULL if pool is full.
// Usage: Bullet *b; POOL_ALLOC(bullets, MAX_BULLETS, b); if (b) { b->x = 10; }
#define POOL_ALLOC(arr, max, out) do { \
    (out) = NULL; \
    for (int _pa = 0; _pa < (max); _pa++) { \
        if (!(arr)[_pa].active) { \
            memset(&(arr)[_pa], 0, sizeof((arr)[0])); \
            (arr)[_pa].active = true; \
            (out) = &(arr)[_pa]; \
            break; \
        } \
    } \
} while(0)

// Deactivate a slot by index.
#define POOL_FREE(arr, i) ((arr)[i].active = false)

#endif
