#include "subsystems/camera.h"
#include <stdlib.h>
#include <math.h>

struct Camera {
    Vec2 offset;
    float shake_timer;
    float shake_magnitude;
};

Camera *camera_create(void) {
    return calloc(1, sizeof(Camera));
}

void camera_destroy(Camera *cam) { free(cam); }

void camera_update(Camera *cam, float dt) {
    if (cam->shake_timer > 0) {
        cam->shake_timer -= dt;
        float t = cam->shake_timer > 0 ? cam->shake_timer : 0;
        float intensity = cam->shake_magnitude * t;
        cam->offset.x = sinf(t * 97.0f) * intensity;
        cam->offset.y = sinf(t * 131.0f) * intensity;
    } else {
        cam->offset = (Vec2){0, 0};
    }
}

void camera_shake(Camera *cam, float duration, float magnitude) {
    cam->shake_timer = duration;
    cam->shake_magnitude = magnitude;
}

Vec2 camera_get_offset(const Camera *cam) {
    return cam->offset;
}
