#include "effects.h"

#include <string.h>

#include "world.h"

static float RandomFloat(float minValue, float maxValue)
{
    return minValue + (maxValue - minValue)*((float)GetRandomValue(0, 10000)/10000.0f);
}

void InitPalette(RenderPalette *palette)
{
    palette->background = (Color){ 5, 7, 16, 255 };
    palette->backdropTop = (Color){ 14, 24, 54, 255 };
    palette->backdropBottom = (Color){ 4, 7, 18, 255 };
    palette->terrainFill = (Color){ 11, 24, 21, 255 };
    palette->terrainGlow = (Color){ 72, 224, 168, 255 };
    palette->playerPrimary = (Color){ 112, 246, 255, 255 };
    palette->playerAccent = (Color){ 240, 252, 255, 255 };
    palette->human = (Color){ 255, 228, 156, 255 };
    palette->lander = (Color){ 146, 255, 173, 255 };
    palette->mutant = (Color){ 255, 109, 109, 255 };
    palette->bomber = (Color){ 255, 193, 92, 255 };
    palette->baiter = (Color){ 255, 94, 176, 255 };
    palette->swarmer = (Color){ 255, 156, 71, 255 };
    palette->pod = (Color){ 175, 143, 255, 255 };
    palette->projectilePlayer = (Color){ 132, 250, 255, 255 };
    palette->projectileEnemy = (Color){ 255, 110, 94, 255 };
    palette->mine = (Color){ 255, 182, 77, 255 };
    palette->warning = (Color){ 255, 99, 136, 255 };
    palette->hud = (Color){ 216, 238, 255, 255 };
}

void SpawnParticle(Game *game, Vector2 position, Vector2 velocity, float life, float size, float sizeVelocity, Color color)
{
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        Particle *particle = &game->particles[i];

        if (!particle->active)
        {
            particle->active = true;
            particle->position = position;
            particle->velocity = velocity;
            particle->life = life;
            particle->maxLife = life;
            particle->size = size;
            particle->sizeVelocity = sizeVelocity;
            particle->color = color;
            return;
        }
    }
}

void SpawnBurst(Game *game, Vector2 position, Color color, int count, float minSpeed, float maxSpeed, float size)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = RandomFloat(0.0f, TAU);
        float speed = RandomFloat(minSpeed, maxSpeed);
        Vector2 velocity = Vec2(cosf(angle)*speed, sinf(angle)*speed);
        float life = RandomFloat(0.25f, 0.75f);
        float particleSize = size*RandomFloat(0.8f, 1.3f);

        SpawnParticle(game, position, velocity, life, particleSize, -particleSize*0.7f, color);
    }
}

void SpawnThrusterParticles(Game *game, Vector2 position, int facing, Color color)
{
    for (int i = 0; i < 2; ++i)
    {
        Vector2 velocity = Vec2(RandomFloat(-55.0f, -20.0f)*(float)facing,
                                RandomFloat(-30.0f, 30.0f));
        SpawnParticle(game, position, velocity, RandomFloat(0.12f, 0.25f), RandomFloat(2.0f, 4.0f), -7.5f, color);
    }
}

void UpdateEffects(Game *game, float dt)
{
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        Particle *particle = &game->particles[i];

        if (!particle->active) continue;

        particle->life -= dt;
        if (particle->life <= 0.0f)
        {
            particle->active = false;
            continue;
        }

        particle->position = Vec2Add(particle->position, Vec2Scale(particle->velocity, dt));
        particle->velocity.y += 18.0f*dt;
        particle->size += particle->sizeVelocity*dt;
        if (particle->size < 0.5f) particle->size = 0.5f;
    }

    for (int i = 0; i < MAX_CALLOUTS; ++i)
    {
        FloatingText *callout = &game->callouts[i];

        if (!callout->active) continue;

        callout->life -= dt;
        if (callout->life <= 0.0f)
        {
            callout->active = false;
            continue;
        }

        callout->position = Vec2Add(callout->position, Vec2Scale(callout->velocity, dt));
        callout->velocity.y -= 12.0f*dt;
    }

    if (game->camera.shakeTimer > 0.0f)
    {
        game->camera.shakeTimer -= dt;
        if (game->camera.shakeTimer <= 0.0f)
        {
            game->camera.shakeTimer = 0.0f;
            game->camera.shakeOffset = Vec2(0.0f, 0.0f);
        }
        else
        {
            float scale = game->camera.shakeStrength*(game->camera.shakeTimer/0.24f);
            game->camera.shakeOffset = Vec2(RandomFloat(-1.0f, 1.0f)*scale,
                                            RandomFloat(-1.0f, 1.0f)*scale);
        }
    }
    else
    {
        game->camera.shakeOffset = Vec2(0.0f, 0.0f);
    }
}

void UpdateGameCamera(Game *game, float dt)
{
    float targetX = game->camera.center.x;
    float targetY = 320.0f;

    if (game->player.alive)
    {
        targetX = game->player.position.x + 140.0f*(float)game->player.facing;
        targetY = ClampFloat(game->player.position.y + 10.0f, 180.0f, 380.0f);
    }

    game->camera.center.x += WorldWrapDeltaX(&game->world, targetX - game->camera.center.x)*ClampFloat(dt*6.0f, 0.0f, 1.0f);
    game->camera.center.x = WorldWrapX(&game->world, game->camera.center.x);
    game->camera.center.y = LerpFloat(game->camera.center.y, targetY, ClampFloat(dt*5.0f, 0.0f, 1.0f));
}

void AddCameraShake(Game *game, float strength)
{
    if (strength > game->camera.shakeStrength)
    {
        game->camera.shakeStrength = strength;
    }

    if (game->camera.shakeTimer < 0.24f)
    {
        game->camera.shakeTimer = 0.24f;
    }
}
