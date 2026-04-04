#ifndef SCENE_H
#define SCENE_H

typedef enum {
    SCENE_NONE = 0,
    SCENE_TITLE,
    SCENE_GAMEPLAY,
    SCENE_GAMEOVER,
} SceneID;

typedef struct Scene Scene;

struct Scene {
    void *data;
    void (*update)(Scene *self, float dt);
    void (*draw)(Scene *self);
    void (*cleanup)(Scene *self);
    SceneID next;
    int final_score;
};

#endif
