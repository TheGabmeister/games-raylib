#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "raylib.h"

#include <stdbool.h>
#include <math.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define WORLD_WIDTH 6400.0f
#define WORLD_HEIGHT 720.0f
#define TERRAIN_SAMPLE_COUNT 129

#define HUMAN_SPAWN_COUNT 10
#define MAX_HUMANS 16
#define MAX_ENEMIES 64
#define MAX_PROJECTILES 128
#define MAX_PARTICLES 512
#define MAX_CALLOUTS 32

#define STAR_LAYER_FAR_COUNT 64
#define STAR_LAYER_MID_COUNT 48
#define STAR_LAYER_NEAR_COUNT 32

#define FIXED_TIMESTEP (1.0f/60.0f)
#define TAU 6.28318530718f

static inline float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static inline float LerpFloat(float a, float b, float t)
{
    return a + (b - a)*t;
}

static inline float SignFloat(float value)
{
    if (value > 0.0f) return 1.0f;
    if (value < 0.0f) return -1.0f;
    return 0.0f;
}

static inline Vector2 Vec2(float x, float y)
{
    Vector2 value = { x, y };
    return value;
}

static inline Vector2 Vec2Add(Vector2 a, Vector2 b)
{
    return Vec2(a.x + b.x, a.y + b.y);
}

static inline Vector2 Vec2Sub(Vector2 a, Vector2 b)
{
    return Vec2(a.x - b.x, a.y - b.y);
}

static inline Vector2 Vec2Scale(Vector2 value, float scale)
{
    return Vec2(value.x*scale, value.y*scale);
}

static inline float Vec2LengthSquared(Vector2 value)
{
    return value.x*value.x + value.y*value.y;
}

static inline float Vec2Length(Vector2 value)
{
    return sqrtf(Vec2LengthSquared(value));
}

static inline Vector2 Vec2NormalizeSafe(Vector2 value)
{
    float length = Vec2Length(value);
    if (length <= 0.0001f) return Vec2(0.0f, 0.0f);
    return Vec2Scale(value, 1.0f/length);
}

typedef enum GameMode
{
    GAME_MODE_TITLE = 0,
    GAME_MODE_PLAYING,
    GAME_MODE_PAUSED,
    GAME_MODE_WAVE_CLEAR,
    GAME_MODE_GAME_OVER
} GameMode;

typedef enum InputAction
{
    INPUT_ACTION_MOVE_LEFT = 0,
    INPUT_ACTION_MOVE_RIGHT,
    INPUT_ACTION_THRUST,
    INPUT_ACTION_REVERSE,
    INPUT_ACTION_FIRE,
    INPUT_ACTION_SMART_BOMB,
    INPUT_ACTION_HYPERSPACE,
    INPUT_ACTION_PAUSE,
    INPUT_ACTION_CONFIRM,
    INPUT_ACTION_COUNT
} InputAction;

typedef enum HumanState
{
    HUMAN_STATE_INACTIVE = 0,
    HUMAN_STATE_GROUNDED,
    HUMAN_STATE_ABDUCTED,
    HUMAN_STATE_FALLING,
    HUMAN_STATE_CARRIED,
    HUMAN_STATE_DEAD
} HumanState;

typedef enum EnemyType
{
    ENEMY_LANDER = 0,
    ENEMY_MUTANT,
    ENEMY_BOMBER,
    ENEMY_BAITER,
    ENEMY_SWARMER,
    ENEMY_POD
} EnemyType;

typedef struct InputState
{
    bool down[INPUT_ACTION_COUNT];
    bool pressed[INPUT_ACTION_COUNT];
    bool previousDown[INPUT_ACTION_COUNT];
    float moveAxis;
    bool usingGamepad;
} InputState;

typedef struct WorldState
{
    float width;
    float height;
    float sampleSpacing;
    float terrainHeights[TERRAIN_SAMPLE_COUNT];
    float humanSpawnXs[HUMAN_SPAWN_COUNT];
} WorldState;

typedef struct WaveState
{
    int index;
    float bannerTimer;
    float clearTimer;
    int survivorBonusAwarded;
} WaveState;

typedef struct SpawnDirector
{
    float baiterTimer;
    float baiterInterval;
} SpawnDirector;

typedef struct Player
{
    Vector2 position;
    Vector2 velocity;
    float radius;
    float fireCooldown;
    float invulnerabilityTimer;
    float respawnTimer;
    float hyperspaceCooldown;
    float hitFlashTimer;
    bool alive;
    int facing;
    int smartBombs;
    int lives;
    int carriedHuman;
    int hyperspaceUsesThisLife;
} Player;

typedef struct Human
{
    bool active;
    bool rescueBonusEligible;
    HumanState state;
    Vector2 position;
    Vector2 velocity;
    int carrierEnemy;
} Human;

typedef struct Enemy
{
    bool active;
    EnemyType type;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float fireTimer;
    float stateTimer;
    float speed;
    int hp;
    int targetHuman;
    bool carryingHuman;
} Enemy;

typedef struct Projectile
{
    bool active;
    bool fromPlayer;
    Vector2 position;
    Vector2 velocity;
    float life;
    float radius;
    EnemyType ownerType;
} Projectile;

typedef struct Particle
{
    bool active;
    Vector2 position;
    Vector2 velocity;
    float life;
    float maxLife;
    float size;
    float sizeVelocity;
    Color color;
} Particle;

typedef struct FloatingText
{
    bool active;
    char text[24];
    Vector2 position;
    Vector2 velocity;
    float life;
    float maxLife;
    Color color;
} FloatingText;

typedef struct CameraState
{
    Vector2 center;
    Vector2 shakeOffset;
    float shakeTimer;
    float shakeStrength;
} CameraState;

typedef struct RenderPalette
{
    Color background;
    Color backdropTop;
    Color backdropBottom;
    Color terrainFill;
    Color terrainGlow;
    Color playerPrimary;
    Color playerAccent;
    Color human;
    Color lander;
    Color mutant;
    Color bomber;
    Color baiter;
    Color swarmer;
    Color pod;
    Color projectilePlayer;
    Color projectileEnemy;
    Color mine;
    Color warning;
    Color hud;
} RenderPalette;

typedef struct Game
{
    GameMode mode;
    InputState input;
    WorldState world;
    WaveState wave;
    SpawnDirector spawnDirector;
    Player player;
    Human humans[MAX_HUMANS];
    Enemy enemies[MAX_ENEMIES];
    Projectile projectiles[MAX_PROJECTILES];
    Particle particles[MAX_PARTICLES];
    FloatingText callouts[MAX_CALLOUTS];
    CameraState camera;
    RenderPalette palette;
    Vector2 starsFar[STAR_LAYER_FAR_COUNT];
    Vector2 starsMid[STAR_LAYER_MID_COUNT];
    Vector2 starsNear[STAR_LAYER_NEAR_COUNT];
    int score;
    int nextExtraLifeScore;
    float titleTimer;
    bool requestQuit;
} Game;

#endif
