#include "enemy.h"

#include "game.h"
#include "world.h"

static float RandomFloat(float minValue, float maxValue)
{
    return minValue + (maxValue - minValue)*((float)GetRandomValue(0, 10000)/10000.0f);
}

static int FindNearestGroundedHuman(Game *game, Vector2 position)
{
    int bestIndex = -1;
    float bestDistance = 9999999.0f;

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        Human *human = &game->humans[i];
        float dx;
        float dy;
        float distanceSq;

        if (!human->active || human->state != HUMAN_STATE_GROUNDED) continue;
        if (human->carrierEnemy >= 0) continue;

        dx = WorldWrapDeltaX(&game->world, human->position.x - position.x);
        dy = human->position.y - position.y;
        distanceSq = dx*dx + dy*dy;

        if (distanceSq < bestDistance)
        {
            bestDistance = distanceSq;
            bestIndex = i;
        }
    }

    return bestIndex;
}

static void FireAtPlayer(Game *game, Enemy *enemy, float projectileSpeed, float radius, float life)
{
    float dx;
    float dy;
    Vector2 direction;

    if (!game->player.alive) return;

    dx = WorldWrapDeltaX(&game->world, game->player.position.x - enemy->position.x);
    dy = game->player.position.y - enemy->position.y;
    direction = Vec2NormalizeSafe(Vec2(dx, dy));

    if (Vec2LengthSquared(direction) <= 0.0001f)
    {
        direction = Vec2((float)(enemy->velocity.x >= 0.0f ? 1 : -1), 0.0f);
    }

    GameSpawnProjectile(game,
                        enemy->position,
                        Vec2Add(Vec2Scale(direction, projectileSpeed), enemy->velocity),
                        false,
                        radius,
                        life,
                        enemy->type);
}

static void UpdateLander(Game *game, Enemy *enemy, int enemyIndex, float dt)
{
    Vector2 targetPosition;
    float dx;
    float dy;

    if (enemy->targetHuman >= 0)
    {
        Human *targetHuman = &game->humans[enemy->targetHuman];
        if (!targetHuman->active || targetHuman->state != HUMAN_STATE_GROUNDED)
        {
            enemy->targetHuman = -1;
        }
    }

    if (enemy->carryingHuman)
    {
        Human *human = &game->humans[enemy->targetHuman];
        targetPosition = Vec2(enemy->position.x, 92.0f);
        dx = WorldWrapDeltaX(&game->world, targetPosition.x - enemy->position.x);
        dy = targetPosition.y - enemy->position.y;
        enemy->velocity.x += dx*0.4f*dt;
        enemy->velocity.y += dy*0.9f*dt;

        human->position.x = enemy->position.x;
        human->position.y = enemy->position.y + 26.0f;
        human->velocity = enemy->velocity;

        if (enemy->position.y <= 92.0f)
        {
            human->active = false;
            human->state = HUMAN_STATE_DEAD;
            human->carrierEnemy = -1;
            enemy->carryingHuman = false;
            enemy->targetHuman = -1;
            enemy->type = ENEMY_MUTANT;
            enemy->radius = 18.0f;
            enemy->speed = 300.0f;
            enemy->hp = 1;
            enemy->fireTimer = 0.65f;
            GameSpawnFloatingText(game, enemy->position, "MUTANT", game->palette.warning);
        }
        return;
    }

    if (enemy->targetHuman < 0)
    {
        enemy->targetHuman = FindNearestGroundedHuman(game, enemy->position);
    }

    if (enemy->targetHuman >= 0)
    {
        Human *targetHuman = &game->humans[enemy->targetHuman];
        targetPosition = Vec2(targetHuman->position.x, targetHuman->position.y - 56.0f);
        dx = WorldWrapDeltaX(&game->world, targetPosition.x - enemy->position.x);
        dy = targetPosition.y - enemy->position.y;

        enemy->velocity.x += dx*0.75f*dt;
        enemy->velocity.y += dy*0.65f*dt;

        if (fabsf(dx) < 16.0f && fabsf(enemy->position.y - targetHuman->position.y) < 36.0f)
        {
            targetHuman->rescueBonusEligible = true;
            targetHuman->state = HUMAN_STATE_ABDUCTED;
            targetHuman->carrierEnemy = enemyIndex;
            enemy->carryingHuman = true;
        }
    }
    else
    {
        enemy->velocity.x += sinf(enemy->stateTimer + enemyIndex)*85.0f*dt;
        enemy->velocity.y += sinf(enemy->stateTimer*1.3f + enemyIndex*0.7f)*55.0f*dt;
    }

    if (enemy->fireTimer <= 0.0f && game->player.alive)
    {
        dx = fabsf(WorldWrapDeltaX(&game->world, game->player.position.x - enemy->position.x));
        if (dx < 380.0f)
        {
            FireAtPlayer(game, enemy, 280.0f, 5.0f, 3.0f);
            enemy->fireTimer = RandomFloat(1.35f, 2.2f);
        }
    }
}

