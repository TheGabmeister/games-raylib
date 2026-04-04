#include "raylib.h"

int main(void)
{
    InitWindow(800, 450, "Defender");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello, World!", 310, 200, 30, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
