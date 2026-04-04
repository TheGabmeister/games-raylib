#include "starfield.h"

void StarfieldInit(Starfield *sf)
{
    for (int i = 0; i < STAR_COUNT; i++) {
        Star *s = &sf->stars[i];
        s->position.x = (float)GetRandomValue(0, VIRTUAL_WIDTH);
        s->position.y = (float)GetRandomValue(0, VIRTUAL_HEIGHT);

        /* 3 parallax layers */
        int layer = i % 3;
        switch (layer) {
            case 0: s->speed = 20.0f;  s->size = 0.6f; break;
            case 1: s->speed = 40.0f;  s->size = 1.0f; break;
            case 2: s->speed = 70.0f;  s->size = 1.6f; break;
        }

        /* Slight color variation */
        int v = GetRandomValue(0, 2);
        if (v == 0)      s->color = (Color){255, 255, 255, 200};
        else if (v == 1)  s->color = (Color){180, 200, 255, 180};
        else              s->color = (Color){255, 255, 200, 160};
    }
}

void StarfieldUpdate(Starfield *sf, float dt)
{
    for (int i = 0; i < STAR_COUNT; i++) {
        sf->stars[i].position.y += sf->stars[i].speed * dt;
        if (sf->stars[i].position.y > VIRTUAL_HEIGHT) {
            sf->stars[i].position.y -= VIRTUAL_HEIGHT;
            sf->stars[i].position.x = (float)GetRandomValue(0, VIRTUAL_WIDTH);
        }
    }
}

void StarfieldDraw(Starfield *sf)
{
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < STAR_COUNT; i++) {
        Star *s = &sf->stars[i];
        DrawCircleGradient((int)s->position.x, (int)s->position.y,
                           s->size, s->color, (Color){0, 0, 0, 0});
    }
    EndBlendMode();
}
