#include "scenes/scene_title.h"
#include "subsystems/renderer.h"
#include "subsystems/input.h"
#include <stdlib.h>
#include <math.h>

typedef struct {
    Renderer *renderer;
    Input *input;
    float timer;
} TitleData;

static void update(Scene *self, float dt) {
    TitleData *td = self->data;
    td->timer += dt;
    if (input_pressed(td->input, ACTION_CONFIRM))
        self->next = SCENE_GAMEPLAY;
}

static void draw(Scene *self) {
    TitleData *td = self->data;

    renderer_push_text(td->renderer, LAYER_UI,
                       (Vec2){440, 250}, "ASTEROIDS", 60, CLR_WHITE);

    if (fmodf(td->timer, 1.0f) < 0.6f) {
        renderer_push_text(td->renderer, LAYER_UI,
                           (Vec2){430, 400}, "PRESS SPACE TO PLAY", 20, CLR_GREEN);
    }
}

static void cleanup(Scene *self) {
    free(self->data);
}

Scene scene_title_create(Renderer *r, Input *inp) {
    TitleData *td = calloc(1, sizeof(TitleData));
    td->renderer = r;
    td->input = inp;
    return (Scene){
        .data    = td,
        .update  = update,
        .draw    = draw,
        .cleanup = cleanup,
        .next    = SCENE_NONE,
    };
}
