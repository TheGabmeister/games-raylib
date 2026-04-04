#include "path.h"
#include "config.h"
#include <math.h>

Vector2 EvalBezier(BezierSegment *seg, float t)
{
    float u = 1.0f - t;
    float uu = u * u;
    float uuu = uu * u;
    float tt = t * t;
    float ttt = tt * t;
    Vector2 p;
    p.x = uuu * seg->p0.x + 3 * uu * t * seg->c1.x + 3 * u * tt * seg->c2.x + ttt * seg->p3.x;
    p.y = uuu * seg->p0.y + 3 * uu * t * seg->c1.y + 3 * u * tt * seg->c2.y + ttt * seg->p3.y;
    return p;
}

float BezierRotation(BezierSegment *seg, float t)
{
    float dt = 0.01f;
    float t0 = t - dt;
    float t1 = t + dt;
    if (t0 < 0) t0 = 0;
    if (t1 > 1) t1 = 1;
    Vector2 p0 = EvalBezier(seg, t0);
    Vector2 p1 = EvalBezier(seg, t1);
    return atan2f(p1.x - p0.x, p1.y - p0.y);
}

Vector2 EvalDivePath(DivePath *path, float t_global)
{
    float seg_f = t_global * path->segment_count;
    int seg = (int)seg_f;
    if (seg >= path->segment_count) seg = path->segment_count - 1;
    float local_t = seg_f - seg;
    if (local_t < 0) local_t = 0;
    if (local_t > 1) local_t = 1;
    return EvalBezier(&path->segments[seg], local_t);
}

float PathGetRotation(DivePath *path, float t_global)
{
    float dt = 0.005f;
    float t0 = t_global - dt;
    float t1 = t_global + dt;
    if (t0 < 0) t0 = 0;
    if (t1 > 1) t1 = 1;
    Vector2 p0 = EvalDivePath(path, t0);
    Vector2 p1 = EvalDivePath(path, t1);
    return atan2f(p1.x - p0.x, p1.y - p0.y);
}

void GenerateDivePath(DivePath *path, Vector2 s, DivePathType type, float speed_mult)
{
    float exit_y = VIRTUAL_HEIGHT + 50.0f;

    switch (type) {
    case PATH_SWOOP_LEFT:
        path->segment_count = 3;
        path->total_time = 3.0f / speed_mult;
        path->segments[0] = (BezierSegment){
            s,
            {s.x, s.y + 60},
            {s.x - 80, s.y + 120},
            {s.x - 100, s.y + 200}
        };
        path->segments[1] = (BezierSegment){
            {s.x - 100, s.y + 200},
            {s.x - 120, s.y + 280},
            {s.x - 40, s.y + 380},
            {s.x, s.y + 400}
        };
        path->segments[2] = (BezierSegment){
            {s.x, s.y + 400},
            {s.x + 20, s.y + 460},
            {s.x + 10, s.y + 540},
            {s.x, exit_y}
        };
        break;

    case PATH_SWOOP_RIGHT:
        path->segment_count = 3;
        path->total_time = 3.0f / speed_mult;
        path->segments[0] = (BezierSegment){
            s,
            {s.x, s.y + 60},
            {s.x + 80, s.y + 120},
            {s.x + 100, s.y + 200}
        };
        path->segments[1] = (BezierSegment){
            {s.x + 100, s.y + 200},
            {s.x + 120, s.y + 280},
            {s.x + 40, s.y + 380},
            {s.x, s.y + 400}
        };
        path->segments[2] = (BezierSegment){
            {s.x, s.y + 400},
            {s.x - 20, s.y + 460},
            {s.x - 10, s.y + 540},
            {s.x, exit_y}
        };
        break;

    case PATH_LOOP:
        path->segment_count = 4;
        path->total_time = 3.5f / speed_mult;
        path->segments[0] = (BezierSegment){
            s,
            {s.x + 40, s.y - 10},
            {s.x + 100, s.y + 30},
            {s.x + 80, s.y + 100}
        };
        path->segments[1] = (BezierSegment){
            {s.x + 80, s.y + 100},
            {s.x + 60, s.y + 170},
            {s.x - 20, s.y + 140},
            {s.x, s.y + 80}
        };
        path->segments[2] = (BezierSegment){
            {s.x, s.y + 80},
            {s.x - 20, s.y + 120},
            {s.x - 50, s.y + 220},
            {s.x - 30, s.y + 320}
        };
        path->segments[3] = (BezierSegment){
            {s.x - 30, s.y + 320},
            {s.x - 10, s.y + 420},
            {s.x, s.y + 540},
            {s.x, exit_y}
        };
        break;

    case PATH_FLAGSHIP_CENTER:
        path->segment_count = 3;
        path->total_time = 4.0f / speed_mult;
        path->segments[0] = (BezierSegment){
            s,
            {s.x, s.y + 80},
            {s.x - 120, s.y + 160},
            {s.x - 140, s.y + 250}
        };
        path->segments[1] = (BezierSegment){
            {s.x - 140, s.y + 250},
            {s.x - 160, s.y + 340},
            {s.x + 100, s.y + 400},
            {s.x + 120, s.y + 450}
        };
        path->segments[2] = (BezierSegment){
            {s.x + 120, s.y + 450},
            {s.x + 100, s.y + 520},
            {s.x, s.y + 580},
            {s.x, exit_y}
        };
        break;
    }
}

void GenerateReturnPath(BezierSegment *seg, Vector2 from, Vector2 to)
{
    float dx = to.x - from.x;
    seg->p0 = from;
    seg->c1 = (Vector2){ from.x + dx * 0.3f, from.y + 50.0f };
    seg->c2 = (Vector2){ to.x, to.y - 40.0f };
    seg->p3 = to;
}
