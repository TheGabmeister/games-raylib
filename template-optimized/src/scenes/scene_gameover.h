#ifndef SCENE_GAMEOVER_H
#define SCENE_GAMEOVER_H

#include "scenes/scene.h"

typedef struct Renderer Renderer;
typedef struct Input Input;

Scene scene_gameover_create(Renderer *r, Input *inp, int final_score);

#endif
