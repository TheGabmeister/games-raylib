#include "game.h"

#include <stdio.h>
#include <string.h>

#include "effects.h"
#include "enemy.h"
#include "player.h"
#include "render.h"
#include "wave.h"
#include "world.h"

static float RandomFloat(float minValue, float maxValue)
{
    return minValue + (maxValue - minValue)*((float)GetRandomValue(0, 10000)/10000.0f);
}

static int EnemyScoreValue(EnemyType type)
{
    switch (type)
    {
        case ENEMY_LANDER: return 150;
        case ENEMY_MUTANT: return 150;
        case ENEMY_BOMBER: return 250;
        case ENEMY_BAITER: return 200;
        case ENEMY_SWARMER: return 150;
        case ENEMY_POD: return 1000;
    }

    return 0;
}

static void GenerateStars(Vector2 *stars, int count, float maxHeight)
{
    for (int i = 0; i < count; ++i)
    {
        stars[i].x = RandomFloat(0.0f, WORLD_WIDTH);
        stars[i].y = RandomFloat(20.0f, maxHeight);
    }
}

static void BeginNewRun(Game *game)
{
    memset(game->humans, 0, sizeof(game->humans));
    memset(game->enemies, 0, sizeof(game->enemies));
    memset(game->projectiles, 0, sizeof(game->projectiles));
    memset(game->particles, 0, sizeof(game->particles));
    memset(game->callouts, 0, sizeof(game->callouts));

    game->score = 0;
    game->nextExtraLifeScore = 10000;
    game->wave.index = 1;
    game->wave.clearTimer = 0.0f;
    game->titleTimer = 0.0f;
    game->mode = GAME_MODE_PLAYING;

    ResetPlayerForNewRun(game);
    game->camera.center = game->player.position;
    SpawnWave(game);
}

static bool IsWaveResolved(const Game *game)
{
    if (GameActiveEnemyCount(game) > 0) return false;

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        const Human *human = &game->humans[i];

        if (!human->active) continue;
        if (human->state == HUMAN_STATE_FALLING || human->state == HUMAN_STATE_ABDUCTED || human->state == HUMAN_STATE_CARRIED)
        {
            return false;
        }
    }

    return true;
}

static void UpdateHumans(Game *game, float dt)
{
    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        Human *human = &game->humans[i];
        float terrain;

        if (!human->active) continue;

        switch (human->state)
        {
            case HUMAN_STATE_GROUNDED:
                human->position.y = WorldGetTerrainHeight(&game->world, human->position.x) - 10.0f;
                human->velocity = Vec2(0.0f, 0.0f);
                break;

            case HUMAN_STATE_FALLING:
                human->velocity.y += 520.0f*dt;
                human->position = Vec2Add(human->position, Vec2Scale(human->velocity, dt));
                human->position.x = WorldWrapX(&game->world, human->position.x);
                terrain = WorldGetTerrainHeight(&game->world, human->position.x);

                if (human->position.y >= terrain - 10.0f)
                {
                    if (fabsf(human->velocity.y) <= 90.0f)
                    {
                        human->state = HUMAN_STATE_GROUNDED;
                        human->rescueBonusEligible = false;
                        human->position.y = terrain - 10.0f;
                        human->velocity = Vec2(0.0f, 0.0f);
                    }
                    else
                    {
                        GameSpawnExplosion(game, human->position, game->palette.warning, 10, 120.0f);
                        human->active = false;
                        human->rescueBonusEligible = false;
                        human->state = HUMAN_STATE_DEAD;
                    }
                }
                break;

            case HUMAN_STATE_CARRIED:
            case HUMAN_STATE_ABDUCTED:
                break;

            case HUMAN_STATE_DEAD:
            case HUMAN_STATE_INACTIVE:
                human->active = false;
                break;
        }
    }
}

static void UpdateProjectiles(Game *game, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i)
    {
        Projectile *projectile = &game->projectiles[i];
        float terrain;

        if (!projectile->active) continue;

        projectile->life -= dt;
        if (projectile->life <= 0.0f)
        {
            projectile->active = false;
            continue;
        }

        projectile->position = Vec2Add(projectile->position, Vec2Scale(projectile->velocity, dt));
        projectile->position.x = WorldWrapX(&game->world, projectile->position.x);

        if (!projectile->fromPlayer && projectile->ownerType == ENEMY_BOMBER)
        {
            projectile->velocity.y += 10.0f*dt;
        }

        if (projectile->position.y < -20.0f || projectile->position.y > WORLD_HEIGHT + 20.0f)
        {
            projectile->active = false;
            continue;
        }

        terrain = WorldGetTerrainHeight(&game->world, projectile->position.x);
        if (projectile->position.y >= terrain)
        {
            GameSpawnImpact(game, projectile->position, projectile->fromPlayer ? game->palette.playerPrimary : game->palette.warning);
            projectile->active = false;
        }
    }
}

