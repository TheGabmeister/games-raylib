#include "core/config.h"
#include "subsystems/renderer.h"
#include "subsystems/input.h"
#include "subsystems/camera.h"
#include "subsystems/debug.h"
#include "scenes/scene.h"
#include "scenes/scene_title.h"
#include "scenes/scene_gameplay.h"
#include "scenes/scene_gameover.h"

int main(void) {
    Config    config = config_defaults();
    Renderer *renderer = renderer_create(&config, "Template");
    Input    *input    = input_create();
    Camera   *camera   = camera_create();
    Debug     debug;
    debug_init(&debug);

    Scene scene = scene_title_create(renderer, input);

    while (!renderer_should_close(renderer)) {
        float dt = renderer_get_dt(renderer);

        debug_update(&debug, dt);
        input_update(input);

        scene.update(&scene, dt);
        scene.draw(&scene);
        debug_draw(&debug, renderer);
        renderer_flush(renderer);

        if (scene.next != SCENE_NONE) {
            int score = scene.final_score;
            SceneID next = scene.next;
            scene.cleanup(&scene);

            switch (next) {
                case SCENE_TITLE:
                    scene = scene_title_create(renderer, input);
                    break;
                case SCENE_GAMEPLAY:
                    scene = scene_gameplay_create(renderer, input, camera, &debug);
                    break;
                case SCENE_GAMEOVER:
                    scene = scene_gameover_create(renderer, input, score);
                    break;
                default:
                    break;
            }
        }
    }

    scene.cleanup(&scene);
    camera_destroy(camera);
    input_destroy(input);
    renderer_destroy(renderer);
    return 0;
}
