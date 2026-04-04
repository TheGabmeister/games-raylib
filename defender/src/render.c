#include "render.h"

#include <stdio.h>

#include "world.h"

static Vector2 CameraWorldCenter(const Game *game)
{
    return Vec2(game->camera.center.x + game->camera.shakeOffset.x,
                game->camera.center.y + game->camera.shakeOffset.y);
}

static Vector2 WorldToScreen(const Game *game, Vector2 worldPosition)
{
    Vector2 camera = CameraWorldCenter(game);
    float x = WorldWrapDeltaX(&game->world, worldPosition.x - camera.x) + SCREEN_WIDTH*0.5f;
    float y = worldPosition.y - camera.y + SCREEN_HEIGHT*0.5f;
    return Vec2(x, y);
}

static bool ShouldBlinkVisible(float timer)
{
    return (((int)(timer*12.0f)) % 2) == 0;
}

static Color FadeByLife(Color color, float life, float maxLife)
{
    float t = ClampFloat(life/maxLife, 0.0f, 1.0f);
    return Fade(color, t);
}

static void DrawBackgroundGradient(const Game *game)
{
    for (int i = 0; i < 14; ++i)
    {
        float t = (float)i/13.0f;
        Color color = {
            (unsigned char)LerpFloat(game->palette.backdropTop.r, game->palette.backdropBottom.r, t),
            (unsigned char)LerpFloat(game->palette.backdropTop.g, game->palette.backdropBottom.g, t),
            (unsigned char)LerpFloat(game->palette.backdropTop.b, game->palette.backdropBottom.b, t),
            255
        };

        DrawRectangle(0, i*(SCREEN_HEIGHT/14), SCREEN_WIDTH, SCREEN_HEIGHT/14 + 2, color);
    }
}

static void DrawStarLayer(const Game *game, const Vector2 *stars, int count, float parallax, float radius, Color color)
{
    Vector2 camera = CameraWorldCenter(game);

    for (int i = 0; i < count; ++i)
    {
        float screenX = WorldWrapDeltaX(&game->world, stars[i].x - camera.x*parallax) + SCREEN_WIDTH*0.5f;
        float screenY = stars[i].y - camera.y*parallax + SCREEN_HEIGHT*0.5f;

        if (screenX >= -20.0f && screenX <= SCREEN_WIDTH + 20.0f &&
            screenY >= -20.0f && screenY <= SCREEN_HEIGHT + 20.0f)
        {
            DrawCircleV(Vec2(screenX, screenY), radius, color);
        }
    }
}

static void DrawTerrain(const Game *game)
{
    const int step = 8;
    Vector2 previous = Vec2(0.0f, 0.0f);
    bool hasPrevious = false;

    for (int x = 0; x <= SCREEN_WIDTH + step; x += step)
    {
        Vector2 camera = CameraWorldCenter(game);
        float worldX = WorldWrapX(&game->world, camera.x + ((float)x - SCREEN_WIDTH*0.5f));
        float terrainY = WorldGetTerrainHeight(&game->world, worldX) - camera.y + SCREEN_HEIGHT*0.5f;
        Vector2 current = Vec2((float)x, terrainY);

        if (hasPrevious)
        {
            DrawTriangle(Vec2(previous.x, (float)SCREEN_HEIGHT), previous, current, Fade(game->palette.terrainFill, 0.95f));
            DrawTriangle(Vec2(previous.x, (float)SCREEN_HEIGHT), current, Vec2(current.x, (float)SCREEN_HEIGHT), Fade(game->palette.terrainFill, 0.95f));
            DrawLineEx(previous, current, 7.0f, Fade(game->palette.terrainGlow, 0.18f));
            DrawLineEx(previous, current, 2.25f, game->palette.terrainGlow);
        }

        previous = current;
        hasPrevious = true;
    }
}

