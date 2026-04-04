#include "collision.h"
#include "game.h"
#include "draw_utils.h"

void CheckCollisions(struct Game *game)
{
    /* === Player bullet vs enemies === */
    if (game->bullets.player_bullet.active) {
        Vector2 bp = game->bullets.player_bullet.position;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->alive || e->state == ENEMY_INACTIVE) continue;
            /* Skip enemies still waiting to enter */
            if (e->state == ENEMY_ENTERING && e->enter_timer < e->enter_delay) continue;

            float r = EnemyCollisionRadius(e->type);
            if (CheckCollisionCircles(bp, PLAYER_BULLET_RADIUS, e->position, r)) {
                /* Kill enemy */
                game->bullets.player_bullet.active = false;

                int score;
                if (e->state == ENEMY_IN_FORMATION)
                    score = EnemyScoreFormation(e->type);
                else
                    score = EnemyScoreDiving(e);

                game->score += score;
                EmitExplosion(game->particles, &game->particle_count, e->position, EnemyColor(e->type));
                EmitScorePopup(game->particles, &game->particle_count, e->position, score);
                EffectsTriggerShake(&game->effects, SHAKE_INTENSITY * 0.5f);

                /* Bonus life */
                if (!game->bonus_life_awarded && game->score >= BONUS_LIFE_SCORE) {
                    game->bonus_life_awarded = true;
                    game->lives++;
                }

                /* If this was an escort, notify the flagship */
                if (e->is_escort && e->escort_of >= 0 && e->escort_of < MAX_ENEMIES) {
                    Enemy *flagship = &game->enemies[e->escort_of];
                    if (flagship->alive)
                        flagship->escorts_killed++;
                }

                e->alive = false;
                e->state = ENEMY_INACTIVE;
                break;
            }
        }
    }

    /* === Enemy bullets vs player === */
    if (game->player.alive && game->player.invincible_timer <= 0) {
        for (int i = 0; i < game->bullets.enemy_bullet_count; i++) {
            Vector2 bp = game->bullets.enemy_bullets[i].position;
            if (CheckCollisionCircles(bp, ENEMY_BULLET_RADIUS,
                                      game->player.position, PLAYER_COLLISION_RADIUS))
            {
                /* Kill player */
                game->player.alive = false;
                game->lives--;
                if (game->lives > 0) {
                    game->player.respawn_timer = RESPAWN_DELAY;
                }
                EmitExplosion(game->particles, &game->particle_count,
                              game->player.position, COLOR_PLAYER);
                EffectsTriggerShake(&game->effects, SHAKE_INTENSITY);
                EffectsTriggerFlash(&game->effects);

                /* Remove the bullet */
                game->bullets.enemy_bullets[i] = game->bullets.enemy_bullets[game->bullets.enemy_bullet_count - 1];
                game->bullets.enemy_bullet_count--;
                break;
            }
        }
    }

    /* === Diving enemies vs player (body contact / kamikaze) === */
    if (game->player.alive && game->player.invincible_timer <= 0) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->alive || e->state != ENEMY_DIVING) continue;

            float r = EnemyCollisionRadius(e->type);
            if (CheckCollisionCircles(game->player.position, PLAYER_COLLISION_RADIUS,
                                      e->position, r))
            {
                /* Kill player */
                game->player.alive = false;
                game->lives--;
                if (game->lives > 0) {
                    game->player.respawn_timer = RESPAWN_DELAY;
                }
                EmitExplosion(game->particles, &game->particle_count,
                              game->player.position, COLOR_PLAYER);
                EffectsTriggerShake(&game->effects, SHAKE_INTENSITY);
                EffectsTriggerFlash(&game->effects);

                /* Enemy also destroyed — no points for kamikaze */
                EmitExplosion(game->particles, &game->particle_count,
                              e->position, EnemyColor(e->type));

                if (e->is_escort && e->escort_of >= 0 && e->escort_of < MAX_ENEMIES) {
                    Enemy *flagship = &game->enemies[e->escort_of];
                    if (flagship->alive)
                        flagship->escorts_killed++;
                }

                e->alive = false;
                e->state = ENEMY_INACTIVE;
                break;
            }
        }
    }
}
