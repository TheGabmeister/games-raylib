#ifndef DRAW_UTILS_H
#define DRAW_UTILS_H

#include "raylib.h"

Vector2 RotatePoint(Vector2 point, float angle);

void DrawNeonTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawNeonRect(Vector2 pos, float w, float h, Color color, float rotation);
void DrawGlow(Vector2 pos, float radius, Color color);

void DrawPlayerShip(Vector2 pos, Color color, float alpha);
void DrawBlueEnemy(Vector2 pos, float rotation, Color color);
void DrawPurpleEnemy(Vector2 pos, float rotation, Color color);
void DrawRedEnemy(Vector2 pos, float rotation, Color color);
void DrawFlagshipEnemy(Vector2 pos, float rotation, Color color);
void DrawBulletGlow(Vector2 pos, Vector2 vel, Color color);

#endif /* DRAW_UTILS_H */
