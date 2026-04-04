#ifndef PARTICLE_H
#define PARTICLE_H

#include "config.h"

typedef enum {
    PARTICLE_SPARK,
    PARTICLE_DEBRIS,
    PARTICLE_THRUST,
    PARTICLE_SCORE_POPUP
} ParticleType;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float life;
    float max_life;
    Color color;
    float size;
    float rotation;
    float rot_speed;
    ParticleType type;
    int score_value;
    bool active;
} Particle;

void ParticlesUpdate(Particle *particles, int *count, float dt);
void ParticlesDraw(Particle *particles, int count);
void EmitExplosion(Particle *particles, int *count, Vector2 pos, Color color);
void EmitThrust(Particle *particles, int *count, Vector2 pos);
void EmitScorePopup(Particle *particles, int *count, Vector2 pos, int score);

#endif /* PARTICLE_H */
