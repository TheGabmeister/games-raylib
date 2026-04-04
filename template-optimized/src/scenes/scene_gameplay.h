#ifndef SCENE_GAMEPLAY_H
#define SCENE_GAMEPLAY_H

#include "scenes/scene.h"

typedef struct Renderer Renderer;
typedef struct Input Input;
typedef struct Camera Camera;
typedef struct Debug Debug;

Scene scene_gameplay_create(Renderer *r, Input *inp, Camera *cam, Debug *debug);

#endif
