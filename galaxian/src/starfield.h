#ifndef STARFIELD_H
#define STARFIELD_H

#include "config.h"

typedef struct {
    Vector2 position;
    float speed;
    float size;
    Color color;
} Star;

typedef struct {
    Star stars[STAR_COUNT];
} Starfield;

void StarfieldInit(Starfield *sf);
void StarfieldUpdate(Starfield *sf, float dt);
void StarfieldDraw(Starfield *sf);

#endif /* STARFIELD_H */
