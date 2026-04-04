#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct Config {
    int screen_width;
    int screen_height;
    int target_fps;
    bool fullscreen;
    float master_volume;
} Config;

static inline Config config_defaults(void) {
    return (Config){
        .screen_width  = 1280,
        .screen_height = 720,
        .target_fps    = 60,
        .fullscreen    = false,
        .master_volume = 1.0f,
    };
}

#endif
