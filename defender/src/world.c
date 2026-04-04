#include "world.h"

static void FlattenTerrainAround(WorldState *world, float centerX, float radius, float targetHeight)
{
    for (int i = 0; i < TERRAIN_SAMPLE_COUNT; ++i)
    {
        float sampleX = i*world->sampleSpacing;
        float delta = WorldWrapDeltaX(world, sampleX - centerX);
        float distance = fabsf(delta);

        if (distance < radius)
        {
            float t = 1.0f - distance/radius;
            world->terrainHeights[i] = LerpFloat(world->terrainHeights[i], targetHeight, t*0.85f);
        }
    }
}

void WorldInit(WorldState *world)
{
    world->width = WORLD_WIDTH;
    world->height = WORLD_HEIGHT;
    world->sampleSpacing = world->width/(float)(TERRAIN_SAMPLE_COUNT - 1);

    for (int i = 0; i < TERRAIN_SAMPLE_COUNT; ++i)
    {
        float x = i*world->sampleSpacing;
        float t = x/world->width;
        float height = 560.0f
                     + sinf(t*TAU*2.2f)*42.0f
                     + sinf(t*TAU*5.3f + 0.65f)*22.0f
                     + cosf(t*TAU*1.4f + 1.1f)*28.0f;

        world->terrainHeights[i] = ClampFloat(height, 490.0f, 650.0f);
    }

    for (int i = 0; i < HUMAN_SPAWN_COUNT; ++i)
    {
        float spawnX = 320.0f + i*580.0f;
        float targetHeight = 585.0f + sinf((float)i*0.75f)*18.0f;

        world->humanSpawnXs[i] = WorldWrapX(world, spawnX);
        FlattenTerrainAround(world, world->humanSpawnXs[i], 95.0f, targetHeight);
    }
}

float WorldWrapX(const WorldState *world, float x)
{
    while (x < 0.0f) x += world->width;
    while (x >= world->width) x -= world->width;
    return x;
}

float WorldWrapDeltaX(const WorldState *world, float delta)
{
    float halfWidth = world->width*0.5f;

    if (delta > halfWidth) delta -= world->width;
    if (delta < -halfWidth) delta += world->width;
    return delta;
}

float WorldGetTerrainHeight(const WorldState *world, float x)
{
    float wrappedX = WorldWrapX(world, x);
    int indexA = (int)floorf(wrappedX/world->sampleSpacing);
    int indexB = indexA + 1;
    float localT = (wrappedX - indexA*world->sampleSpacing)/world->sampleSpacing;

    if (indexA < 0) indexA = 0;
    if (indexB >= TERRAIN_SAMPLE_COUNT) indexB = TERRAIN_SAMPLE_COUNT - 1;

    return LerpFloat(world->terrainHeights[indexA], world->terrainHeights[indexB], localT);
}

Vector2 WorldFindSafePosition(const WorldState *world, float desiredX, float desiredY)
{
    static const float searchOffsets[] = { 0.0f, 260.0f, -260.0f, 520.0f, -520.0f, 860.0f, -860.0f };
    Vector2 best = Vec2(WorldWrapX(world, desiredX), ClampFloat(desiredY, 120.0f, 320.0f));

    for (int i = 0; i < (int)(sizeof(searchOffsets)/sizeof(searchOffsets[0])); ++i)
    {
        float candidateX = WorldWrapX(world, desiredX + searchOffsets[i]);
        float terrain = WorldGetTerrainHeight(world, candidateX);
        float candidateY = ClampFloat(desiredY, 130.0f, terrain - 140.0f);

        if (candidateY < terrain - 110.0f)
        {
            best = Vec2(candidateX, candidateY);
            break;
        }
    }

    return best;
}
