#include "bullet.h"
#include "draw_utils.h"
#include <math.h>
#include <string.h>

void BulletPoolInit(BulletPool *bp)
{
    memset(bp, 0, sizeof(*bp));
}

void BulletPoolUpdate(BulletPool *bp, float dt)
{
    /* Player bullet */
    if (bp->player_bullet.active) {
        bp->player_bullet.position.x += bp->player_bullet.velocity.x * dt;
        bp->player_bullet.position.y += bp->player_bullet.velocity.y * dt;
        if (bp->player_bullet.position.y < -10)
            bp->player_bullet.active = false;
    }

    /* Enemy bullets */
    for (int i = 0; i < bp->enemy_bullet_count; ) {
        Bullet *b = &bp->enemy_bullets[i];
        b->position.x += b->velocity.x * dt;
        b->position.y += b->velocity.y * dt;
        if (b->position.y > VIRTUAL_HEIGHT + 10 || b->position.y < -10 ||
            b->position.x < -10 || b->position.x > VIRTUAL_WIDTH + 10)
        {
            bp->enemy_bullets[i] = bp->enemy_bullets[bp->enemy_bullet_count - 1];
            bp->enemy_bullet_count--;
            continue;
        }
        i++;
    }
}

void BulletPoolDraw(BulletPool *bp)
{
    if (bp->player_bullet.active)
        DrawBulletGlow(bp->player_bullet.position, bp->player_bullet.velocity, COLOR_PLAYER_BULLET);

    for (int i = 0; i < bp->enemy_bullet_count; i++)
        DrawBulletGlow(bp->enemy_bullets[i].position, bp->enemy_bullets[i].velocity, COLOR_ENEMY_BULLET);
}

bool FirePlayerBullet(BulletPool *bp, Vector2 pos)
{
    if (bp->player_bullet.active) return false;
    bp->player_bullet.active = true;
    bp->player_bullet.position = pos;
    bp->player_bullet.velocity = (Vector2){ 0, -PLAYER_BULLET_SPEED };
    return true;
}

void FireEnemyBullet(BulletPool *bp, Vector2 pos, Vector2 target)
{
    if (bp->enemy_bullet_count >= MAX_ENEMY_BULLETS) return;

    float dx = target.x - pos.x + RandFloat(-20, 20);
    float dy = target.y - pos.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) len = 1.0f;

    Bullet b = {0};
    b.active = true;
    b.position = pos;
    b.velocity = (Vector2){ (dx / len) * ENEMY_BULLET_SPEED, (dy / len) * ENEMY_BULLET_SPEED };
    bp->enemy_bullets[bp->enemy_bullet_count++] = b;
}
