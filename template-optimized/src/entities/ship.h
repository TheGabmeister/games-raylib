#ifndef SHIP_H
#define SHIP_H

#include "core/types.h"

#define SHIP_SIZE       20.0f
#define SHIP_THRUST     300.0f
#define SHIP_DRAG       0.98f
#define SHIP_ROT_SPEED  250.0f
#define SHIP_INVULN_TIME 3.0f
#define SHIP_BLINK_RATE 0.1f
#define SHIP_FIRE_RATE  0.15f

typedef struct Renderer Renderer;
typedef struct Input Input;

typedef struct {
    Vec2 position;
    Vec2 velocity;
    float rotation;
    bool alive;
    bool thrusting;
    float invuln_timer;
    float respawn_timer;
    float fire_cooldown;
    int lives;
} Ship;

void ship_init(Ship *ship);
void ship_update(Ship *ship, const Input *inp, float dt);
void ship_draw(const Ship *ship, Renderer *r);
void ship_kill(Ship *ship);
Vec2 ship_nose(const Ship *ship);
float ship_radius(void);

#endif
