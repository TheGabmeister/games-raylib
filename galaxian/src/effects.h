#ifndef EFFECTS_H
#define EFFECTS_H

#include "raylib.h"

typedef struct {
    float shake_timer;
    float shake_intensity;
    Vector2 shake_offset;
    float flash_timer;
    float flash_alpha;
} Effects;

void EffectsInit(Effects *fx);
void EffectsUpdate(Effects *fx, float dt);
void EffectsTriggerShake(Effects *fx, float intensity);
void EffectsTriggerFlash(Effects *fx);
void EffectsApplyShake(Effects *fx);
void EffectsResetShake(Effects *fx);
void EffectsDrawFlash(Effects *fx);

#endif /* EFFECTS_H */
