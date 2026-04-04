#include "formation.h"
#include "game.h"
#include "path.h"
#include <math.h>
#include <string.h>

void FormationInit(Formation *f)
{
    memset(f, 0, sizeof(*f));
}

Vector2 FormationSlotPosition(int col, int row, float sway_offset)
{
    float x = FORMATION_X_OFFSET + col * FORMATION_COL_SPACING + sway_offset;
    float y = FORMATION_TOP_Y + row * FORMATION_ROW_SPACING;
    return (Vector2){ x, y };
}

/* Count enemies currently diving (not counting escorts) */
static int CountActiveDivers(Game *game)
{
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (e->alive && e->state == ENEMY_DIVING && !e->is_escort)
            count++;
    }
    return count;
}

/* Select an enemy to dive — priority: flagships first, then lower rows weighted higher */
static int SelectDiver(Game *game)
{
    /* Try flagships first */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (e->alive && e->state == ENEMY_IN_FORMATION && e->type == ENEMY_FLAGSHIP)
            return i;
    }

    /* Build weighted pool: lower rows (higher row number) get higher weight */
    int candidates[MAX_ENEMIES];
    int weights[MAX_ENEMIES];
    int num = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (e->alive && e->state == ENEMY_IN_FORMATION) {
            candidates[num] = i;
            weights[num] = e->slot_row + 1;  /* row 0=1, row 5=6 */
            num++;
        }
    }

    if (num == 0) return -1;

    int total_weight = 0;
    for (int i = 0; i < num; i++) total_weight += weights[i];

    int roll = GetRandomValue(1, total_weight);
    int accum = 0;
    for (int i = 0; i < num; i++) {
        accum += weights[i];
        if (roll <= accum) return candidates[i];
    }

    return candidates[num - 1];
}

/* Find escort reds adjacent to a flagship */
static void AssignEscorts(Game *game, int flagship_idx)
{
    Enemy *flagship = &game->enemies[flagship_idx];
    flagship->escort_count = 0;
    flagship->escorts_killed = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (flagship->escort_count >= 2) break;
        Enemy *e = &game->enemies[i];
        if (e->alive && e->type == ENEMY_RED && e->state == ENEMY_IN_FORMATION) {
            int col_diff = e->slot_col - flagship->slot_col;
            if (col_diff == -1 || col_diff == 0 || col_diff == 1) {
                flagship->escort_indices[flagship->escort_count++] = i;
                e->is_escort = true;
                e->escort_of = flagship_idx;

                /* Start escort dive */
                e->state = ENEMY_DIVING;
                e->dive_time = 0;
                e->fire_timer = RandFloat(0.5f, 1.5f);
                DivePathType ptype = (e->slot_col <= flagship->slot_col)
                    ? PATH_SWOOP_LEFT : PATH_SWOOP_RIGHT;
                GenerateDivePath(&e->dive_path, e->position, ptype,
                    Lerpf(DIVE_SPEED_MIN, DIVE_SPEED_MAX, game->difficulty));
            }
        }
    }
}

