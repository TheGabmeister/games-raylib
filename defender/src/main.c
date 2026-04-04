#include "game.h"
#include "input.h"

int main(void)
{
    Game game;
    float accumulator = 0.0f;

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Defender");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    GameInit(&game);

    while (!WindowShouldClose() && !game.requestQuit)
    {
        float frameTime = ClampFloat(GetFrameTime(), 0.0f, 0.25f);

        UpdateInput(&game.input);

        accumulator += frameTime;
        while (accumulator >= FIXED_TIMESTEP)
        {
            GameUpdate(&game, FIXED_TIMESTEP);
            ClearInputPressed(&game.input);
            accumulator -= FIXED_TIMESTEP;
        }

        BeginDrawing();
        GameDraw(&game);
        EndDrawing();
    }

    GameShutdown(&game);
    CloseWindow();
    return 0;
}