static void ResolveProjectileCollisions(Game *game)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i)
    {
        Projectile *projectile = &game->projectiles[i];

        if (!projectile->active) continue;

        if (projectile->fromPlayer)
        {
            for (int enemyIndex = 0; enemyIndex < MAX_ENEMIES; ++enemyIndex)
            {
                Enemy *enemy = &game->enemies[enemyIndex];
                float dx;
                float dy;
                float radius;

                if (!enemy->active) continue;

                dx = WorldWrapDeltaX(&game->world, projectile->position.x - enemy->position.x);
                dy = projectile->position.y - enemy->position.y;
                radius = projectile->radius + enemy->radius;

                if (dx*dx + dy*dy < radius*radius)
                {
                    projectile->active = false;
                    GameDamageEnemy(game, enemyIndex, (enemy->type == ENEMY_POD) ? 1 : 99);
                    break;
                }
            }
        }
        else if (game->player.alive && game->player.invulnerabilityTimer <= 0.0f)
        {
            float dx = WorldWrapDeltaX(&game->world, projectile->position.x - game->player.position.x);
            float dy = projectile->position.y - game->player.position.y;
            float radius = projectile->radius + game->player.radius;

            if (dx*dx + dy*dy < radius*radius)
            {
                projectile->active = false;
                GameKillPlayer(game, projectile->position);
            }
        }
    }
}

static void UpdateRespawn(Game *game, float dt)
{
    if (game->player.alive) return;

    if (game->player.respawnTimer > 0.0f)
    {
        game->player.respawnTimer -= dt;
        if (game->player.respawnTimer <= 0.0f)
        {
            if (game->player.lives > 0)
            {
                RespawnPlayer(game, game->camera.center.x);
            }
            else
            {
                game->mode = GAME_MODE_GAME_OVER;
            }
        }
    }
}

static void HandleWaveProgression(Game *game, float dt)
{
    if (game->mode == GAME_MODE_PLAYING)
    {
        if (game->wave.bannerTimer > 0.0f)
        {
            game->wave.bannerTimer = ClampFloat(game->wave.bannerTimer - dt, 0.0f, 10.0f);
        }

        if (game->wave.index >= 5)
        {
            game->spawnDirector.baiterTimer -= dt;
            if (game->spawnDirector.baiterTimer <= 0.0f)
            {
                GameSpawnEnemy(game, ENEMY_BAITER, WorldFindSafePosition(&game->world, game->player.position.x + 520.0f, 180.0f));
                GameSpawnFloatingText(game, game->player.position, "BAITER", game->palette.baiter);
                game->spawnDirector.baiterTimer = game->spawnDirector.baiterInterval;
            }
        }

        if (IsWaveResolved(game))
        {
            int survivors = GameLivingHumanCount(game);

            if (!game->wave.survivorBonusAwarded)
            {
                GameAddScore(game, survivors*1000, game->player.position);
                game->wave.survivorBonusAwarded = 1;
            }

            game->mode = GAME_MODE_WAVE_CLEAR;
            game->wave.clearTimer = 2.4f;
        }
    }
    else if (game->mode == GAME_MODE_WAVE_CLEAR)
    {
        game->wave.clearTimer -= dt;
        if (game->input.pressed[INPUT_ACTION_CONFIRM] || game->wave.clearTimer <= 0.0f)
        {
            game->wave.index += 1;
            game->mode = GAME_MODE_PLAYING;
            RespawnPlayer(game, game->player.position.x + 220.0f);
            SpawnWave(game);
        }
    }
}