void FormationUpdate(struct Game *game, float dt)
{
    Formation *f = &game->formation;

    /* Sway */
    float sway_speed = Lerpf(SWAY_SPEED_MIN, SWAY_SPEED_MAX, game->difficulty);
    float sway_amp = Lerpf(SWAY_AMP_MIN, SWAY_AMP_MAX, game->difficulty);
    f->sway_timer += dt * sway_speed;
    f->sway_offset = sinf(f->sway_timer) * sway_amp;

    /* Check if all enemies have entered */
    if (!f->all_entered) {
        bool all_in = true;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy *e = &game->enemies[i];
            if (e->alive && e->state == ENEMY_ENTERING) {
                all_in = false;
                break;
            }
        }
        f->all_entered = all_in;
    }

    /* Update enemy states */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (!e->alive || e->state == ENEMY_INACTIVE) continue;

        switch (e->state) {
        case ENEMY_ENTERING: {
            e->enter_timer += dt;
            if (e->enter_timer < e->enter_delay) {
                e->position = (Vector2){ -100, -100 }; /* offscreen */
                break;
            }
            float progress = (e->enter_timer - e->enter_delay) / ENTRY_DURATION;
            if (progress >= 1.0f) {
                progress = 1.0f;
                e->state = ENEMY_IN_FORMATION;
            }
            /* Ease-out */
            float t = 1.0f - (1.0f - progress) * (1.0f - progress);
            Vector2 slot = FormationSlotPosition(e->slot_col, e->slot_row, 0);
            BezierSegment entry_seg;
            float mid_x = VIRTUAL_WIDTH / 2.0f;
            entry_seg.p0 = e->enter_start;
            entry_seg.c1 = (Vector2){ mid_x, 80.0f };
            entry_seg.c2 = (Vector2){ slot.x, slot.y - 40.0f };
            entry_seg.p3 = slot;
            e->position = EvalBezier(&entry_seg, t);
            e->rotation = BezierRotation(&entry_seg, t);
        } break;

        case ENEMY_IN_FORMATION: {
            e->position = FormationSlotPosition(e->slot_col, e->slot_row, f->sway_offset);
            e->rotation = 0;
        } break;

        case ENEMY_DIVING: {
            e->dive_time += dt;
            float t_global = e->dive_time / e->dive_path.total_time;
            if (t_global >= 1.0f) {
                /* Path complete — wrap to top and return */
                e->position.x = FormationSlotPosition(e->slot_col, e->slot_row, 0).x;
                e->position.y = -30.0f;
                e->state = ENEMY_RETURNING;
                e->return_t = 0;
                Vector2 slot = FormationSlotPosition(e->slot_col, e->slot_row, 0);
                GenerateReturnPath(&e->return_seg, e->position, slot);
                e->is_escort = false;
                e->escort_of = -1;
                break;
            }
            e->position = EvalDivePath(&e->dive_path, t_global);
            e->rotation = PathGetRotation(&e->dive_path, t_global);

            /* Fire at player */
            float shot_freq = Lerpf(SHOT_FREQ_MIN, SHOT_FREQ_MAX, game->difficulty);
            e->fire_timer -= dt;
            if (e->fire_timer <= 0 && game->player.alive) {
                FireEnemyBullet(&game->bullets, e->position, game->player.position);
                e->fire_timer = 1.0f / shot_freq + RandFloat(-0.2f, 0.2f);
            }
        } break;

        case ENEMY_RETURNING: {
            e->return_t += dt / RETURN_DURATION;
            if (e->return_t >= 1.0f) {
                e->return_t = 1.0f;
                e->state = ENEMY_IN_FORMATION;
            }
            float t = 1.0f - (1.0f - e->return_t) * (1.0f - e->return_t);
            e->position = EvalBezier(&e->return_seg, t);
            e->rotation = BezierRotation(&e->return_seg, t);
        } break;

        default: break;
        }
    }

    /* Dive initiation */
    if (f->all_entered && !game->stage_clearing) {
        float cooldown = Lerpf(DIVE_COOLDOWN_EASY, DIVE_COOLDOWN_HARD, game->difficulty);
        f->dive_timer += dt;
        int max_divers = (int)Lerpf((float)MAX_DIVERS_MIN, (float)MAX_DIVERS_MAX, game->difficulty);
        if (f->dive_timer >= cooldown) {
            f->dive_timer = 0;
            int current_divers = CountActiveDivers(game);
            if (current_divers < max_divers) {
                int idx = SelectDiver(game);
                if (idx >= 0) {
                    Enemy *e = &game->enemies[idx];
                    e->state = ENEMY_DIVING;
                    e->dive_time = 0;
                    e->fire_timer = RandFloat(0.3f, 1.0f);
                    e->is_escort = false;
                    e->escort_of = -1;
                    e->escort_count = 0;
                    e->escorts_killed = 0;

                    float speed_mult = Lerpf(DIVE_SPEED_MIN, DIVE_SPEED_MAX, game->difficulty);
                    DivePathType ptype;
                    if (e->type == ENEMY_FLAGSHIP) {
                        ptype = PATH_FLAGSHIP_CENTER;
                        AssignEscorts(game, idx);
                    } else {
                        int r = GetRandomValue(0, 2);
                        if (r == 0) ptype = PATH_SWOOP_LEFT;
                        else if (r == 1) ptype = PATH_SWOOP_RIGHT;
                        else ptype = PATH_LOOP;
                    }
                    GenerateDivePath(&e->dive_path, e->position, ptype, speed_mult);
                }
            }
        }
    }
}

void FormationInitStage(struct Game *game)
{
    memset(game->enemies, 0, sizeof(game->enemies));

    int idx = 0;

    /* Row 0: 2 Flagships (cols 4-5) */
    for (int c = 4; c <= 5; c++) {
        Enemy *e = &game->enemies[idx++];
        e->type = ENEMY_FLAGSHIP;
        e->state = ENEMY_ENTERING;
        e->alive = true;
        e->slot_row = 0;
        e->slot_col = c;
        e->escort_of = -1;
    }

    /* Row 1: 6 Red (cols 2-7) */
    for (int c = 2; c <= 7; c++) {
        Enemy *e = &game->enemies[idx++];
        e->type = ENEMY_RED;
        e->state = ENEMY_ENTERING;
        e->alive = true;
        e->slot_row = 1;
        e->slot_col = c;
        e->escort_of = -1;
    }

    /* Row 2: 8 Purple (cols 1-8) */
    for (int c = 1; c <= 8; c++) {
        Enemy *e = &game->enemies[idx++];
        e->type = ENEMY_PURPLE;
        e->state = ENEMY_ENTERING;
        e->alive = true;
        e->slot_row = 2;
        e->slot_col = c;
        e->escort_of = -1;
    }

    /* Rows 3-5: 10 Blue each (cols 0-9) */
    for (int r = 3; r <= 5; r++) {
        for (int c = 0; c <= 9; c++) {
            Enemy *e = &game->enemies[idx++];
            e->type = ENEMY_BLUE;
            e->state = ENEMY_ENTERING;
            e->alive = true;
            e->slot_row = r;
            e->slot_col = c;
            e->escort_of = -1;
        }
    }

    /* Set entry animation parameters */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (!e->alive) continue;

        float wave_delay;
        switch (e->slot_row) {
            case 5: wave_delay = 0.0f;  break;
            case 4: wave_delay = 0.7f;  break;
            case 3: wave_delay = 1.4f;  break;
            case 2: wave_delay = 2.1f;  break;
            case 1: wave_delay = 2.8f;  break;
            case 0: wave_delay = 3.5f;  break;
            default: wave_delay = 0.0f;
        }
        float in_wave = e->slot_col * 0.06f;
        e->enter_delay = wave_delay + in_wave;
        e->enter_timer = 0;

        /* Entry start: alternate sides based on column */
        if (e->slot_col < 5) {
            e->enter_start = (Vector2){ -20.0f, -20.0f };
        } else {
            e->enter_start = (Vector2){ VIRTUAL_WIDTH + 20.0f, -20.0f };
        }
    }

    /* Reset formation */
    game->formation.sway_timer = 0;
    game->formation.sway_offset = 0;
    game->formation.dive_timer = 0;
    game->formation.active_divers = 0;
    game->formation.all_entered = false;
}
