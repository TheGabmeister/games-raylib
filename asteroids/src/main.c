#include "raylib.h"
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900
#define SHIP_SIZE 20
#define SHIP_TURN_SPEED 5.0f
#define SHIP_ACCELERATION 0.2f
#define SHIP_FRICTION 0.99f
#define BULLET_SPEED 8.0f
#define BULLET_LIFETIME 60
#define MAX_BULLETS 10
#define MAX_ASTEROIDS 16
#define ASTEROID_MIN_SIZE 20
#define ASTEROID_MAX_SIZE 60
#define ASTEROID_MIN_SPEED 1.0f
#define ASTEROID_MAX_SPEED 3.0f

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float angle;
    bool active;
} Bullet;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float angle;
    float size;
    int sides;
    bool active;
} Asteroid;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float angle;
    float radius;
} Ship;

static Ship ship;
static Bullet bullets[MAX_BULLETS];
static int bulletTimer[MAX_BULLETS];
static Asteroid asteroids[MAX_ASTEROIDS];

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void FireBullet(void);
static void SpawnAsteroid(Vector2 pos, float size);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "asteroids");
    InitGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateGame();
        DrawGame();
    }

    UnloadGame();
    CloseWindow();
    return 0;
}

void InitGame(void)
{
    ship.pos = (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    ship.vel = (Vector2){0, 0};
    ship.angle = 0;
    ship.radius = SHIP_SIZE/2;

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;

    // Spawn initial asteroids
    for (int i = 0; i < 5; i++) {
        Vector2 pos = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)};
        SpawnAsteroid(pos, ASTEROID_MAX_SIZE);
    }
}

