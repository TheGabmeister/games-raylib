#include "input.h"

static bool IsTriggerPressed(float value)
{
    return value > 0.35f;
}

void UpdateInput(InputState *input)
{
    bool currentDown[INPUT_ACTION_COUNT] = { false };
    float keyboardAxis = 0.0f;
    float stickAxis = 0.0f;
    bool usingGamepad = IsGamepadAvailable(0);
    float rightTrigger = -1.0f;
    float leftTrigger = -1.0f;
    bool gamepadReverse = false;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) keyboardAxis -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) keyboardAxis += 1.0f;

    if (usingGamepad)
    {
        stickAxis = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        if (fabsf(stickAxis) < 0.2f) stickAxis = 0.0f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) stickAxis = -1.0f;
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) stickAxis = 1.0f;

        rightTrigger = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER);
        leftTrigger = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER);
        gamepadReverse = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || IsTriggerPressed(leftTrigger);
    }

    input->usingGamepad = usingGamepad;
    input->moveAxis = ClampFloat(keyboardAxis + stickAxis, -1.0f, 1.0f);

    currentDown[INPUT_ACTION_MOVE_LEFT] = (input->moveAxis < -0.1f);
    currentDown[INPUT_ACTION_MOVE_RIGHT] = (input->moveAxis > 0.1f);
    currentDown[INPUT_ACTION_THRUST] = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ||
                                       (usingGamepad && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
    currentDown[INPUT_ACTION_REVERSE] = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) || gamepadReverse;
    currentDown[INPUT_ACTION_FIRE] = IsKeyDown(KEY_SPACE) ||
                                     (usingGamepad && (IsTriggerPressed(rightTrigger) ||
                                     IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) ||
                                     IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)));
    currentDown[INPUT_ACTION_SMART_BOMB] = IsKeyDown(KEY_LEFT_SHIFT) ||
                                           (usingGamepad && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1));
    currentDown[INPUT_ACTION_HYPERSPACE] = IsKeyDown(KEY_TAB) || IsKeyDown(KEY_E) ||
                                           (usingGamepad && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1));
    currentDown[INPUT_ACTION_PAUSE] = IsKeyDown(KEY_ESCAPE) ||
                                      (usingGamepad && IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT));
    currentDown[INPUT_ACTION_CONFIRM] = IsKeyDown(KEY_ENTER) ||
                                        (usingGamepad && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));

    for (int i = 0; i < INPUT_ACTION_COUNT; ++i)
    {
        input->pressed[i] = input->pressed[i] || (currentDown[i] && !input->previousDown[i]);
        input->down[i] = currentDown[i];
        input->previousDown[i] = currentDown[i];
    }
}

void ClearInputPressed(InputState *input)
{
    for (int i = 0; i < INPUT_ACTION_COUNT; ++i)
    {
        input->pressed[i] = false;
    }
}
