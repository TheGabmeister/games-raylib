#include "draw_utils.h"
#include "config.h"
#include <math.h>

Vector2 RotatePoint(Vector2 point, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return (Vector2){ point.x * c - point.y * s, point.x * s + point.y * c };
}

void DrawGlow(Vector2 pos, float radius, Color color)
{
    BeginBlendMode(BLEND_ADDITIVE);
    Color gc = color;
    gc.a = 38;  /* ~15% */
    DrawCircleGradient((int)pos.x, (int)pos.y, radius, gc, (Color){0, 0, 0, 0});
    EndBlendMode();
}

void DrawNeonTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    /* Glow — large circle at centroid */
    Vector2 center = { (v1.x + v2.x + v3.x) / 3.0f, (v1.y + v2.y + v3.y) / 3.0f };
    float dx = v1.x - v2.x, dy = v1.y - v2.y;
    float size = sqrtf(dx * dx + dy * dy);
    DrawGlow(center, size * 1.5f, color);

    /* Fill at 40% alpha */
    Color fill = color;
    fill.a = 102;
    DrawTriangle(v1, v2, v3, fill);

    /* Edge at full brightness */
    DrawTriangleLines(v1, v2, v3, color);
}

void DrawNeonRect(Vector2 pos, float w, float h, Color color, float rotation)
{
    Vector2 center = { pos.x + w * 0.5f, pos.y + h * 0.5f };
    DrawGlow(center, fmaxf(w, h) * 1.2f, color);

    float deg = rotation * RAD2DEG;
    Rectangle rec = { pos.x + w * 0.5f, pos.y + h * 0.5f, w, h };
    Vector2 origin = { w * 0.5f, h * 0.5f };
    Color fill = color;
    fill.a = 102;
    DrawRectanglePro(rec, origin, deg, fill);

    /* Outline via rotated corner lines */
    Vector2 corners[4] = {
        {-w * 0.5f, -h * 0.5f}, { w * 0.5f, -h * 0.5f},
        { w * 0.5f,  h * 0.5f}, {-w * 0.5f,  h * 0.5f}
    };
    for (int i = 0; i < 4; i++) {
        Vector2 ra = RotatePoint(corners[i], rotation);
        Vector2 rb = RotatePoint(corners[(i + 1) % 4], rotation);
        DrawLineV((Vector2){center.x + ra.x, center.y + ra.y},
                  (Vector2){center.x + rb.x, center.y + rb.y}, color);
    }
}

/* ======== Entity Drawing ======== */

static void DrawRotatedTriangle(Vector2 pos, Vector2 a, Vector2 b, Vector2 c, float angle, Color color)
{
    Vector2 ra = RotatePoint(a, angle);
    Vector2 rb = RotatePoint(b, angle);
    Vector2 rc = RotatePoint(c, angle);
    Vector2 v1 = { pos.x + ra.x, pos.y + ra.y };
    Vector2 v2 = { pos.x + rb.x, pos.y + rb.y };
    Vector2 v3 = { pos.x + rc.x, pos.y + rc.y };

    Color fill = color;
    fill.a = 102;
    DrawTriangle(v1, v2, v3, fill);
    DrawTriangleLines(v1, v2, v3, color);
}

void DrawPlayerShip(Vector2 pos, Color color, float alpha)
{
    Color c = color;
    c.a = (unsigned char)(255 * alpha);
    Color cfill = c;
    cfill.a = (unsigned char)(102 * alpha);

    /* Glow */
    DrawGlow(pos, 30.0f, c);

    /* Body triangle (pointing up) — raylib needs CCW winding */
    Vector2 top = { pos.x, pos.y - 14 };
    Vector2 bl  = { pos.x - 12, pos.y + 10 };
    Vector2 br  = { pos.x + 12, pos.y + 10 };
    DrawTriangle(top, br, bl, cfill);
    DrawTriangleLines(top, br, bl, c);

    /* Left wing */
    Vector2 lw1 = { pos.x - 12, pos.y + 4 };
    Vector2 lw2 = { pos.x - 18, pos.y - 2 };
    Vector2 lw3 = { pos.x - 8,  pos.y + 8 };
    DrawTriangle(lw1, lw3, lw2, cfill);
    DrawTriangleLines(lw1, lw3, lw2, c);

    /* Right wing */
    Vector2 rw1 = { pos.x + 12, pos.y + 4 };
    Vector2 rw2 = { pos.x + 18, pos.y - 2 };
    Vector2 rw3 = { pos.x + 8,  pos.y + 8 };
    DrawTriangle(rw1, rw2, rw3, cfill);
    DrawTriangleLines(rw1, rw2, rw3, c);

    /* Engine block */
    Color efill = c;
    efill.a = (unsigned char)(80 * alpha);
    DrawRectangle((int)pos.x - 4, (int)pos.y + 8, 8, 5, efill);
    DrawRectangleLines((int)pos.x - 4, (int)pos.y + 8, 8, 5, c);

    /* Thrust glow at engine nozzle */
    Color thrust = { 255, 180, 50, (unsigned char)(120 * alpha) };
    BeginBlendMode(BLEND_ADDITIVE);
    float pulse = 0.8f + 0.2f * sinf((float)GetTime() * 20.0f);
    DrawCircleGradient((int)pos.x, (int)pos.y + 14, 8.0f * pulse, thrust, (Color){0, 0, 0, 0});
    EndBlendMode();
}

