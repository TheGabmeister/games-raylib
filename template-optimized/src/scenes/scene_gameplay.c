#include "scenes/scene_gameplay.h"
#include "subsystems/renderer.h"
#include "subsystems/input.h"
#include "subsystems/camera.h"
#include "subsystems/collision.h"
#include "subsystems/debug.h"
#include "core/pool.h"
#include "core/event.h"
#include "entities/ship.h"
#include "entities/asteroid.h"
#include "entities/bullet.h"
#include "entities/particle.h"
#include <stdlib.h>
#include <stdio.h>

// Collision layers (bitmask)
#define COL_PLAYER     (1u << 0)
#define COL_ASTEROID   (1u << 1)
#define COL_PROJECTILE (1u << 2)

// Entity tags for collision results
enum { TAG_SHIP, TAG_BULLET, TAG_ASTEROID };

// Game events
enum { EVT_ASTEROID_HIT, EVT_SHIP_HIT };

typedef struct {
    Renderer *renderer;
    Input    *input;
    Camera   *camera;
    Debug    *debug;

    Ship     ship;
    Asteroid asteroids[MAX_ASTEROIDS];
    Bullet   bullets[MAX_BULLETS];
    Particle particles[MAX_PARTICLES];

    CollisionWorld collision;
    EventQueue     events;

    int   score;
    int   wave;
    float wave_timer;
} GameplayData;

static void update(Scene *self, float dt) {
    GameplayData *gp = self->data;

    camera_update(gp->camera, dt);
    ship_update(&gp->ship, gp->input, dt);

    if (gp->ship.alive && gp->ship.fire_cooldown <= 0 &&
        input_held(gp->input, ACTION_FIRE)) {
        bullet_fire(gp->bullets, MAX_BULLETS, ship_nose(&gp->ship), gp->ship.rotation);
        gp->ship.fire_cooldown = SHIP_FIRE_RATE;
    }

    bullets_update(gp->bullets, MAX_BULLETS, dt);
    asteroids_update(gp->asteroids, MAX_ASTEROIDS, dt);
    particles_update(gp->particles, MAX_PARTICLES, dt);

    // --- Collision detection ---
    collision_world_clear(&gp->collision);

    if (gp->ship.alive && gp->ship.invuln_timer <= 0)
        collision_world_add(&gp->collision, gp->ship.position, ship_radius(),
                            COL_PLAYER, COL_ASTEROID, TAG_SHIP, 0);

    POOL_EACH(gp->bullets, MAX_BULLETS, bi) {
        collision_world_add(&gp->collision, gp->bullets[bi].position, BULLET_RADIUS,
                            COL_PROJECTILE, COL_ASTEROID, TAG_BULLET, bi);
    }

    POOL_EACH(gp->asteroids, MAX_ASTEROIDS, ai) {
        collision_world_add(&gp->collision, gp->asteroids[ai].position,
                            gp->asteroids[ai].radius,
                            COL_ASTEROID, COL_PLAYER | COL_PROJECTILE, TAG_ASTEROID, ai);
    }

    CollisionPair pairs[MAX_COLLISION_PAIRS];
    int pair_count = collision_world_check(&gp->collision, pairs, MAX_COLLISION_PAIRS);

    // --- Detect → push events (no side effects beyond deactivating bullets) ---
    event_queue_clear(&gp->events);

    for (int i = 0; i < pair_count; i++) {
        int bullet_idx = -1, asteroid_idx = -1;
        bool ship_hit = false;

        if (pairs[i].tag_a == TAG_BULLET)   bullet_idx   = pairs[i].index_a;
        if (pairs[i].tag_b == TAG_BULLET)   bullet_idx   = pairs[i].index_b;
        if (pairs[i].tag_a == TAG_ASTEROID) asteroid_idx  = pairs[i].index_a;
        if (pairs[i].tag_b == TAG_ASTEROID) asteroid_idx  = pairs[i].index_b;
        if (pairs[i].tag_a == TAG_SHIP || pairs[i].tag_b == TAG_SHIP) ship_hit = true;

        if (bullet_idx >= 0 && asteroid_idx >= 0 &&
            gp->bullets[bullet_idx].active && gp->asteroids[asteroid_idx].active) {
            POOL_FREE(gp->bullets, bullet_idx);
            event_queue_push(&gp->events, (Event){
                .type = EVT_ASTEROID_HIT,
                .pos  = gp->asteroids[asteroid_idx].position,
                .a    = gp->asteroids[asteroid_idx].size,
                .b    = asteroid_idx,
            });
        }

        if (ship_hit && asteroid_idx >= 0 &&
            gp->asteroids[asteroid_idx].active && gp->ship.alive) {
            event_queue_push(&gp->events, (Event){
                .type = EVT_SHIP_HIT,
                .pos  = gp->ship.position,
            });
        }
    }

    // --- Process events (scoring, effects, destruction) ---
    for (int i = 0; i < gp->events.count; i++) {
        Event *e = &gp->events.events[i];
        switch (e->type) {
            case EVT_ASTEROID_HIT: {
                AsteroidSize size = (AsteroidSize)e->a;
                gp->score += (size == SIZE_LARGE ? 20 : size == SIZE_MEDIUM ? 50 : 100);

                int pcount = size == SIZE_LARGE ? 25 : size == SIZE_MEDIUM ? 14 : 7;
                particle_spawn_burst(gp->particles, MAX_PARTICLES,
                                     e->pos, pcount, 80.0f, 1.0f, CLR_LGRAY);

                if (size == SIZE_LARGE)       camera_shake(gp->camera, 0.3f, 8.0f);
                else if (size == SIZE_MEDIUM) camera_shake(gp->camera, 0.15f, 4.0f);

                asteroid_destroy(gp->asteroids, MAX_ASTEROIDS, e->b);
                break;
            }
            case EVT_SHIP_HIT:
                particle_spawn_burst(gp->particles, MAX_PARTICLES,
                                     e->pos, 20, 100.0f, 1.0f, CLR_GREEN);
                camera_shake(gp->camera, 0.4f, 12.0f);
                ship_kill(&gp->ship);
                break;
        }
    }

    // Wave progression
    if (asteroids_count_active(gp->asteroids, MAX_ASTEROIDS) == 0) {
        gp->wave_timer += dt;
        if (gp->wave_timer >= 2.0f) {
            gp->wave++;
            asteroid_spawn_wave(gp->asteroids, MAX_ASTEROIDS, gp->wave);
            gp->wave_timer = 0;
        }
    }

    // Game over
    if (gp->ship.lives <= 0 && !gp->ship.alive) {
        self->final_score = gp->score;
        self->next = SCENE_GAMEOVER;
    }

    // Debug stats
    if (gp->debug && gp->debug->active) {
        int bc = 0, ac = 0, pc = 0;
        POOL_EACH(gp->bullets, MAX_BULLETS, _b)   { bc++; }
        POOL_EACH(gp->asteroids, MAX_ASTEROIDS, _a) { ac++; }
        POOL_EACH(gp->particles, MAX_PARTICLES, _p) { pc++; }
        debug_stat(gp->debug, "Bullets", bc);
        debug_stat(gp->debug, "Asteroids", ac);
        debug_stat(gp->debug, "Particles", pc);
        debug_stat(gp->debug, "Col Bodies", gp->collision.count);
    }
}

