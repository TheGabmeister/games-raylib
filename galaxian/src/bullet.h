#ifndef BULLET_H
#define BULLET_H

#include "config.h"

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
} Bullet;

typedef struct {
    Bullet player_bullet;
    Bullet enemy_bullets[MAX_ENEMY_BULLETS];
    int enemy_bullet_count;
} BulletPool;

void BulletPoolInit(BulletPool *bp);
void BulletPoolUpdate(BulletPool *bp, float dt);
void BulletPoolDraw(BulletPool *bp);
bool FirePlayerBullet(BulletPool *bp, Vector2 pos);
void FireEnemyBullet(BulletPool *bp, Vector2 pos, Vector2 target);

#endif /* BULLET_H */
