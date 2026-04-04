#include "particle.h"
#include "draw_utils.h"
#include <stdio.h>
#include <math.h>

void ParticlesUpdate(Particle *particles, int *count, float dt)
{
    for (int i = 0; i < *count; ) {
        Particle *p = &particles[i];
        p->life -= dt;
        if (p->life <= 0) {
            particles[i] = particles[*count - 1];
            (*count)--;
            continue;
        }
        p->position.x += p->velocity.x * dt;
        p->position.y += p->velocity.y * dt;
        p->rotation += p->rot_speed * dt;
        i++;
    }
}

void ParticlesDraw(Particle *particles, int count)
{
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < count; i++) {
        Particle *p = &particles[i];
        float alpha = p->life / p->max_life;

        switch (p->type) {
        case PARTICLE_SPARK: {
            Color c = p->color;
            c.a = (unsigned char)(255 * alpha);
            DrawCircleGradient((int)p->position.x, (int)p->position.y,
                               p->size * alpha, c, (Color){0, 0, 0, 0});
        } break;

        case PARTICLE_DEBRIS: {
            Color c = p->color;
            c.a = (unsigned char)(200 * alpha);
            Rectangle rec = { p->position.x, p->position.y, p->size, p->size * 0.5f };
            Vector2 origin = { p->size * 0.5f, p->size * 0.25f };
            DrawRectanglePro(rec, origin, p->rotation * RAD2DEG, c);
        } break;

        case PARTICLE_THRUST: {
            Color c = p->color;
            c.a = (unsigned char)(180 * alpha);
            DrawCircleGradient((int)p->position.x, (int)p->position.y,
                               p->size * alpha, c, (Color){0, 0, 0, 0});
        } break;

        case PARTICLE_SCORE_POPUP: {
            EndBlendMode();
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", p->score_value);
            Color c = { 255, 255, 255, (unsigned char)(255 * alpha) };
            int w = MeasureText(buf, 14);
            DrawText(buf, (int)p->position.x - w / 2, (int)p->position.y, 14, c);
            BeginBlendMode(BLEND_ADDITIVE);
        } break;
        }
    }
    EndBlendMode();
}

static void AddParticle(Particle *particles, int *count, Particle p)
{
    if (*count >= MAX_PARTICLES) return;
    p.active = true;
    particles[*count] = p;
    (*count)++;
}

void EmitExplosion(Particle *particles, int *count, Vector2 pos, Color color)
{
    int sparks = GetRandomValue(20, 40);
    for (int i = 0; i < sparks; i++) {
        float angle = RandFloat(0, 2.0f * PI);
        float speed = RandFloat(50.0f, 200.0f);
        float life = RandFloat(0.3f, 0.8f);

        /* Hue variation via HSV */
        float h, s, v;
        Vector3 hsv = ColorToHSV(color);
        h = hsv.x + RandFloat(-20, 20);
        s = hsv.y;
        v = hsv.z;
        Color c = ColorFromHSV(h, s, v);

        Particle p = {0};
        p.type = PARTICLE_SPARK;
        p.position = pos;
        p.velocity = (Vector2){ cosf(angle) * speed, sinf(angle) * speed };
        p.life = life;
        p.max_life = life;
        p.color = c;
        p.size = RandFloat(2.0f, 5.0f);
        AddParticle(particles, count, p);
    }

    int debris = GetRandomValue(3, 6);
    for (int i = 0; i < debris; i++) {
        float angle = RandFloat(0, 2.0f * PI);
        float speed = RandFloat(30.0f, 120.0f);
        float life = RandFloat(0.4f, 0.9f);

        Particle p = {0};
        p.type = PARTICLE_DEBRIS;
        p.position = pos;
        p.velocity = (Vector2){ cosf(angle) * speed, sinf(angle) * speed };
        p.life = life;
        p.max_life = life;
        p.color = color;
        p.size = RandFloat(4.0f, 10.0f);
        p.rotation = RandFloat(0, 2.0f * PI);
        p.rot_speed = RandFloat(-8.0f, 8.0f);
        AddParticle(particles, count, p);
    }
}

void EmitThrust(Particle *particles, int *count, Vector2 pos)
{
    for (int i = 0; i < 2; i++) {
        Particle p = {0};
        p.type = PARTICLE_THRUST;
        p.position = (Vector2){ pos.x + RandFloat(-3, 3), pos.y };
        p.velocity = (Vector2){ RandFloat(-10, 10), RandFloat(40, 80) };
        p.life = RandFloat(0.1f, 0.3f);
        p.max_life = p.life;
        p.color = (Color){ 255, (unsigned char)GetRandomValue(150, 220), 50, 255 };
        p.size = RandFloat(2.0f, 4.0f);
        AddParticle(particles, count, p);
    }
}

void EmitScorePopup(Particle *particles, int *count, Vector2 pos, int score)
{
    Particle p = {0};
    p.type = PARTICLE_SCORE_POPUP;
    p.position = pos;
    p.velocity = (Vector2){ 0, -40.0f };
    p.life = 1.0f;
    p.max_life = 1.0f;
    p.score_value = score;
    AddParticle(particles, count, p);
}
