#include "ui.h"
#include "game.h"
#include "draw_utils.h"
#include <stdio.h>

void DrawHUD(struct Game *game)
{
    /* Score — top left */
    char score_buf[32];
    snprintf(score_buf, sizeof(score_buf), "%06d", game->score);
    DrawText(score_buf, 10, 8, 20, COLOR_HUD);

    /* "STAGE" — top right */
    char stage_buf[32];
    snprintf(stage_buf, sizeof(stage_buf), "STAGE %d", game->stage);
    int sw = MeasureText(stage_buf, 16);
    DrawText(stage_buf, VIRTUAL_WIDTH - sw - 10, 10, 16, COLOR_HUD);

    /* Lives — bottom left as small ship icons */
    for (int i = 0; i < game->lives - 1; i++) {
        float lx = 16.0f + i * 22.0f;
        float ly = VIRTUAL_HEIGHT - 18.0f;
        /* Small triangle as life icon */
        Vector2 v1 = { lx, ly - 6 };
        Vector2 v2 = { lx - 5, ly + 4 };
        Vector2 v3 = { lx + 5, ly + 4 };
        DrawTriangle(v1, v3, v2, COLOR_PLAYER);
    }

    /* Stage flags — bottom right */
    int flags = game->stage;
    if (flags > 48) flags = 48;
    for (int i = 0; i < flags && i < 20; i++) {
        float fx = VIRTUAL_WIDTH - 14.0f - i * 12.0f;
        float fy = VIRTUAL_HEIGHT - 18.0f;
        /* Small flag rectangle */
        DrawRectangle((int)fx - 4, (int)fy - 4, 8, 8, COLOR_RED_ENEMY);
        DrawRectangleLines((int)fx - 4, (int)fy - 4, 8, 8, (Color){255, 100, 100, 255});
    }
}

void DrawTitleScreen(void)
{
    /* Glow behind title */
    BeginBlendMode(BLEND_ADDITIVE);
    Color glow = COLOR_PLAYER;
    glow.a = 20;
    DrawCircleGradient(VIRTUAL_WIDTH / 2, 180, 200, glow, (Color){0, 0, 0, 0});
    EndBlendMode();

    /* Title */
    const char *title = "GALAXIAN";
    int tw = MeasureText(title, 48);
    /* Edge (bright) */
    DrawText(title, VIRTUAL_WIDTH / 2 - tw / 2, 160, 48, COLOR_PLAYER);
    /* Shadow/fill beneath */
    Color fill = COLOR_PLAYER;
    fill.a = 80;
    DrawText(title, VIRTUAL_WIDTH / 2 - tw / 2 + 1, 161, 48, fill);

    /* Subtitle */
    const char *sub = "MODERNIZED";
    int subw = MeasureText(sub, 16);
    DrawText(sub, VIRTUAL_WIDTH / 2 - subw / 2, 218, 16, COLOR_HUD);

    /* Instructions */
    float blink = sinf((float)GetTime() * 3.0f);
    if (blink > 0) {
        const char *start = "PRESS SPACE TO START";
        int startw = MeasureText(start, 18);
        DrawText(start, VIRTUAL_WIDTH / 2 - startw / 2, 380, 18, COLOR_HUD);
    }

    /* Controls */
    Color dim = COLOR_HUD;
    dim.a = 150;
    const char *ctrl1 = "ARROW KEYS / A,D  -  MOVE";
    int cw1 = MeasureText(ctrl1, 14);
    DrawText(ctrl1, VIRTUAL_WIDTH / 2 - cw1 / 2, 460, 14, dim);

    const char *ctrl2 = "SPACE  -  FIRE";
    int cw2 = MeasureText(ctrl2, 14);
    DrawText(ctrl2, VIRTUAL_WIDTH / 2 - cw2 / 2, 480, 14, dim);

    /* Decorative enemies */
    DrawBlueEnemy((Vector2){120, 320}, 0, COLOR_BLUE_ENEMY);
    DrawText("30", 145, 314, 14, COLOR_HUD);

    DrawPurpleEnemy((Vector2){200, 320}, 0, COLOR_PURPLE_ENEMY);
    DrawText("40", 225, 314, 14, COLOR_HUD);

    DrawRedEnemy((Vector2){280, 320}, 0, COLOR_RED_ENEMY);
    DrawText("50", 305, 314, 14, COLOR_HUD);

    DrawFlagshipEnemy((Vector2){360, 320}, 0, COLOR_FLAGSHIP);
    DrawText("60", 385, 314, 14, COLOR_HUD);
}

void DrawGameOverScreen(int score)
{
    /* Dark overlay */
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, (Color){0, 0, 0, 180});

    /* Game Over text */
    const char *go = "GAME OVER";
    int gow = MeasureText(go, 48);
    DrawText(go, VIRTUAL_WIDTH / 2 - gow / 2, 220, 48, COLOR_RED_ENEMY);

    /* Final score */
    char buf[64];
    snprintf(buf, sizeof(buf), "SCORE: %d", score);
    int bw = MeasureText(buf, 24);
    DrawText(buf, VIRTUAL_WIDTH / 2 - bw / 2, 300, 24, COLOR_HUD);

    /* Restart prompt */
    float blink = sinf((float)GetTime() * 3.0f);
    if (blink > 0) {
        const char *restart = "PRESS SPACE TO CONTINUE";
        int rw = MeasureText(restart, 16);
        DrawText(restart, VIRTUAL_WIDTH / 2 - rw / 2, 380, 16, COLOR_HUD);
    }
}