static void DrawHuman(const Game *game, const Human *human)
{
    Vector2 screen = WorldToScreen(game, human->position);
    Color color = game->palette.human;

    if (screen.x < -20.0f || screen.x > SCREEN_WIDTH + 20.0f) return;
    if (screen.y < -20.0f || screen.y > SCREEN_HEIGHT + 20.0f) return;

    DrawCircleV(Vec2(screen.x, screen.y - 7.0f), 3.0f, color);
    DrawLineEx(Vec2(screen.x, screen.y - 4.0f), Vec2(screen.x, screen.y + 6.0f), 2.0f, color);
    DrawLineEx(Vec2(screen.x - 4.0f, screen.y), Vec2(screen.x + 4.0f, screen.y), 2.0f, color);
    DrawLineEx(Vec2(screen.x, screen.y + 6.0f), Vec2(screen.x - 4.0f, screen.y + 11.0f), 2.0f, color);
    DrawLineEx(Vec2(screen.x, screen.y + 6.0f), Vec2(screen.x + 4.0f, screen.y + 11.0f), 2.0f, color);
}

static void DrawPlayer(const Game *game)
{
    const Player *player = &game->player;
    Vector2 screen;
    float facing;
    Vector2 nose;
    Vector2 tailTop;
    Vector2 tailBottom;
    Vector2 wingTop;
    Vector2 wingBottom;

    if (!player->alive) return;
    if (player->invulnerabilityTimer > 0.0f && !ShouldBlinkVisible(player->invulnerabilityTimer)) return;

    screen = WorldToScreen(game, player->position);
    facing = (float)player->facing;
    nose = Vec2(screen.x + 22.0f*facing, screen.y);
    tailTop = Vec2(screen.x - 18.0f*facing, screen.y - 11.0f);
    tailBottom = Vec2(screen.x - 18.0f*facing, screen.y + 11.0f);
    wingTop = Vec2(screen.x - 1.0f*facing, screen.y - 15.0f);
    wingBottom = Vec2(screen.x - 1.0f*facing, screen.y + 15.0f);

    DrawTriangle(nose, tailTop, tailBottom, Fade(game->palette.playerPrimary, 0.90f));
    DrawTriangle(nose, wingTop, wingBottom, Fade(game->palette.playerAccent, 0.75f));
    DrawLineEx(tailTop, wingTop, 2.0f, game->palette.playerAccent);
    DrawLineEx(tailBottom, wingBottom, 2.0f, game->palette.playerAccent);
    DrawCircleV(Vec2(screen.x + 3.0f*facing, screen.y), 4.0f, game->palette.playerAccent);

    if (player->hitFlashTimer > 0.0f)
    {
        DrawCircleLines((int)screen.x, (int)screen.y, 28.0f, game->palette.warning);
    }
}