static void draw(Scene *self) {
    GameplayData *gp = self->data;

    renderer_set_camera(gp->renderer, camera_get_offset(gp->camera));

    asteroids_draw(gp->asteroids, MAX_ASTEROIDS, gp->renderer);
    bullets_draw(gp->bullets, MAX_BULLETS, gp->renderer);
    particles_draw(gp->particles, MAX_PARTICLES, gp->renderer);
    ship_draw(&gp->ship, gp->renderer);

    debug_draw_collisions(gp->debug, gp->renderer, &gp->collision);

    char buf[64];
    snprintf(buf, sizeof(buf), "SCORE: %d", gp->score);
    renderer_push_text(gp->renderer, LAYER_UI, (Vec2){10, 10}, buf, 20, CLR_WHITE);

    snprintf(buf, sizeof(buf), "LIVES: %d", gp->ship.lives);
    renderer_push_text(gp->renderer, LAYER_UI, (Vec2){10, 35}, buf, 20, CLR_WHITE);

    snprintf(buf, sizeof(buf), "WAVE: %d", gp->wave);
    renderer_push_text(gp->renderer, LAYER_UI, (Vec2){10, 60}, buf, 20, CLR_WHITE);
}

static void cleanup(Scene *self) {
    free(self->data);
}

Scene scene_gameplay_create(Renderer *r, Input *inp, Camera *cam, Debug *debug) {
    GameplayData *gp = calloc(1, sizeof(GameplayData));
    gp->renderer = r;
    gp->input    = inp;
    gp->camera   = cam;
    gp->debug    = debug;
    gp->wave     = 1;

    ship_init(&gp->ship);
    asteroid_spawn_wave(gp->asteroids, MAX_ASTEROIDS, gp->wave);

    return (Scene){
        .data    = gp,
        .update  = update,
        .draw    = draw,
        .cleanup = cleanup,
        .next    = SCENE_NONE,
    };
}