static void UpdateHunter(Game *game, Enemy *enemy, float dt, float turnStrength, float shotSpeed, float fireDelay)
{
    float dx = WorldWrapDeltaX(&game->world, game->player.position.x - enemy->position.x);
    float dy = game->player.position.y - enemy->position.y;
    Vector2 desiredVelocity = Vec2Scale(Vec2NormalizeSafe(Vec2(dx, dy)), enemy->speed);

    enemy->velocity.x = LerpFloat(enemy->velocity.x, desiredVelocity.x, ClampFloat(dt*turnStrength, 0.0f, 1.0f));
    enemy->velocity.y = LerpFloat(enemy->velocity.y, desiredVelocity.y, ClampFloat(dt*turnStrength, 0.0f, 1.0f));

    if (enemy->fireTimer <= 0.0f && game->player.alive)
    {
        FireAtPlayer(game, enemy, shotSpeed, 5.5f, 3.0f);
        enemy->fireTimer = fireDelay;
    }
}

static void UpdateBomber(Game *game, Enemy *enemy, float dt)
{
    float targetY = 180.0f + sinf(enemy->stateTimer*0.8f)*70.0f;
    enemy->velocity.x = (enemy->velocity.x >= 0.0f ? 1.0f : -1.0f)*enemy->speed;
    enemy->velocity.y += (targetY - enemy->position.y)*0.9f*dt;

    if (enemy->fireTimer <= 0.0f)
    {
        Vector2 mineVelocity = Vec2(RandomFloat(-20.0f, 20.0f), RandomFloat(35.0f, 65.0f));
        GameSpawnProjectile(game, enemy->position, mineVelocity, false, 9.0f, 6.0f, ENEMY_BOMBER);
        enemy->fireTimer = RandomFloat(1.8f, 2.8f);
    }
}

static void UpdatePod(Game *game, Enemy *enemy, float dt)
{
    float dx = WorldWrapDeltaX(&game->world, game->player.position.x - enemy->position.x);
    enemy->velocity.x = LerpFloat(enemy->velocity.x, SignFloat(dx)*enemy->speed, ClampFloat(dt*0.7f, 0.0f, 1.0f));
    enemy->velocity.y += sinf(enemy->stateTimer*1.1f)*30.0f*dt;

    if (enemy->fireTimer <= 0.0f && game->player.alive)
    {
        FireAtPlayer(game, enemy, 240.0f, 6.0f, 4.5f);
        enemy->fireTimer = RandomFloat(1.9f, 3.0f);
    }
}

static void UpdateSwarmer(Game *game, Enemy *enemy, float dt)
{
    float dx = WorldWrapDeltaX(&game->world, game->player.position.x - enemy->position.x);
    float dy = game->player.position.y - enemy->position.y;
    Vector2 desiredVelocity = Vec2Scale(Vec2NormalizeSafe(Vec2(dx, dy + sinf(enemy->stateTimer*6.5f)*50.0f)), enemy->speed);

    enemy->velocity.x = LerpFloat(enemy->velocity.x, desiredVelocity.x, ClampFloat(dt*7.5f, 0.0f, 1.0f));
    enemy->velocity.y = LerpFloat(enemy->velocity.y, desiredVelocity.y, ClampFloat(dt*6.0f, 0.0f, 1.0f));
}

void UpdateEnemies(Game *game, float dt)
{
    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        Enemy *enemy = &game->enemies[i];
        float terrain;

        if (!enemy->active) continue;

        enemy->stateTimer += dt;
        enemy->fireTimer -= dt;

        switch (enemy->type)
        {
            case ENEMY_LANDER:
                UpdateLander(game, enemy, i, dt);
                break;

            case ENEMY_MUTANT:
                UpdateHunter(game, enemy, dt, 4.0f, 320.0f, 1.2f);
                break;

            case ENEMY_BAITER:
                UpdateHunter(game, enemy, dt, 6.5f, 360.0f, 0.8f);
                break;

            case ENEMY_BOMBER:
                UpdateBomber(game, enemy, dt);
                break;

            case ENEMY_SWARMER:
                UpdateSwarmer(game, enemy, dt);
                break;

            case ENEMY_POD:
                UpdatePod(game, enemy, dt);
                break;
        }

        enemy->position = Vec2Add(enemy->position, Vec2Scale(enemy->velocity, dt));
        enemy->position.x = WorldWrapX(&game->world, enemy->position.x);

        terrain = WorldGetTerrainHeight(&game->world, enemy->position.x) - enemy->radius - 18.0f;
        enemy->position.y = ClampFloat(enemy->position.y, 60.0f, terrain);

        if (enemy->type != ENEMY_BOMBER && enemy->type != ENEMY_POD)
        {
            enemy->velocity.y = ClampFloat(enemy->velocity.y, -220.0f, 220.0f);
        }
        else
        {
            enemy->velocity.y = ClampFloat(enemy->velocity.y, -110.0f, 110.0f);
        }

        enemy->velocity.x = ClampFloat(enemy->velocity.x, -enemy->speed, enemy->speed);

        if (game->player.alive && game->player.invulnerabilityTimer <= 0.0f)
        {
            float dx = WorldWrapDeltaX(&game->world, enemy->position.x - game->player.position.x);
            float dy = enemy->position.y - game->player.position.y;
            float distanceSq = dx*dx + dy*dy;
            float radius = enemy->radius + game->player.radius - 4.0f;

            if (distanceSq < radius*radius)
            {
                GameKillPlayer(game, game->player.position);
                if (enemy->type != ENEMY_POD) GameDamageEnemy(game, i, 1);
            }
        }
    }
}
