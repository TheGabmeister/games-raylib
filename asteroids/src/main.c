#include "raylib.h"
#include <math.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);

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

}

void UpdateGame(void)
{

    
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(BLACK);


    EndDrawing();
}

void UnloadGame(void)
{
    // No dynamic resources to unload
}