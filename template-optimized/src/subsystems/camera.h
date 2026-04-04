#ifndef CAMERA_H
#define CAMERA_H

#include "core/types.h"

typedef struct Camera Camera;

Camera *camera_create(void);
void camera_destroy(Camera *cam);
void camera_update(Camera *cam, float dt);
void camera_shake(Camera *cam, float duration, float magnitude);
Vec2 camera_get_offset(const Camera *cam);

#endif
