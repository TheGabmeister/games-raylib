#include "raylib.h"

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900
#define STARTING_LIVES 3

static int score = 0;
static int hiScore = 0;
static int lives = STARTING_LIVES;

Music music = { 0 };
Sound sfxLaser = { 0 };

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
    
}

void UpdateGame(void)
{
    
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText(TextFormat("Score: %d", score), 20, 20, 32, WHITE);
    DrawText(TextFormat("Lives: %d", lives), SCREEN_WIDTH - 160, 20, 32, WHITE);

    EndDrawing();
}

void UnloadGame(void)
{
    UnloadMusicStream(music);
    UnloadSound(sfxLaser);
}