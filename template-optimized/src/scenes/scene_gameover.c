#include "scenes/scene_gameover.h"
#include "subsystems/renderer.h"
#include "subsystems/input.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    Renderer *renderer;
    Input *input;
    int score;
    float timer;
} GameOverData;

static void update(Scene *self, float dt) {
    GameOverData *go = self->data;
    go->timer += dt;
    if (go->timer > 2.0f && input_pressed(go->input, ACTION_CONFIRM))
        self->next = SCENE_TITLE;
}

static void draw(Scene *self) {
    GameOverData *go = self->data;

    renderer_push_text(go->renderer, LAYER_UI,
                       (Vec2){430, 250}, "GAME OVER", 60, CLR_RED);

    char buf[64];
    snprintf(buf, sizeof(buf), "FINAL SCORE: %d", go->score);
    renderer_push_text(go->renderer, LAYER_UI, (Vec2){450, 350}, buf, 30, CLR_WHITE);

    if (go->timer > 2.0f && fmodf(go->timer, 1.0f) < 0.6f) {
        renderer_push_text(go->renderer, LAYER_UI,
                           (Vec2){390, 450}, "PRESS SPACE TO CONTINUE", 20, CLR_GREEN);
    }
}

static void cleanup(Scene *self) {
    free(self->data);
}

Scene scene_gameover_create(Renderer *r, Input *inp, int final_score) {
    GameOverData *go = calloc(1, sizeof(GameOverData));
    go->renderer = r;
    go->input = inp;
    go->score = final_score;
    return (Scene){
        .data    = go,
        .update  = update,
        .draw    = draw,
        .cleanup = cleanup,
        .next    = SCENE_NONE,
    };
}