void UpdateGame(void)
{
    // Ship controls
    if (IsKeyDown(KEY_LEFT)) ship.angle -= SHIP_TURN_SPEED;
    if (IsKeyDown(KEY_RIGHT)) ship.angle += SHIP_TURN_SPEED;
    if (IsKeyDown(KEY_UP)) {
        ship.vel.x += cosf(DEG2RAD * ship.angle) * SHIP_ACCELERATION;
        ship.vel.y += sinf(DEG2RAD * ship.angle) * SHIP_ACCELERATION;
    }
    ship.vel.x *= SHIP_FRICTION;
    ship.vel.y *= SHIP_FRICTION;
    ship.pos.x += ship.vel.x;
    ship.pos.y += ship.vel.y;

    // Screen wrap
    if (ship.pos.x < 0) ship.pos.x += SCREEN_WIDTH;
    if (ship.pos.x > SCREEN_WIDTH) ship.pos.x -= SCREEN_WIDTH;
    if (ship.pos.y < 0) ship.pos.y += SCREEN_HEIGHT;
    if (ship.pos.y > SCREEN_HEIGHT) ship.pos.y -= SCREEN_HEIGHT;

    // Fire bullets
    static bool canShoot = true;
    if (IsKeyDown(KEY_SPACE)) {
        if (canShoot) {
            FireBullet();
            canShoot = false;
        }
    } else {
        canShoot = true;
    }

    // Update bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].pos.x += bullets[i].vel.x;
            bullets[i].pos.y += bullets[i].vel.y;
            bulletTimer[i]++;
            // Screen wrap
            if (bullets[i].pos.x < 0) bullets[i].pos.x += SCREEN_WIDTH;
            if (bullets[i].pos.x > SCREEN_WIDTH) bullets[i].pos.x -= SCREEN_WIDTH;
            if (bullets[i].pos.y < 0) bullets[i].pos.y += SCREEN_HEIGHT;
            if (bullets[i].pos.y > SCREEN_HEIGHT) bullets[i].pos.y -= SCREEN_HEIGHT;
            if (bulletTimer[i] > BULLET_LIFETIME) bullets[i].active = false;
        }
    }

    // Update asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            asteroids[i].pos.x += asteroids[i].vel.x;
            asteroids[i].pos.y += asteroids[i].vel.y;
            // Screen wrap
            if (asteroids[i].pos.x < 0) asteroids[i].pos.x += SCREEN_WIDTH;
            if (asteroids[i].pos.x > SCREEN_WIDTH) asteroids[i].pos.x -= SCREEN_WIDTH;
            if (asteroids[i].pos.y < 0) asteroids[i].pos.y += SCREEN_HEIGHT;
            if (asteroids[i].pos.y > SCREEN_HEIGHT) asteroids[i].pos.y -= SCREEN_HEIGHT;
        }
    }

    // Bullet-asteroid collision
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        for (int j = 0; j < MAX_ASTEROIDS; j++) {
            if (!asteroids[j].active) continue;
            float dx = bullets[i].pos.x - asteroids[j].pos.x;
            float dy = bullets[i].pos.y - asteroids[j].pos.y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < asteroids[j].size) {
                bullets[i].active = false;
                asteroids[j].active = false;
                // Split asteroid
                if (asteroids[j].size > ASTEROID_MIN_SIZE) {
                    for (int s = 0; s < 2; s++) {
                        SpawnAsteroid(asteroids[j].pos, asteroids[j].size/2);
                    }
                }
                break;
            }
        }
    }

    // Ship-asteroid collision
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) continue;
        float dx = ship.pos.x - asteroids[i].pos.x;
        float dy = ship.pos.y - asteroids[i].pos.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist < asteroids[i].size + ship.radius) {
            // Reset game on collision
            InitGame();
            break;
        }
    }
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw ship
    Vector2 nose = {
        ship.pos.x + cosf(DEG2RAD * ship.angle) * SHIP_SIZE,
        ship.pos.y + sinf(DEG2RAD * ship.angle) * SHIP_SIZE
    };
    Vector2 left = {
        ship.pos.x + cosf(DEG2RAD * (ship.angle + 140)) * SHIP_SIZE * 0.6f,
        ship.pos.y + sinf(DEG2RAD * (ship.angle + 140)) * SHIP_SIZE * 0.6f
    };
    Vector2 right = {
        ship.pos.x + cosf(DEG2RAD * (ship.angle - 140)) * SHIP_SIZE * 0.6f,
        ship.pos.y + sinf(DEG2RAD * (ship.angle - 140)) * SHIP_SIZE * 0.6f
    };
    DrawTriangle(nose, left, right, WHITE);

    // Draw bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            DrawCircleV(bullets[i].pos, 2, YELLOW);
        }
    }

    // Draw asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            Vector2 points[16];
            float angleStep = 360.0f / asteroids[i].sides;
            for (int v = 0; v < asteroids[i].sides; v++) {
                float ang = DEG2RAD * (asteroids[i].angle + v * angleStep);
                float rad = asteroids[i].size * (0.75f + 0.25f * (float)GetRandomValue(0, 100)/100.0f);
                points[v].x = asteroids[i].pos.x + cosf(ang) * rad;
                points[v].y = asteroids[i].pos.y + sinf(ang) * rad;
            }
            for (int v = 0; v < asteroids[i].sides; v++) {
                DrawLineV(points[v], points[(v+1)%asteroids[i].sides], GRAY);
            }
        }
    }

    EndDrawing();
}

void UnloadGame(void)
{
    // No dynamic resources to unload
}

void FireBullet(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].pos = ship.pos;
            bullets[i].vel.x = cosf(DEG2RAD * ship.angle) * BULLET_SPEED;
            bullets[i].vel.y = sinf(DEG2RAD * ship.angle) * BULLET_SPEED;
            bulletTimer[i] = 0;
            break;
        }
    }
}

void SpawnAsteroid(Vector2 pos, float size)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            asteroids[i].active = true;
            asteroids[i].pos = pos;
            float angle = DEG2RAD * GetRandomValue(0, 359);
            float speed = ASTEROID_MIN_SPEED + (float)GetRandomValue(0, 100)/100.0f * (ASTEROID_MAX_SPEED - ASTEROID_MIN_SPEED);
            asteroids[i].vel.x = cosf(angle) * speed;
            asteroids[i].vel.y = sinf(angle) * speed;
            asteroids[i].angle = GetRandomValue(0, 359);
            asteroids[i].size = size;
            asteroids[i].sides = 8 + GetRandomValue(0, 3); // 8-11 sides
            break;
        }
    }
}