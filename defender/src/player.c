#include "player.h"

#include "effects.h"
#include "game.h"
#include "world.h"

void RespawnPlayer(Game *game, float desiredX)
{
    Player *player = &game->player;
    Vector2 spawn = WorldFindSafePosition(&game->world, desiredX, 220.0f);

    player->position = spawn;
    player->velocity = Vec2(0.0f, 0.0f);
    player->radius = 18.0f;
    player->fireCooldown = 0.0f;
    player->invulnerabilityTimer = 1.5f;
    player->respawnTimer = 0.0f;
    player->hyperspaceCooldown = 0.0f;
    player->hitFlashTimer = 0.0f;
    player->alive = true;
    player->facing = 1;
    player->smartBombs = 3;
    player->carriedHuman = -1;
    player->hyperspaceUsesThisLife = 0;
}

void ResetPlayerForNewRun(Game *game)
{
    Player *player = &game->player;

    player->lives = 3;
    RespawnPlayer(game, game->world.width*0.5f);
}

static void TryPickupHuman(Game *game)
{
    Player *player = &game->player;

    if (player->carriedHuman >= 0) return;

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        Human *human = &game->humans[i];
        HumanState previousState;
        float dx;
        float dy;

        if (!human->active) continue;
        if (human->state != HUMAN_STATE_GROUNDED && human->state != HUMAN_STATE_FALLING) continue;

        dx = WorldWrapDeltaX(&game->world, human->position.x - player->position.x);
        dy = human->position.y - player->position.y;

        if (fabsf(dx) < 26.0f && fabsf(dy) < 36.0f)
        {
            previousState = human->state;
            human->state = HUMAN_STATE_CARRIED;
            human->velocity = Vec2(0.0f, 0.0f);
            human->carrierEnemy = -1;
            player->carriedHuman = i;

            if (previousState == HUMAN_STATE_FALLING && human->rescueBonusEligible)
            {
                GameAddScore(game, 250, player->position);
            }

            return;
        }
    }
}

static void UpdateCarriedHuman(Game *game)
{
    Player *player = &game->player;
    Human *human;
    float terrain;

    if (player->carriedHuman < 0) return;

    human = &game->humans[player->carriedHuman];
    if (!human->active)
    {
        player->carriedHuman = -1;
        return;
    }

    human->position.x = player->position.x - 8.0f*(float)player->facing;
    human->position.y = player->position.y + 22.0f;
    human->velocity = player->velocity;

    terrain = WorldGetTerrainHeight(&game->world, human->position.x);
    if (terrain - player->position.y < 58.0f && fabsf(player->velocity.y) < 80.0f)
    {
        bool awardRescueBonus = human->rescueBonusEligible;

        human->state = HUMAN_STATE_GROUNDED;
        human->rescueBonusEligible = false;
        human->position.y = terrain - 10.0f;
        human->velocity = Vec2(0.0f, 0.0f);
        player->carriedHuman = -1;

        if (awardRescueBonus)
        {
            GameAddScore(game, 500, human->position);
            GameSpawnFloatingText(game, human->position, "RESCUED", game->palette.human);
        }
    }
}

void UpdatePlayer(Game *game, float dt)
{
    Player *player = &game->player;
    float terrain;
    float horizontalAcceleration;
    float gravity = 420.0f;
    Vector2 muzzlePosition;
    Vector2 projectileVelocity;

    if (!player->alive) return;

    player->fireCooldown = ClampFloat(player->fireCooldown - dt, 0.0f, 10.0f);
    player->hyperspaceCooldown = ClampFloat(player->hyperspaceCooldown - dt, 0.0f, 10.0f);
    player->invulnerabilityTimer = ClampFloat(player->invulnerabilityTimer - dt, 0.0f, 10.0f);
    player->hitFlashTimer = ClampFloat(player->hitFlashTimer - dt, 0.0f, 10.0f);

    if (game->input.moveAxis > 0.15f) player->facing = 1;
    if (game->input.moveAxis < -0.15f) player->facing = -1;

    horizontalAcceleration = game->input.moveAxis*1100.0f;
    if (game->input.down[INPUT_ACTION_REVERSE])
    {
        horizontalAcceleration -= SignFloat(player->velocity.x)*900.0f;
        player->velocity.x *= 0.9f;
    }

    if (game->input.down[INPUT_ACTION_THRUST])
    {
        gravity = 300.0f;
        player->velocity.y -= 960.0f*dt;
        SpawnThrusterParticles(game,
                               Vec2(player->position.x - 16.0f*(float)player->facing, player->position.y + 4.0f),
                               player->facing,
                               game->palette.playerPrimary);
    }

    player->velocity.x += horizontalAcceleration*dt;
    player->velocity.y += gravity*dt;

    player->velocity.x *= 0.988f;
    player->velocity.y *= 0.996f;

    player->velocity.x = ClampFloat(player->velocity.x, -480.0f, 480.0f);
    player->velocity.y = ClampFloat(player->velocity.y, -360.0f, 420.0f);

    if (game->input.down[INPUT_ACTION_FIRE] && player->fireCooldown <= 0.0f)
    {
        muzzlePosition = Vec2(player->position.x + 24.0f*(float)player->facing, player->position.y - 1.0f);
        projectileVelocity = Vec2(player->velocity.x + 860.0f*(float)player->facing,
                                  player->velocity.y*0.15f);

        GameSpawnProjectile(game, muzzlePosition, projectileVelocity, true, 3.5f, 1.5f, ENEMY_LANDER);
        player->fireCooldown = 0.12f;
        GameSpawnImpact(game, muzzlePosition, game->palette.playerPrimary);
    }

    if (game->input.pressed[INPUT_ACTION_SMART_BOMB])
    {
        GameUseSmartBomb(game);
    }

    if (game->input.pressed[INPUT_ACTION_HYPERSPACE])
    {
        if (!GameUseHyperspace(game))
        {
            return;
        }

        if (!player->alive)
        {
            return;
        }
    }

    player->position = Vec2Add(player->position, Vec2Scale(player->velocity, dt));
    player->position.x = WorldWrapX(&game->world, player->position.x);
    player->position.y = ClampFloat(player->position.y, 36.0f, game->world.height - 40.0f);

    terrain = WorldGetTerrainHeight(&game->world, player->position.x);
    if (player->position.y + player->radius >= terrain)
    {
        GameKillPlayer(game, player->position);
        return;
    }

    TryPickupHuman(game);
    UpdateCarriedHuman(game);
}
