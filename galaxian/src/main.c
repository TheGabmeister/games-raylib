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

#define ENEMY_COLS 8
#define ENEMY_ROWS 4
#define ENEMY_WIDTH 40
#define ENEMY_HEIGHT 32
#define ENEMY_HORZ_SPACING 16
#define ENEMY_VERT_SPACING 16
#define ENEMY_SPEED 2

typedef struct {
    Rectangle rect;
    bool active;
} Bullet;

typedef struct {
    Rectangle rect;
    bool alive;
} Enemy;

static int score = 0;
static int hiScore = 0;
static int lives = STARTING_LIVES;

Music music = { 0 };
Sound sfxLaser = { 0 };

static Rectangle player;
static Bullet bullet = {0};
static Enemy enemies[ENEMY_ROWS][ENEMY_COLS];
static int enemyDir = 1;
static int enemyMoveDown = 0;

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);

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

    // Enemies
    for (int r = 0; r < ENEMY_ROWS; r++) {
        for (int c = 0; c < ENEMY_COLS; c++) {
            enemies[r][c].rect.x = 80 + c * (ENEMY_WIDTH + ENEMY_HORZ_SPACING);
            enemies[r][c].rect.y = 80 + r * (ENEMY_HEIGHT + ENEMY_VERT_SPACING);
            enemies[r][c].rect.width = ENEMY_WIDTH;
            enemies[r][c].rect.height = ENEMY_HEIGHT;
            enemies[r][c].alive = true;
        }
    }
    enemyDir = 1;
    enemyMoveDown = 0;
    score = 0;
    lives = STARTING_LIVES;
}

void UpdateGame(void)
{
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

    // Bullet-enemy collision
    if (bullet.active) {
        for (int r = 0; r < ENEMY_ROWS; r++) {
            for (int c = 0; c < ENEMY_COLS; c++) {
                if (enemies[r][c].alive && CheckCollisionRecs(bullet.rect, enemies[r][c].rect)) {
                    enemies[r][c].alive = false;
                    bullet.active = false;
                    score += 100;
                }
            }
        }
    }

    // Enemy-player collision (lose life)
    for (int r = 0; r < ENEMY_ROWS; r++) {
        for (int c = 0; c < ENEMY_COLS; c++) {
            if (enemies[r][c].alive && enemies[r][c].rect.y + ENEMY_HEIGHT >= player.y) {
                lives--;
                InitGame();
                return;
            }
        }
    }

    // Win condition: all enemies dead
    bool allDead = true;
    for (int r = 0; r < ENEMY_ROWS; r++)
        for (int c = 0; c < ENEMY_COLS; c++)
            if (enemies[r][c].alive) allDead = false;
    if (allDead) InitGame();

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

    // Draw enemies
    for (int r = 0; r < ENEMY_ROWS; r++) {
        for (int c = 0; c < ENEMY_COLS; c++) {
            if (enemies[r][c].alive)
                DrawRectangleRec(enemies[r][c].rect, (Color){255, 0, 128, 255});
        }
    }

    DrawText(TextFormat("Score: %d", score), 20, 20, 32, WHITE);
    DrawText(TextFormat("Lives: %d", lives), SCREEN_WIDTH - 160, 20, 32, WHITE);

    EndDrawing();
}

void UnloadGame(void)
{
    UnloadMusicStream(music);
    UnloadSound(sfxLaser);
}