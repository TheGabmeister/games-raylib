#include "raylib.h"
#include "config.h"
#include "game.h"

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Galaxian");
    SetTargetFPS(60);

    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    Game game;
    GameInit(&game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        GameUpdate(&game, dt);

        BeginTextureMode(target);
        ClearBackground(BLACK);
        GameDraw(&game);
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture,
            (Rectangle){ 0, 0, (float)VIRTUAL_WIDTH, -(float)VIRTUAL_HEIGHT },
            (Rectangle){ 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT },
            (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
