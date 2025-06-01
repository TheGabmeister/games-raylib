#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900
#define STARTING_LIVES 3

#define PLAYER_WIDTH 48
#define PLAYER_HEIGHT 32
#define PLAYER_SPEED 6

#define BULLET_WIDTH 4
#define BULLET_HEIGHT 16
#define BULLET_SPEED 12

#define ENEMY_WIDTH 40
#define ENEMY_HEIGHT 32
#define ENEMY_SPEED 20

typedef struct {
    Rectangle rect;
    bool active;
} Bullet;

typedef struct {
    Rectangle rect;
    bool alive;
    bool isAttacking;
} Enemy;

typedef struct Timer {
    double startTime;   // Start time (seconds)
    double lifeTime;    // Lifetime (seconds)
} Timer;

static int score = 0;
static int hiScore = 0;
static int lives = STARTING_LIVES;

Music music = { 0 };
Sound sfxLaser = { 0 };

static Rectangle player;
static Bullet bullet = {0};
static Enemy enemy = {0};
static Timer timer = {0};

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void StartTimer(Timer *timer, double lifetime);
static bool TimerDone(Timer timer);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "galaxian");
    InitAudioDevice();    
    music = LoadMusicStream("resources/background_music.ogg"); 
    sfxLaser = LoadSound("resources/laser.wav");

    SetMusicVolume(music, 1.0f);
    PlayMusicStream(music);

    InitGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateGame();
        DrawGame();
        UpdateMusicStream(music);
    }

    UnloadGame();
    CloseWindow();
    return 0;
}

void InitGame(void)
{
    // Player
    player.width = PLAYER_WIDTH;
    player.height = PLAYER_HEIGHT;
    player.x = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
    player.y = SCREEN_HEIGHT - PLAYER_HEIGHT - 40;

    // Bullet
    bullet.active = false;

    // Enemy - place in the middle of the screen
    enemy.rect.width = ENEMY_WIDTH;
    enemy.rect.height = ENEMY_HEIGHT;
    enemy.rect.x = (SCREEN_WIDTH - ENEMY_WIDTH) / 2;
    enemy.rect.y = SCREEN_HEIGHT / 2 - ENEMY_HEIGHT / 2;
    enemy.alive = true;

    StartTimer(&timer, 1.0f);

    score = 0;
    lives = STARTING_LIVES;
}

void UpdateGame(void)
{
    DrawText(TextFormat("Timer done: %s", TimerDone(timer) ? "true" : "false"), 20, 60, 32, WHITE);
    if (TimerDone(timer))
    {
        // Move enemy along a curved path (arc up-right, then down-left)
        if (enemy.alive) {
            // Parameters for the curve
            static float t = 0.0f;
            const float curveDuration = 2.0f; // seconds for a full curve
            const float arcRadius = 200.0f;
            const float centerX = SCREEN_WIDTH / 2.0f;
            const float centerY = SCREEN_HEIGHT / 2.0f;

            t += GetFrameTime() / curveDuration;
            
            // Arc up-right (first half), then down-left (second half)
            float angle;
            if (t < 0.5f) {
            // Move from bottom center, arc up and right (0.75pi to 0.25pi)
            angle = Lerp(3.0f * PI / 4.0f, PI / 4.0f, t * 2.0f);
            } else {
            // Move from top right, arc down and left (0.25pi to 1.25pi)
            angle = Lerp(PI / 4.0f, 5.0f * PI / 4.0f, (t - 0.5f) * 2.0f);
            }

            TraceLog(LOG_INFO, TextFormat("t: %.2f", angle));

            enemy.rect.x = centerX + arcRadius * cosf(angle) - ENEMY_WIDTH / 2.0f;
            enemy.rect.y = centerY + arcRadius * sinf(angle) - ENEMY_HEIGHT / 2.0f;

            // If enemy reaches bottom of the screen, reset
            if (enemy.rect.y > SCREEN_HEIGHT - ENEMY_HEIGHT) {
            enemy.rect.x = (SCREEN_WIDTH - ENEMY_WIDTH) / 2;
            enemy.rect.y = SCREEN_HEIGHT / 2 - ENEMY_HEIGHT / 2;
            t = 0.0f;
            StartTimer(&timer, 1.0f);
            }
        }
    }

    // Player movement
    if (IsKeyDown(KEY_LEFT)) player.x -= PLAYER_SPEED;
    if (IsKeyDown(KEY_RIGHT)) player.x += PLAYER_SPEED;
    if (player.x < 0) player.x = 0;
    if (player.x > SCREEN_WIDTH - PLAYER_WIDTH) player.x = SCREEN_WIDTH - PLAYER_WIDTH;

    // Shooting
    if (IsKeyPressed(KEY_SPACE) && !bullet.active) {
        bullet.rect.x = player.x + PLAYER_WIDTH/2 - BULLET_WIDTH/2;
        bullet.rect.y = player.y - BULLET_HEIGHT;
        bullet.rect.width = BULLET_WIDTH;
        bullet.rect.height = BULLET_HEIGHT;
        bullet.active = true;
        PlaySound(sfxLaser);
    }

    // Bullet movement
    if (bullet.active) {
        bullet.rect.y -= BULLET_SPEED;
        if (bullet.rect.y + BULLET_HEIGHT < 0) bullet.active = false;
    }

    // Lose condition
    if (lives <= 0) {
        InitGame();
    }
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw player
    DrawRectangleRec(player, SKYBLUE);

    // Draw bullet
    if (bullet.active) DrawRectangleRec(bullet.rect, YELLOW);

    if (enemy.alive) DrawRectangleRec(enemy.rect, RED);

    DrawText(TextFormat("Score: %d", score), 20, 20, 32, WHITE);
    DrawText(TextFormat("Lives: %d", lives), SCREEN_WIDTH - 160, 20, 32, WHITE);

    EndDrawing();
}

void UnloadGame(void)
{
    UnloadMusicStream(music);
    UnloadSound(sfxLaser);
}

void StartTimer(Timer *timer, double lifetime)
{
    timer->startTime = GetTime();
    timer->lifeTime = lifetime;
}

bool TimerDone(Timer timer)
{
    return GetTime() - timer.startTime >= timer.lifeTime;
}

double GetElapsed(Timer timer)
{
    return GetTime() - timer.startTime;
}