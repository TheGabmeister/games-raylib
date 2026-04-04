#include "subsystems/input.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYS 3

typedef struct { int keys[MAX_KEYS]; int count; } Binding;

struct Input {
    Binding bindings[ACTION_COUNT];
    bool current[ACTION_COUNT];
    bool previous[ACTION_COUNT];
};

Input *input_create(void) {
    Input *inp = calloc(1, sizeof(Input));
    inp->bindings[ACTION_UP]      = (Binding){{ KEY_W,     KEY_UP    }, 2};
    inp->bindings[ACTION_DOWN]    = (Binding){{ KEY_S,     KEY_DOWN  }, 2};
    inp->bindings[ACTION_LEFT]    = (Binding){{ KEY_A,     KEY_LEFT  }, 2};
    inp->bindings[ACTION_RIGHT]   = (Binding){{ KEY_D,     KEY_RIGHT }, 2};
    inp->bindings[ACTION_FIRE]    = (Binding){{ KEY_SPACE            }, 1};
    inp->bindings[ACTION_CONFIRM] = (Binding){{ KEY_ENTER, KEY_SPACE }, 2};
    return inp;
}

void input_destroy(Input *inp) { free(inp); }

void input_update(Input *inp) {
    memcpy(inp->previous, inp->current, sizeof(inp->current));
    for (int a = 0; a < ACTION_COUNT; a++) {
        inp->current[a] = false;
        for (int k = 0; k < inp->bindings[a].count; k++) {
            if (IsKeyDown(inp->bindings[a].keys[k])) {
                inp->current[a] = true;
                break;
            }
        }
    }
}

bool input_held(const Input *inp, Action a) {
    return inp->current[a];
}

bool input_pressed(const Input *inp, Action a) {
    return inp->current[a] && !inp->previous[a];
}