void GameInit(Game *game)
{
    memset(game, 0, sizeof(*game));

    WorldInit(&game->world);
    InitPalette(&game->palette);

    game->mode = GAME_MODE_TITLE;
    game->wave.index = 1;
    game->camera.center = Vec2(game->world.width*0.5f, 320.0f);
    game->player.position = Vec2(game->world.width*0.5f, 220.0f);
    game->player.radius = 18.0f;
    game->player.facing = 1;
    game->player.lives = 3;
    game->player.smartBombs = 3;
    game->player.carriedHuman = -1;

    GenerateStars(game->starsFar, STAR_LAYER_FAR_COUNT, 300.0f);
    GenerateStars(game->starsMid, STAR_LAYER_MID_COUNT, 360.0f);
    GenerateStars(game->starsNear, STAR_LAYER_NEAR_COUNT, 420.0f);
}

void GameShutdown(Game *game)
{
    (void)game;
}

int GameSpawnEnemy(Game *game, EnemyType type, Vector2 position)
{
    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        Enemy *enemy = &game->enemies[i];

        if (!enemy->active)
        {
            memset(enemy, 0, sizeof(*enemy));
            enemy->active = true;
            enemy->type = type;
            enemy->position = position;
            enemy->targetHuman = -1;

            switch (type)
            {
                case ENEMY_LANDER:
                    enemy->radius = 16.0f;
                    enemy->speed = 180.0f;
                    enemy->hp = 1;
                    enemy->velocity = Vec2(RandomFloat(-80.0f, 80.0f), 0.0f);
                    enemy->fireTimer = RandomFloat(1.0f, 2.0f);
                    break;

                case ENEMY_MUTANT:
                    enemy->radius = 18.0f;
                    enemy->speed = 310.0f;
                    enemy->hp = 1;
                    enemy->fireTimer = 0.7f;
                    break;

                case ENEMY_BOMBER:
                    enemy->radius = 18.0f;
                    enemy->speed = 135.0f;
                    enemy->hp = 2;
                    enemy->velocity = Vec2((GetRandomValue(0, 1) == 0) ? -135.0f : 135.0f, 0.0f);
                    enemy->fireTimer = RandomFloat(1.2f, 2.4f);
                    break;

                case ENEMY_BAITER:
                    enemy->radius = 17.0f;
                    enemy->speed = 360.0f;
                    enemy->hp = 1;
                    enemy->fireTimer = 0.5f;
                    break;

                case ENEMY_SWARMER:
                    enemy->radius = 11.0f;
                    enemy->speed = 390.0f;
                    enemy->hp = 1;
                    break;

                case ENEMY_POD:
                    enemy->radius = 20.0f;
                    enemy->speed = 150.0f;
                    enemy->hp = 3;
                    enemy->fireTimer = RandomFloat(1.4f, 2.4f);
                    enemy->velocity = Vec2((GetRandomValue(0, 1) == 0) ? -100.0f : 100.0f, 0.0f);
                    break;
            }

            return i;
        }
    }

    return -1;
}

void GameSpawnProjectile(Game *game, Vector2 position, Vector2 velocity, bool fromPlayer, float radius, float life, EnemyType ownerType)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i)
    {
        Projectile *projectile = &game->projectiles[i];

        if (!projectile->active)
        {
            projectile->active = true;
            projectile->fromPlayer = fromPlayer;
            projectile->position = position;
            projectile->velocity = velocity;
            projectile->life = life;
            projectile->radius = radius;
            projectile->ownerType = ownerType;
            return;
        }
    }
}

void GameSpawnExplosion(Game *game, Vector2 position, Color color, int count, float speedScale)
{
    SpawnBurst(game, position, color, count, speedScale*0.35f, speedScale, 4.0f);
    SpawnBurst(game, position, Fade(WHITE, 0.8f), count/2, speedScale*0.2f, speedScale*0.5f, 2.5f);
    AddCameraShake(game, ClampFloat(speedScale/220.0f, 3.0f, 9.0f));
}

void GameSpawnImpact(Game *game, Vector2 position, Color color)
{
    SpawnBurst(game, position, color, 6, 50.0f, 140.0f, 2.4f);
}

void GameSpawnFloatingText(Game *game, Vector2 position, const char *text, Color color)
{
    for (int i = 0; i < MAX_CALLOUTS; ++i)
    {
        FloatingText *callout = &game->callouts[i];

        if (!callout->active)
        {
            memset(callout, 0, sizeof(*callout));
            callout->active = true;
            snprintf(callout->text, sizeof(callout->text), "%s", text);
            callout->position = position;
            callout->velocity = Vec2(0.0f, -28.0f);
            callout->life = 1.1f;
            callout->maxLife = 1.1f;
            callout->color = color;
            return;
        }
    }
}