static void DrawEnemy(const Game *game, const Enemy *enemy)
{
    Vector2 screen = WorldToScreen(game, enemy->position);
    Color color = game->palette.lander;

    if (screen.x < -40.0f || screen.x > SCREEN_WIDTH + 40.0f) return;
    if (screen.y < -40.0f || screen.y > SCREEN_HEIGHT + 40.0f) return;

    switch (enemy->type)
    {
        case ENEMY_LANDER:
            color = game->palette.lander;
            DrawTriangle(Vec2(screen.x - 14.0f, screen.y), Vec2(screen.x, screen.y - 12.0f), Vec2(screen.x + 14.0f, screen.y), Fade(color, 0.9f));
            DrawLineEx(Vec2(screen.x - 18.0f, screen.y + 8.0f), Vec2(screen.x - 6.0f, screen.y + 3.0f), 2.0f, color);
            DrawLineEx(Vec2(screen.x + 18.0f, screen.y + 8.0f), Vec2(screen.x + 6.0f, screen.y + 3.0f), 2.0f, color);
            DrawCircleV(Vec2(screen.x, screen.y + 2.0f), 3.0f, color);
            break;

        case ENEMY_MUTANT:
            color = game->palette.mutant;
            DrawTriangle(Vec2(screen.x + 17.0f, screen.y), Vec2(screen.x - 14.0f, screen.y - 11.0f), Vec2(screen.x - 7.0f, screen.y), Fade(color, 0.85f));
            DrawTriangle(Vec2(screen.x + 17.0f, screen.y), Vec2(screen.x - 14.0f, screen.y + 11.0f), Vec2(screen.x - 7.0f, screen.y), Fade(color, 0.85f));
            DrawLineEx(Vec2(screen.x - 18.0f, screen.y), Vec2(screen.x + 10.0f, screen.y), 2.0f, color);
            break;

        case ENEMY_BOMBER:
            color = game->palette.bomber;
            DrawRectangle((int)(screen.x - 16.0f), (int)(screen.y - 8.0f), 32, 16, Fade(color, 0.8f));
            DrawTriangle(Vec2(screen.x - 18.0f, screen.y - 4.0f), Vec2(screen.x - 28.0f, screen.y), Vec2(screen.x - 18.0f, screen.y + 4.0f), color);
            DrawCircleV(Vec2(screen.x - 6.0f, screen.y + 12.0f), 3.0f, color);
            DrawCircleV(Vec2(screen.x + 6.0f, screen.y + 12.0f), 3.0f, color);
            break;

        case ENEMY_BAITER:
            color = game->palette.baiter;
            DrawTriangle(Vec2(screen.x + 18.0f, screen.y), Vec2(screen.x - 14.0f, screen.y - 14.0f), Vec2(screen.x - 4.0f, screen.y), Fade(color, 0.86f));
            DrawTriangle(Vec2(screen.x + 18.0f, screen.y), Vec2(screen.x - 14.0f, screen.y + 14.0f), Vec2(screen.x - 4.0f, screen.y), Fade(color, 0.86f));
            DrawLineEx(Vec2(screen.x - 4.0f, screen.y - 14.0f), Vec2(screen.x + 6.0f, screen.y), 2.0f, color);
            DrawLineEx(Vec2(screen.x - 4.0f, screen.y + 14.0f), Vec2(screen.x + 6.0f, screen.y), 2.0f, color);
            break;

        case ENEMY_SWARMER:
            color = game->palette.swarmer;
            DrawTriangle(Vec2(screen.x + 12.0f, screen.y), Vec2(screen.x - 8.0f, screen.y - 8.0f), Vec2(screen.x - 8.0f, screen.y + 8.0f), Fade(color, 0.86f));
            DrawCircleV(Vec2(screen.x - 2.0f, screen.y), 2.0f, color);
            break;

        case ENEMY_POD:
            color = game->palette.pod;
            DrawCircleV(screen, 15.0f, Fade(color, 0.7f));
            DrawCircleLines((int)screen.x, (int)screen.y, 15.0f, color);
            DrawLineEx(Vec2(screen.x - 10.0f, screen.y), Vec2(screen.x + 10.0f, screen.y), 2.0f, color);
            DrawCircleV(Vec2(screen.x - 5.0f, screen.y), 2.0f, color);
            DrawCircleV(Vec2(screen.x + 5.0f, screen.y), 2.0f, color);
            break;
    }
}

static void DrawProjectile(const Game *game, const Projectile *projectile)
{
    Vector2 screen = WorldToScreen(game, projectile->position);
    Color color = projectile->fromPlayer ? game->palette.projectilePlayer : game->palette.projectileEnemy;

    if (!projectile->fromPlayer && projectile->ownerType == ENEMY_BOMBER)
    {
        color = game->palette.mine;
    }

    if (screen.x < -30.0f || screen.x > SCREEN_WIDTH + 30.0f) return;
    if (screen.y < -30.0f || screen.y > SCREEN_HEIGHT + 30.0f) return;

    if (!projectile->fromPlayer && projectile->ownerType == ENEMY_BOMBER)
    {
        DrawCircleV(screen, projectile->radius, Fade(color, 0.8f));
        DrawCircleLines((int)screen.x, (int)screen.y, projectile->radius + 2.5f, color);
    }
    else
    {
        Vector2 tail = Vec2(screen.x - projectile->velocity.x*0.02f, screen.y - projectile->velocity.y*0.02f);
        DrawLineEx(tail, screen, projectile->radius + 1.0f, Fade(color, 0.35f));
        DrawCircleV(screen, projectile->radius, color);
    }
}

static void DrawParticles(const Game *game)
{
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        const Particle *particle = &game->particles[i];
        Vector2 screen;

        if (!particle->active) continue;

        screen = WorldToScreen(game, particle->position);
        if (screen.x < -30.0f || screen.x > SCREEN_WIDTH + 30.0f) continue;
        if (screen.y < -30.0f || screen.y > SCREEN_HEIGHT + 30.0f) continue;

        DrawCircleV(screen, particle->size, FadeByLife(particle->color, particle->life, particle->maxLife));
    }
}

