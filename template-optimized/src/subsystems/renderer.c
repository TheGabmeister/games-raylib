#include "subsystems/renderer.h"
#include "core/render_queue.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

struct Renderer {
    RenderQueue queue;
    Vec2 camera_offset;
};

static Color to_rl(Colr c) { return (Color){c.r, c.g, c.b, c.a}; }
static Vector2 to_rv(Vec2 v) { return (Vector2){v.x, v.y}; }

Renderer *renderer_create(const Config *cfg, const char *title) {
    Renderer *r = calloc(1, sizeof(Renderer));
    InitWindow(cfg->screen_width, cfg->screen_height, title);
    SetTargetFPS(cfg->target_fps);
    return r;
}

void renderer_destroy(Renderer *r) {
    CloseWindow();
    free(r);
}

bool renderer_should_close(Renderer *r) {
    (void)r;
    return WindowShouldClose();
}

float renderer_get_dt(Renderer *r) {
    (void)r;
    return GetFrameTime();
}

void renderer_push_line(Renderer *r, int layer, Vec2 start, Vec2 end, float thick, Colr color) {
    rq_push(&r->queue, (RenderCommand){
        .type = RCMD_LINE, .layer = layer, .color = color,
        .line = { start, end, thick }
    });
}

void renderer_push_circle(Renderer *r, int layer, Vec2 center, float radius, bool filled, Colr color) {
    rq_push(&r->queue, (RenderCommand){
        .type = RCMD_CIRCLE, .layer = layer, .color = color,
        .circle = { center, radius, filled }
    });
}

void renderer_push_text(Renderer *r, int layer, Vec2 pos, const char *str, int size, Colr color) {
    RenderCommand cmd = { .type = RCMD_TEXT, .layer = layer, .color = color };
    cmd.text.pos = pos;
    cmd.text.size = size;
    strncpy(cmd.text.str, str, 63);
    cmd.text.str[63] = '\0';
    rq_push(&r->queue, cmd);
}

void renderer_push_neon_line(Renderer *r, int layer, Vec2 start, Vec2 end, Colr color) {
    Colr glow = color;
    glow.a = (unsigned char)(color.a * 0.3f);
    renderer_push_line(r, layer, start, end, 3.0f, glow);
    renderer_push_line(r, layer, start, end, 1.0f, color);
}

void renderer_push_neon_circle(Renderer *r, int layer, Vec2 center, float radius, Colr color) {
    Colr glow = color;
    glow.a = (unsigned char)(color.a * 0.3f);
    renderer_push_circle(r, layer, center, radius + 1.5f, false, glow);
    renderer_push_circle(r, layer, center, radius, false, color);
}

void renderer_set_camera(Renderer *r, Vec2 offset) {
    r->camera_offset = offset;
}

static int cmd_compare(const void *a, const void *b) {
    return ((const RenderCommand *)a)->layer - ((const RenderCommand *)b)->layer;
}

static void draw_cmd(const RenderCommand *cmd) {
    Color c = to_rl(cmd->color);
    switch (cmd->type) {
        case RCMD_LINE:
            DrawLineEx(to_rv(cmd->line.start), to_rv(cmd->line.end), cmd->line.thick, c);
            break;
        case RCMD_CIRCLE:
            if (cmd->circle.filled)
                DrawCircleV(to_rv(cmd->circle.center), cmd->circle.radius, c);
            else
                DrawCircleLines((int)cmd->circle.center.x, (int)cmd->circle.center.y,
                                cmd->circle.radius, c);
            break;
        case RCMD_TEXT:
            DrawText(cmd->text.str, (int)cmd->text.pos.x, (int)cmd->text.pos.y,
                     cmd->text.size, c);
            break;
    }
}

void renderer_flush(Renderer *r) {
    qsort(r->queue.commands, r->queue.count, sizeof(RenderCommand), cmd_compare);

    int ui_start = r->queue.count;
    for (int i = 0; i < r->queue.count; i++) {
        if (r->queue.commands[i].layer >= LAYER_UI) { ui_start = i; break; }
    }

    BeginDrawing();
    ClearBackground(BLACK);

    Camera2D cam = {
        .offset   = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f },
        .target   = { SCREEN_WIDTH / 2.0f - r->camera_offset.x,
                      SCREEN_HEIGHT / 2.0f - r->camera_offset.y },
        .rotation = 0,
        .zoom     = 1.0f,
    };
    BeginMode2D(cam);
    for (int i = 0; i < ui_start; i++) draw_cmd(&r->queue.commands[i]);
    EndMode2D();

    for (int i = ui_start; i < r->queue.count; i++) draw_cmd(&r->queue.commands[i]);

    EndDrawing();
    rq_clear(&r->queue);
}
