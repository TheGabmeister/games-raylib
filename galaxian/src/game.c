#include "game.h"
#include "collision.h"
#include "ui.h"
#include "draw_utils.h"
#include <string.h>

float GameDifficulty(int stage)
{
    float t = (float)(stage - 1) / (float)(MAX_DIFFICULTY_STAGE - 1);
    return Clampf(t, 0, 1);
}

static bool AllEnemiesDead(Game *game)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].alive) return false;
    }
    return true;
}

void GameInit(Game *game)
{
    memset(game, 0, sizeof(*game));
    game->screen = SCREEN_TITLE;
    game->score = 0;
    game->lives = PLAYER_START_LIVES;
    game->stage = 1;
    game->difficulty = GameDifficulty(1);
    game->bonus_life_awarded = false;

    PlayerInit(&game->player);
    BulletPoolInit(&game->bullets);
    FormationInit(&game->formation);
    StarfieldInit(&game->starfield);
    EffectsInit(&game->effects);
    game->particle_count = 0;
}

void GameStartStage(Game *game)
{
    game->difficulty = GameDifficulty(game->stage);
    game->stage_clearing = false;
    game->stage_clear_timer = 0;

    BulletPoolInit(&game->bullets);
    game->particle_count = 0;

    FormationInitStage(game);
}

void GameUpdate(Game *game, float dt)
{
    switch (game->screen) {
    case SCREEN_TITLE:
        StarfieldUpdate(&game->starfield, dt);
        if (IsKeyPressed(KEY_SPACE)) {
            game->screen = SCREEN_GAMEPLAY;
            game->score = 0;
            game->lives = PLAYER_START_LIVES;
            game->stage = 1;
            game->bonus_life_awarded = false;
            game->gameover_timer = 0;
            PlayerInit(&game->player);
            GameStartStage(game);
        }
        break;

    case SCREEN_GAMEPLAY:
        /* 1. Effects */
        EffectsUpdate(&game->effects, dt);

        /* 2. Starfield */
        StarfieldUpdate(&game->starfield, dt);

        /* 3. Player */
        PlayerUpdate(&game->player, dt);

        /* Player fire */
        if (game->player.alive && IsKeyPressed(KEY_SPACE)) {
            Vector2 muzzle = { game->player.position.x, game->player.position.y - 16 };
            FirePlayerBullet(&game->bullets, muzzle);
        }

        /* Thrust particles */
        if (game->player.alive) {
            EmitThrust(game->particles, &game->particle_count,
                       (Vector2){ game->player.position.x, game->player.position.y + 14 });
        }

        /* 4-5. Formation + enemies */
        FormationUpdate(game, dt);

        /* 6. Bullets */
        BulletPoolUpdate(&game->bullets, dt);

        /* 7. Particles */
        ParticlesUpdate(game->particles, &game->particle_count, dt);

        /* 8. Collisions */
        CheckCollisions(game);

        /* 9. Stage clear / game over */
        if (game->stage_clearing) {
            game->stage_clear_timer -= dt;
            if (game->stage_clear_timer <= 0) {
                game->stage++;
                if (game->stage > 999) game->stage = 1;
                GameStartStage(game);
            }
        } else if (AllEnemiesDead(game)) {
            game->stage_clearing = true;
            game->stage_clear_timer = 2.0f;
        }

        if (game->lives <= 0 && !game->player.alive) {
            game->gameover_timer += dt;
            if (game->gameover_timer > 2.0f) {
                game->screen = SCREEN_GAMEOVER;
            }
        }
        break;

    case SCREEN_GAMEOVER:
        StarfieldUpdate(&game->starfield, dt);
        if (IsKeyPressed(KEY_SPACE)) {
            game->screen = SCREEN_TITLE;
            game->gameover_timer = 0;
        }
        break;
    }
}

void GameDraw(Game *game)
{
    switch (game->screen) {
    case SCREEN_TITLE:
        StarfieldDraw(&game->starfield);
        DrawTitleScreen();
        break;

    case SCREEN_GAMEPLAY:
        /* 1. Screen shake */
        EffectsApplyShake(&game->effects);

        /* 2. Starfield */
        StarfieldDraw(&game->starfield);

        /* 3. Bullets */
        BulletPoolDraw(&game->bullets);

        /* 4. Enemies */
        for (int i = 0; i < MAX_ENEMIES; i++)
            EnemyDraw(&game->enemies[i]);

        /* 5. Player */
        PlayerDraw(&game->player);

        /* 6. Particles (additive) */
        ParticlesDraw(game->particles, game->particle_count);

        /* Reset shake before HUD so HUD is stable */
        EffectsResetShake(&game->effects);

        /* 7. HUD */
        DrawHUD(game);

        /* 8. Flash overlay */
        EffectsDrawFlash(&game->effects);
        break;

    case SCREEN_GAMEOVER:
        StarfieldDraw(&game->starfield);
        /* Draw remaining game state behind overlay */
        for (int i = 0; i < MAX_ENEMIES; i++)
            EnemyDraw(&game->enemies[i]);
        ParticlesDraw(game->particles, game->particle_count);
        DrawGameOverScreen(game->score);
        break;
    }
}