void GameAddScore(Game *game, int amount, Vector2 position)
{
    char scoreText[24];

    game->score += amount;
    snprintf(scoreText, sizeof(scoreText), "%d", amount);
    GameSpawnFloatingText(game, position, scoreText, game->palette.playerAccent);

    while (game->score >= game->nextExtraLifeScore)
    {
        game->player.lives += 1;
        game->nextExtraLifeScore += 10000;
        GameSpawnFloatingText(game, position, "1UP", game->palette.playerPrimary);
    }
}

void GameDamageEnemy(Game *game, int enemyIndex, int damage)
{
    Enemy *enemy;

    if (enemyIndex < 0 || enemyIndex >= MAX_ENEMIES) return;

    enemy = &game->enemies[enemyIndex];
    if (!enemy->active) return;

    enemy->hp -= damage;
    GameSpawnImpact(game, enemy->position, game->palette.warning);
    if (enemy->hp <= 0)
    {
        GameKillEnemy(game, enemyIndex);
    }
}

void GameKillEnemy(Game *game, int enemyIndex)
{
    Enemy *enemy;
    int scoreValue;

    if (enemyIndex < 0 || enemyIndex >= MAX_ENEMIES) return;

    enemy = &game->enemies[enemyIndex];
    if (!enemy->active) return;

    if (enemy->type == ENEMY_LANDER && enemy->carryingHuman && enemy->targetHuman >= 0)
    {
        Human *human = &game->humans[enemy->targetHuman];
        if (human->active)
        {
            human->rescueBonusEligible = true;
            human->state = HUMAN_STATE_FALLING;
            human->carrierEnemy = -1;
            human->velocity = Vec2(0.0f, 90.0f);
        }
    }

    scoreValue = EnemyScoreValue(enemy->type);
    GameAddScore(game, scoreValue, enemy->position);
    GameSpawnExplosion(game, enemy->position, game->palette.warning, (enemy->type == ENEMY_POD) ? 20 : 14, 210.0f);

    if (enemy->type == ENEMY_POD)
    {
        for (int i = 0; i < 3; ++i)
        {
            Vector2 offset = Vec2(RandomFloat(-24.0f, 24.0f), RandomFloat(-16.0f, 16.0f));
            int swarmerIndex = GameSpawnEnemy(game, ENEMY_SWARMER, Vec2Add(enemy->position, offset));
            if (swarmerIndex >= 0)
            {
                game->enemies[swarmerIndex].velocity = Vec2(RandomFloat(-180.0f, 180.0f), RandomFloat(-140.0f, 140.0f));
            }
        }
    }

    enemy->active = false;
}

void GameKillPlayer(Game *game, Vector2 position)
{
    if (!game->player.alive || game->player.invulnerabilityTimer > 0.0f) return;

    if (game->player.carriedHuman >= 0)
    {
        Human *human = &game->humans[game->player.carriedHuman];
        if (human->active)
        {
            human->state = HUMAN_STATE_FALLING;
            human->velocity = Vec2(game->player.velocity.x*0.2f, 70.0f);
        }
        game->player.carriedHuman = -1;
    }

    game->player.alive = false;
    game->player.velocity = Vec2(0.0f, 0.0f);
    game->player.respawnTimer = 2.0f;
    game->player.lives -= 1;
    game->player.hitFlashTimer = 0.5f;

    GameSpawnExplosion(game, position, game->palette.warning, 24, 260.0f);

    if (game->player.lives <= 0)
    {
        GameSpawnFloatingText(game, position, "LAST SHIP", game->palette.warning);
    }
}

