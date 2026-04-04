#include "wave.h"

#include <string.h>

#include "game.h"
#include "world.h"

static Vector2 RandomSpawnPosition(Game *game, float minY, float maxY)
{
    float x = (float)GetRandomValue(0, (int)WORLD_WIDTH - 1);
    float y = (float)GetRandomValue((int)minY, (int)maxY);

    if (fabsf(WorldWrapDeltaX(&game->world, x - game->player.position.x)) < 320.0f)
    {
        x = WorldWrapX(&game->world, x + 480.0f);
    }

    return Vec2(x, y);
}

void SpawnWave(Game *game)
{
    int landers = 5;
    int bombers = 0;
    int pods = 0;

    memset(game->enemies, 0, sizeof(game->enemies));
    memset(game->projectiles, 0, sizeof(game->projectiles));
    memset(game->humans, 0, sizeof(game->humans));
    memset(game->particles, 0, sizeof(game->particles));
    memset(game->callouts, 0, sizeof(game->callouts));

    if (game->wave.index == 2)
    {
        landers = 6;
        bombers = 1;
    }
    else if (game->wave.index == 3)
    {
        landers = 6;
        bombers = 2;
        pods = 1;
    }
    else if (game->wave.index == 4)
    {
        landers = 8;
        bombers = 2;
        pods = 2;
    }
    else if (game->wave.index >= 5)
    {
        landers = 7 + game->wave.index;
        bombers = 2 + (game->wave.index - 3)/2;
        pods = 1 + (game->wave.index - 2)/3;
    }

    for (int i = 0; i < HUMAN_SPAWN_COUNT; ++i)
    {
        Human *human = &game->humans[i];
        float x = game->world.humanSpawnXs[i];
        float y = WorldGetTerrainHeight(&game->world, x) - 10.0f;

        human->active = true;
        human->rescueBonusEligible = false;
        human->state = HUMAN_STATE_GROUNDED;
        human->position = Vec2(x, y);
        human->velocity = Vec2(0.0f, 0.0f);
        human->carrierEnemy = -1;
    }

    for (int i = 0; i < landers; ++i)
    {
        GameSpawnEnemy(game, ENEMY_LANDER, RandomSpawnPosition(game, 110.0f, 280.0f));
    }

    for (int i = 0; i < bombers; ++i)
    {
        GameSpawnEnemy(game, ENEMY_BOMBER, RandomSpawnPosition(game, 120.0f, 240.0f));
    }

    for (int i = 0; i < pods; ++i)
    {
        GameSpawnEnemy(game, ENEMY_POD, RandomSpawnPosition(game, 120.0f, 260.0f));
    }

    game->wave.bannerTimer = 2.4f;
    game->wave.clearTimer = 0.0f;
    game->wave.survivorBonusAwarded = 0;
    game->spawnDirector.baiterInterval = (game->wave.index >= 5) ? ClampFloat(25.0f - (float)(game->wave.index - 5)*2.0f, 10.0f, 25.0f) : 9999.0f;
    game->spawnDirector.baiterTimer = game->spawnDirector.baiterInterval;
}
