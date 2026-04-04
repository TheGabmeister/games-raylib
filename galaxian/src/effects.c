#include "effects.h"
#include "config.h"
#include "rlgl.h"

void EffectsInit(Effects *fx)
{
    fx->shake_timer = 0;
    fx->shake_intensity = 0;
    fx->shake_offset = (Vector2){0, 0};
    fx->flash_timer = 0;
    fx->flash_alpha = 0;
}

void EffectsUpdate(Effects *fx, float dt)
{
    if (fx->shake_timer > 0) {
        fx->shake_timer -= dt;
        float t = fx->shake_timer / SHAKE_DURATION;
        float intensity = fx->shake_intensity * t;
        fx->shake_offset.x = RandFloat(-intensity, intensity);
        fx->shake_offset.y = RandFloat(-intensity, intensity);
        if (fx->shake_timer <= 0) {
            fx->shake_offset = (Vector2){0, 0};
        }
    }

    if (fx->flash_timer > 0) {
        fx->flash_timer -= dt;
        fx->flash_alpha = fx->flash_timer / FLASH_DURATION;
        if (fx->flash_timer <= 0) fx->flash_alpha = 0;
    }
}

void EffectsTriggerShake(Effects *fx, float intensity)
{
    fx->shake_timer = SHAKE_DURATION;
    fx->shake_intensity = intensity;
}

void EffectsTriggerFlash(Effects *fx)
{
    fx->flash_timer = FLASH_DURATION;
    fx->flash_alpha = 1.0f;
}

void EffectsApplyShake(Effects *fx)
{
    rlTranslatef(fx->shake_offset.x, fx->shake_offset.y, 0);
}

void EffectsResetShake(Effects *fx)
{
    rlTranslatef(-fx->shake_offset.x, -fx->shake_offset.y, 0);
}

void EffectsDrawFlash(Effects *fx)
{
    if (fx->flash_alpha > 0.01f) {
        Color white = { 255, 255, 255, (unsigned char)(fx->flash_alpha * 255) };
        DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, white);
    }
}