static void DrawCallouts(const Game *game)
{
    for (int i = 0; i < MAX_CALLOUTS; ++i)
    {
        const FloatingText *callout = &game->callouts[i];
        Vector2 screen;
        Color color;

        if (!callout->active) continue;

        screen = WorldToScreen(game, callout->position);
        color = FadeByLife(callout->color, callout->life, callout->maxLife);
        DrawText(callout->text, (int)screen.x - MeasureText(callout->text, 18)/2, (int)screen.y, 18, color);
    }
}

static void DrawRadar(const Game *game)
{
    const int radarX = 220;
    const int radarY = SCREEN_HEIGHT - 38;
    const int radarWidth = SCREEN_WIDTH - 440;
    const int radarHeight = 18;
    int playerRadarX = radarX + (int)(game->player.position.x/game->world.width*(float)radarWidth);

    DrawRectangle(radarX - 4, radarY - 4, radarWidth + 8, radarHeight + 8, Fade(BLACK, 0.45f));
    DrawRectangleLines(radarX - 4, radarY - 4, radarWidth + 8, radarHeight + 8, Fade(game->palette.hud, 0.35f));
    DrawRectangle(radarX, radarY, radarWidth, radarHeight, Fade(game->palette.backdropTop, 0.7f));

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        const Human *human = &game->humans[i];
        int x;

        if (!human->active) continue;

        x = radarX + (int)(human->position.x/game->world.width*(float)radarWidth);
        DrawLine(x, radarY + 4, x, radarY + radarHeight - 4, game->palette.human);
    }

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        const Enemy *enemy = &game->enemies[i];
        Color color = game->palette.warning;
        int x;

        if (!enemy->active) continue;

        x = radarX + (int)(enemy->position.x/game->world.width*(float)radarWidth);

        switch (enemy->type)
        {
            case ENEMY_LANDER: color = game->palette.lander; break;
            case ENEMY_MUTANT: color = game->palette.mutant; break;
            case ENEMY_BOMBER: color = game->palette.bomber; break;
            case ENEMY_BAITER: color = game->palette.baiter; break;
            case ENEMY_SWARMER: color = game->palette.swarmer; break;
            case ENEMY_POD: color = game->palette.pod; break;
        }

        DrawCircle(x, radarY + radarHeight/2, 2.0f, color);
    }

    DrawTriangle(Vec2((float)playerRadarX, (float)radarY - 4.0f),
                 Vec2((float)playerRadarX - 5.0f, (float)radarY - 12.0f),
                 Vec2((float)playerRadarX + 5.0f, (float)radarY - 12.0f),
                 game->palette.playerPrimary);
}

void RenderWorld(const Game *game)
{
    ClearBackground(game->palette.background);
    DrawBackgroundGradient(game);
    DrawStarLayer(game, game->starsFar, STAR_LAYER_FAR_COUNT, 0.15f, 1.0f, Fade(game->palette.hud, 0.25f));
    DrawStarLayer(game, game->starsMid, STAR_LAYER_MID_COUNT, 0.3f, 1.4f, Fade(game->palette.playerPrimary, 0.35f));
    DrawStarLayer(game, game->starsNear, STAR_LAYER_NEAR_COUNT, 0.5f, 1.8f, Fade(game->palette.playerAccent, 0.50f));
    DrawTerrain(game);
    DrawParticles(game);

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        if (game->humans[i].active) DrawHuman(game, &game->humans[i]);
    }

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        if (game->enemies[i].active) DrawEnemy(game, &game->enemies[i]);
    }

    for (int i = 0; i < MAX_PROJECTILES; ++i)
    {
        if (game->projectiles[i].active) DrawProjectile(game, &game->projectiles[i]);
    }

    DrawPlayer(game);
    DrawCallouts(game);
}

