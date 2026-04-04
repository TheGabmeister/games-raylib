#include "player.h"
#include "config.h"
#include "draw_utils.h"

void PlayerInit(Player *p)
{
    p->position = (Vector2){ PLAYER_START_X, PLAYER_START_Y };
    p->alive = true;
    p->respawn_timer = 0;
    p->invincible_timer = 0;
    p->thrust_timer = 0;
}

void PlayerUpdate(Player *p, float dt)
{
    if (!p->alive) {
        if (p->respawn_timer > 0) {
            p->respawn_timer -= dt;
            if (p->respawn_timer <= 0) {
                p->alive = true;
                p->position = (Vector2){ PLAYER_START_X, PLAYER_START_Y };
                p->invincible_timer = INVINCIBLE_DURATION;
            }
        }
        return;
    }

    if (p->invincible_timer > 0)
        p->invincible_timer -= dt;

    /* Movement */
    float move = 0;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  move -= 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) move += 1.0f;

    p->position.x += move * PLAYER_SPEED * dt;
    p->position.x = Clampf(p->position.x, 16.0f, VIRTUAL_WIDTH - 16.0f);

    p->thrust_timer += dt;
}

void PlayerDraw(Player *p)
{
    if (!p->alive) return;

    /* Blink during invincibility */
    if (p->invincible_timer > 0) {
        if (fmodf(p->invincible_timer * BLINK_RATE, 1.0f) > 0.5f)
            return;
    }

    float alpha = 1.0f;
    DrawPlayerShip(p->position, COLOR_PLAYER, alpha);
}
