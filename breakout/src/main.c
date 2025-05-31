#include "raylib.h"
#include <math.h>

static void InitGame(void);         
static void UpdateGame(void);       
static void DrawGame(void);         
static void UnloadGame(void);       
static void UpdateDrawFrame(void);  

int main(void)
{
    InitWindow(720, 900, "classic game: breakout");
    InitGame();
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateGame();
        DrawGame();
    }

    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    CloseWindow();        // Close window and OpenGL context
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
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}
