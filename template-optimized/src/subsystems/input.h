#ifndef INPUT_H
#define INPUT_H

#include "core/types.h"

typedef enum {
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_FIRE,
    ACTION_CONFIRM,
    ACTION_COUNT,
} Action;

typedef struct Input Input;

Input *input_create(void);
void input_destroy(Input *inp);
void input_update(Input *inp);
bool input_held(const Input *inp, Action a);
bool input_pressed(const Input *inp, Action a);

#endif