void DrawBlueEnemy(Vector2 pos, float rotation, Color color)
{
    DrawGlow(pos, 24.0f, color);

    /* Downward chevron: two arms forming a V */
    /* Left arm */
    DrawRotatedTriangle(pos,
        (Vector2){0, 8}, (Vector2){-12, -8}, (Vector2){-2, -2},
        rotation, color);
    /* Right arm */
    DrawRotatedTriangle(pos,
        (Vector2){0, 8}, (Vector2){2, -2}, (Vector2){12, -8},
        rotation, color);
}

void DrawPurpleEnemy(Vector2 pos, float rotation, Color color)
{
    DrawGlow(pos, 28.0f, color);

    /* Diamond / kite shape */
    /* Top half */
    DrawRotatedTriangle(pos,
        (Vector2){0, -14}, (Vector2){10, 0}, (Vector2){-10, 0},
        rotation, color);
    /* Bottom half */
    DrawRotatedTriangle(pos,
        (Vector2){0, 14}, (Vector2){-10, 0}, (Vector2){10, 0},
        rotation, color);
    /* Left extension */
    DrawRotatedTriangle(pos,
        (Vector2){-10, 0}, (Vector2){-10, 8}, (Vector2){-16, -4},
        rotation, color);
    /* Right extension */
    DrawRotatedTriangle(pos,
        (Vector2){10, 0}, (Vector2){16, -4}, (Vector2){10, 8},
        rotation, color);
}

void DrawRedEnemy(Vector2 pos, float rotation, Color color)
{
    DrawGlow(pos, 30.0f, color);

    /* Body triangle */
    DrawRotatedTriangle(pos,
        (Vector2){0, -10}, (Vector2){6, 8}, (Vector2){-6, 8},
        rotation, color);
    /* Left swept wing */
    DrawRotatedTriangle(pos,
        (Vector2){-6, 0}, (Vector2){-8, 6}, (Vector2){-16, -6},
        rotation, color);
    /* Right swept wing */
    DrawRotatedTriangle(pos,
        (Vector2){6, 0}, (Vector2){16, -6}, (Vector2){8, 6},
        rotation, color);
}

void DrawFlagshipEnemy(Vector2 pos, float rotation, Color color)
{
    DrawGlow(pos, 36.0f, color);

    Color fill = color;
    fill.a = 102;

    /* Body — centered rounded rectangle with rotation via DrawRectanglePro */
    Rectangle body = { pos.x, pos.y, 24, 16 };
    Vector2 body_origin = { 12, 8 };
    DrawRectanglePro(body, body_origin, rotation * RAD2DEG, fill);
    DrawRectanglePro(body, body_origin, rotation * RAD2DEG, (Color){color.r, color.g, color.b, 40});
    /* Outline via slightly larger rect */
    Rectangle body_outline = { pos.x, pos.y, 26, 18 };
    Vector2 outline_origin = { 13, 9 };
    DrawRectanglePro(body_outline, outline_origin, rotation * RAD2DEG, (Color){color.r, color.g, color.b, 60});

    /* Dome at top */
    Vector2 domePos = RotatePoint((Vector2){0, -10}, rotation);
    BeginBlendMode(BLEND_ADDITIVE);
    Color dg = color;
    dg.a = 60;
    DrawCircleGradient((int)(pos.x + domePos.x), (int)(pos.y + domePos.y), 7.0f, dg, (Color){0, 0, 0, 0});
    EndBlendMode();
    DrawCircleSectorLines((Vector2){pos.x + domePos.x, pos.y + domePos.y}, 7.0f, 180, 360, 12, color);

    /* Left nacelle */
    DrawRotatedTriangle(pos,
        (Vector2){-14, -2}, (Vector2){-10, 8}, (Vector2){-18, 8},
        rotation, color);
    /* Right nacelle */
    DrawRotatedTriangle(pos,
        (Vector2){14, -2}, (Vector2){18, 8}, (Vector2){10, 8},
        rotation, color);
}

void DrawBulletGlow(Vector2 pos, Vector2 vel, Color color)
{
    DrawGlow(pos, 10.0f, color);

    /* Elongated trail in direction opposite to velocity */
    float len = sqrtf(vel.x * vel.x + vel.y * vel.y);
    if (len > 0.01f) {
        Vector2 dir = { -vel.x / len, -vel.y / len };
        Color trail = color;
        trail.a = 80;
        BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleGradient((int)(pos.x + dir.x * 4), (int)(pos.y + dir.y * 4), 5.0f, trail, (Color){0, 0, 0, 0});
        EndBlendMode();
    }

    /* Bullet core */
    Color fill = color;
    fill.a = 200;
    DrawCircleV(pos, 2.5f, fill);
    DrawCircleLines((int)pos.x, (int)pos.y, 2.5f, color);
}
