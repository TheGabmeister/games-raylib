#include "entities/ship.h"
#include "subsystems/renderer.h"
#include "subsystems/input.h"
#include <math.h>

void ship_init(Ship *ship) {
    *ship = (Ship){
        .position = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f },
        .alive = true,
        .invuln_timer = SHIP_INVULN_TIME,
        .lives = 3,
    };
}

static Vec2 ship_vertex(const Ship *ship, int index) {
    float offsets[] = { 0.0f, 140.0f, -140.0f };
    float rad = (ship->rotation + offsets[index]) * DEG2RAD_F;
    return (Vec2){
        ship->position.x + sinf(rad) * SHIP_SIZE,
        ship->position.y - cosf(rad) * SHIP_SIZE,
    };
}

Vec2 ship_nose(const Ship *ship) { return ship_vertex(ship, 0); }
float ship_radius(void) { return SHIP_SIZE * 0.6f; }

void ship_update(Ship *ship, const Input *inp, float dt) {
    if (!ship->alive) {
        ship->respawn_timer -= dt;
        if (ship->respawn_timer <= 0 && ship->lives > 0) {
            ship->alive = true;
            ship->position = (Vec2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
            ship->velocity = (Vec2){0, 0};
            ship->rotation = 0;
            ship->invuln_timer = SHIP_INVULN_TIME;
        }
        return;
    }

    ship->invuln_timer -= dt;
    ship->fire_cooldown -= dt;

    if (input_held(inp, ACTION_LEFT))  ship->rotation -= SHIP_ROT_SPEED * dt;
    if (input_held(inp, ACTION_RIGHT)) ship->rotation += SHIP_ROT_SPEED * dt;

    ship->thrusting = input_held(inp, ACTION_UP);
    if (ship->thrusting) {
        float rad = ship->rotation * DEG2RAD_F;
        ship->velocity.x += sinf(rad) * SHIP_THRUST * dt;
        ship->velocity.y -= cosf(rad) * SHIP_THRUST * dt;
    }

    ship->velocity = vec2_scale(ship->velocity, powf(SHIP_DRAG, dt * 60.0f));
    ship->position = vec2_add(ship->position, vec2_scale(ship->velocity, dt));
    ship->position = wrap_position(ship->position);
}

void ship_draw(const Ship *ship, Renderer *r) {
    if (!ship->alive) return;

    if (ship->invuln_timer > 0 &&
        fmodf(ship->invuln_timer, SHIP_BLINK_RATE * 2) > SHIP_BLINK_RATE)
        return;

    Vec2 v0 = ship_vertex(ship, 0);
    Vec2 v1 = ship_vertex(ship, 1);
    Vec2 v2 = ship_vertex(ship, 2);
    renderer_push_neon_line(r, LAYER_ENTITIES, v0, v1, CLR_GREEN);
    renderer_push_neon_line(r, LAYER_ENTITIES, v1, v2, CLR_GREEN);
    renderer_push_neon_line(r, LAYER_ENTITIES, v2, v0, CLR_GREEN);

    if (ship->thrusting) {
        Vec2 mid = { (v1.x + v2.x) / 2.0f, (v1.y + v2.y) / 2.0f };
        float flame_len = SHIP_SIZE * (0.5f + randf(0, 0.5f));
        float rad = ship->rotation * DEG2RAD_F;
        Vec2 tip = { mid.x - sinf(rad) * flame_len, mid.y + cosf(rad) * flame_len };
        Colr flame = (rand() % 2) ? CLR_ORANGE : CLR_YELLOW;
        renderer_push_neon_line(r, LAYER_ENTITIES, v1, tip, flame);
        renderer_push_neon_line(r, LAYER_ENTITIES, v2, tip, flame);
    }
}

void ship_kill(Ship *ship) {
    ship->alive = false;
    ship->lives--;
    ship->respawn_timer = 2.0f;
    ship->velocity = (Vec2){0, 0};
}