bool GameUseSmartBomb(Game *game)
{
    int visibleEnemyIndices[MAX_ENEMIES];
    int visibleEnemyCount = 0;

    if (!game->player.alive) return false;
    if (game->player.smartBombs <= 0) return false;

    game->player.smartBombs -= 1;
    GameSpawnExplosion(game, game->player.position, game->palette.playerPrimary, 20, 320.0f);
    GameSpawnFloatingText(game, game->player.position, "SMART BOMB", game->palette.playerPrimary);

    for (int i = 0; i < MAX_PROJECTILES; ++i)
    {
        Projectile *projectile = &game->projectiles[i];
        if (projectile->active && !projectile->fromPlayer)
        {
            projectile->active = false;
        }
    }

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        Enemy *enemy = &game->enemies[i];
        Vector2 screen;

        if (!enemy->active) continue;

        screen = Vec2(WorldWrapDeltaX(&game->world, enemy->position.x - game->camera.center.x) + SCREEN_WIDTH*0.5f,
                      enemy->position.y - game->camera.center.y + SCREEN_HEIGHT*0.5f);

        if (screen.x < -80.0f || screen.x > SCREEN_WIDTH + 80.0f) continue;
        if (screen.y < -80.0f || screen.y > SCREEN_HEIGHT + 80.0f) continue;

        visibleEnemyIndices[visibleEnemyCount] = i;
        visibleEnemyCount += 1;
    }

    for (int i = 0; i < visibleEnemyCount; ++i)
    {
        Enemy *enemy = &game->enemies[visibleEnemyIndices[i]];

        if (!enemy->active) continue;

        if (enemy->type == ENEMY_POD) GameDamageEnemy(game, visibleEnemyIndices[i], 3);
        else GameDamageEnemy(game, visibleEnemyIndices[i], 99);
    }

    return true;
}

bool GameUseHyperspace(Game *game)
{
    float chance;

    if (!game->player.alive) return false;
    if (game->player.hyperspaceCooldown > 0.0f) return false;

    game->player.hyperspaceCooldown = 4.0f;
    chance = 0.15f + 0.10f*(float)game->player.hyperspaceUsesThisLife;
    chance = ClampFloat(chance, 0.15f, 0.95f);
    game->player.hyperspaceUsesThisLife += 1;

    if (RandomFloat(0.0f, 1.0f) < chance)
    {
        GameSpawnFloatingText(game, game->player.position, "HYPERSPACE FAIL", game->palette.warning);
        GameKillPlayer(game, game->player.position);
        return true;
    }

    GameSpawnExplosion(game, game->player.position, game->palette.playerPrimary, 12, 120.0f);
    game->player.position = WorldFindSafePosition(&game->world, RandomFloat(0.0f, WORLD_WIDTH), RandomFloat(160.0f, 260.0f));
    game->player.velocity = Vec2(0.0f, 0.0f);
    game->player.invulnerabilityTimer = ClampFloat(game->player.invulnerabilityTimer, 0.5f, 1.5f);
    GameSpawnExplosion(game, game->player.position, game->palette.playerAccent, 10, 120.0f);
    return true;
}

int GameLivingHumanCount(const Game *game)
{
    int living = 0;

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        if (game->humans[i].active) living += 1;
    }

    return living;
}

int GameActiveEnemyCount(const Game *game)
{
    int count = 0;

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        if (game->enemies[i].active) count += 1;
    }

    return count;
}

void GameUpdate(Game *game, float dt)
{
    game->titleTimer += dt;

    if (game->input.pressed[INPUT_ACTION_PAUSE])
    {
        if (game->mode == GAME_MODE_PLAYING)
        {
            game->mode = GAME_MODE_PAUSED;
        }
        else if (game->mode == GAME_MODE_PAUSED)
        {
            game->mode = GAME_MODE_PLAYING;
        }
    }

    switch (game->mode)
    {
        case GAME_MODE_TITLE:
            UpdateEffects(game, dt);
            UpdateGameCamera(game, dt);
            if (game->input.pressed[INPUT_ACTION_CONFIRM])
            {
                BeginNewRun(game);
            }
            break;

        case GAME_MODE_PAUSED:
            UpdateEffects(game, dt);
            break;

        case GAME_MODE_GAME_OVER:
            UpdateEffects(game, dt);
            UpdateGameCamera(game, dt);
            if (game->input.pressed[INPUT_ACTION_CONFIRM])
            {
                BeginNewRun(game);
            }
            break;

        case GAME_MODE_PLAYING:
            UpdatePlayer(game, dt);
            UpdateEnemies(game, dt);
            UpdateHumans(game, dt);
            UpdateProjectiles(game, dt);
            ResolveProjectileCollisions(game);
            UpdateRespawn(game, dt);
            HandleWaveProgression(game, dt);
            UpdateEffects(game, dt);
            UpdateGameCamera(game, dt);
            break;

        case GAME_MODE_WAVE_CLEAR:
            HandleWaveProgression(game, dt);
            UpdateEffects(game, dt);
            UpdateGameCamera(game, dt);
            break;
    }
}

void GameDraw(const Game *game)
{
    RenderWorld(game);
    RenderHud(game);
}