void RenderHud(const Game *game)
{
    char hudText[128];
    int livingHumans = 0;

    for (int i = 0; i < MAX_HUMANS; ++i)
    {
        if (game->humans[i].active) livingHumans += 1;
    }

    snprintf(hudText, sizeof(hudText), "SCORE %06d   WAVE %02d", game->score, game->wave.index);
    DrawText(hudText, 24, 20, 28, game->palette.hud);

    snprintf(hudText, sizeof(hudText), "LIVES %d   BOMBS %d", game->player.lives, game->player.smartBombs);
    DrawText(hudText, SCREEN_WIDTH - MeasureText(hudText, 28) - 24, 20, 28, game->palette.hud);

    snprintf(hudText, sizeof(hudText), "HUMANS %d/%d", livingHumans, HUMAN_SPAWN_COUNT);
    DrawText(hudText, SCREEN_WIDTH/2 - MeasureText(hudText, 24)/2, 24, 24, game->palette.human);

    DrawRadar(game);

    if (game->wave.bannerTimer > 0.0f && game->mode == GAME_MODE_PLAYING)
    {
        snprintf(hudText, sizeof(hudText), "WAVE %d", game->wave.index);
        DrawText(hudText, SCREEN_WIDTH/2 - MeasureText(hudText, 42)/2, 82, 42, Fade(game->palette.playerAccent, ClampFloat(game->wave.bannerTimer/2.4f, 0.2f, 1.0f)));
    }

    if (game->mode == GAME_MODE_TITLE)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.25f));
        DrawText("DEFENDER", SCREEN_WIDTH/2 - MeasureText("DEFENDER", 86)/2, 120, 86, game->palette.playerPrimary);
        DrawText("Primitive-Only Modern Arcade", SCREEN_WIDTH/2 - MeasureText("Primitive-Only Modern Arcade", 28)/2, 220, 28, game->palette.hud);
        DrawText("PRESS ENTER OR A TO LAUNCH", SCREEN_WIDTH/2 - MeasureText("PRESS ENTER OR A TO LAUNCH", 30)/2, 320, 30, game->palette.playerAccent);
        DrawText("A/D OR LEFT STICK: MOVE   W/UP OR A: THRUST", SCREEN_WIDTH/2 - MeasureText("A/D OR LEFT STICK: MOVE   W/UP OR A: THRUST", 22)/2, 432, 22, game->palette.hud);
        DrawText("SPACE OR RT: FIRE   L-SHIFT OR LB: SMART BOMB   TAB/E OR RB: HYPERSPACE", SCREEN_WIDTH/2 - MeasureText("SPACE OR RT: FIRE   L-SHIFT OR LB: SMART BOMB   TAB/E OR RB: HYPERSPACE", 22)/2, 466, 22, game->palette.hud);
        DrawText("Protect humans. Stop abductions. Survive the wraparound frontier.", SCREEN_WIDTH/2 - MeasureText("Protect humans. Stop abductions. Survive the wraparound frontier.", 24)/2, 548, 24, game->palette.human);
    }
    else if (game->mode == GAME_MODE_PAUSED)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.35f));
        DrawText("PAUSED", SCREEN_WIDTH/2 - MeasureText("PAUSED", 64)/2, SCREEN_HEIGHT/2 - 56, 64, game->palette.playerAccent);
        DrawText("PRESS ESC OR START TO RESUME", SCREEN_WIDTH/2 - MeasureText("PRESS ESC OR START TO RESUME", 28)/2, SCREEN_HEIGHT/2 + 18, 28, game->palette.hud);
    }
    else if (game->mode == GAME_MODE_WAVE_CLEAR)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.22f));
        DrawText("WAVE CLEAR", SCREEN_WIDTH/2 - MeasureText("WAVE CLEAR", 58)/2, SCREEN_HEIGHT/2 - 52, 58, game->palette.playerAccent);
        DrawText("Survivor bonus awarded. Prepare for the next push.", SCREEN_WIDTH/2 - MeasureText("Survivor bonus awarded. Prepare for the next push.", 26)/2, SCREEN_HEIGHT/2 + 18, 26, game->palette.human);
    }
    else if (game->mode == GAME_MODE_GAME_OVER)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.4f));
        DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 68)/2, SCREEN_HEIGHT/2 - 64, 68, game->palette.warning);
        DrawText("PRESS ENTER OR A TO RESTART", SCREEN_WIDTH/2 - MeasureText("PRESS ENTER OR A TO RESTART", 28)/2, SCREEN_HEIGHT/2 + 20, 28, game->palette.hud);
    }
}
