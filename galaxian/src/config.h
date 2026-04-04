#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"
#include <math.h>
#include <stdbool.h>

/* === Window & Virtual Resolution === */
#define VIRTUAL_WIDTH   480
#define VIRTUAL_HEIGHT  640
#define WINDOW_WIDTH    720
#define WINDOW_HEIGHT   960

/* === Player === */
#define PLAYER_SPEED            250.0f
#define PLAYER_START_X          (VIRTUAL_WIDTH / 2.0f)
#define PLAYER_START_Y          (VIRTUAL_HEIGHT - 40.0f)
#define PLAYER_COLLISION_RADIUS 10.0f
#define PLAYER_START_LIVES      3
#define BONUS_LIFE_SCORE        7000
#define RESPAWN_DELAY           1.5f
#define INVINCIBLE_DURATION     2.0f
#define BLINK_RATE              10.0f

/* === Bullets === */
#define PLAYER_BULLET_SPEED     1600.0f
#define PLAYER_BULLET_RADIUS    4.0f
#define MAX_ENEMY_BULLETS       30
#define ENEMY_BULLET_SPEED      250.0f
#define ENEMY_BULLET_RADIUS     3.0f

/* === Enemies & Formation === */
#define MAX_ENEMIES             46
#define FORMATION_COLS          10
#define FORMATION_ROWS          6
#define FORMATION_COL_SPACING   40.0f
#define FORMATION_ROW_SPACING   34.0f
#define FORMATION_TOP_Y         80.0f
#define FORMATION_X_OFFSET      ((VIRTUAL_WIDTH - (FORMATION_COLS - 1) * FORMATION_COL_SPACING) / 2.0f)

#define COLLISION_BLUE          10.0f
#define COLLISION_PURPLE        12.0f
#define COLLISION_RED           12.0f
#define COLLISION_FLAGSHIP      16.0f

/* === Particles === */
#define MAX_PARTICLES           512

/* === Starfield === */
#define STAR_COUNT              200

/* === Effects === */
#define SHAKE_DURATION          0.3f
#define SHAKE_INTENSITY         6.0f
#define FLASH_DURATION          0.5f

/* === Entry Animation === */
#define ENTRY_DURATION          1.2f

/* === Dive Paths === */
#define RETURN_DURATION         1.5f

/* === Difficulty Scaling (Stage 1 to 48) === */
#define MAX_DIFFICULTY_STAGE    48

#define SWAY_SPEED_MIN          1.0f
#define SWAY_SPEED_MAX          2.5f
#define SWAY_AMP_MIN            30.0f
#define SWAY_AMP_MAX            50.0f
#define DIVE_COOLDOWN_EASY      3.0f
#define DIVE_COOLDOWN_HARD      1.0f
#define MAX_DIVERS_MIN          2
#define MAX_DIVERS_MAX          4
#define SHOT_FREQ_MIN           0.5f
#define SHOT_FREQ_MAX           2.5f
#define DIVE_SPEED_MIN          1.0f
#define DIVE_SPEED_MAX          1.8f

/* === Neon Colors === */
#define COLOR_PLAYER        ((Color){0, 255, 200, 255})
#define COLOR_BLUE_ENEMY    ((Color){50, 120, 255, 255})
#define COLOR_PURPLE_ENEMY  ((Color){180, 80, 255, 255})
#define COLOR_RED_ENEMY     ((Color){255, 60, 60, 255})
#define COLOR_FLAGSHIP      ((Color){255, 220, 50, 255})
#define COLOR_PLAYER_BULLET ((Color){100, 255, 255, 255})
#define COLOR_ENEMY_BULLET  ((Color){255, 255, 100, 255})
#define COLOR_HUD           ((Color){200, 200, 255, 255})

/* === Utility === */
static inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }
static inline float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline float RandFloat(float lo, float hi) {
    return lo + (float)GetRandomValue(0, 10000) / 10000.0f * (hi - lo);
}

#endif /* CONFIG_H */
