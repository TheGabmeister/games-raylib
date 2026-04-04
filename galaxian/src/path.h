#ifndef PATH_H
#define PATH_H

#include "raylib.h"

#define MAX_PATH_SEGMENTS 6

typedef struct {
    Vector2 p0, c1, c2, p3;
} BezierSegment;

typedef enum {
    PATH_SWOOP_LEFT,
    PATH_SWOOP_RIGHT,
    PATH_LOOP,
    PATH_FLAGSHIP_CENTER
} DivePathType;

typedef struct {
    BezierSegment segments[MAX_PATH_SEGMENTS];
    int segment_count;
    float total_time;
} DivePath;

Vector2 EvalBezier(BezierSegment *seg, float t);
void GenerateDivePath(DivePath *path, Vector2 start, DivePathType type, float speed_mult);
void GenerateReturnPath(BezierSegment *seg, Vector2 from, Vector2 to);
Vector2 EvalDivePath(DivePath *path, float t_global);
float PathGetRotation(DivePath *path, float t_global);
float BezierRotation(BezierSegment *seg, float t);

#endif /* PATH_H */
