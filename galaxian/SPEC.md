# SPEC.md — Galaxian (Modernized)

## 1. Overview

A modernized 2D recreation of Galaxian (1979) in C using raylib. All visuals use primitive shapes only — no sprites, no loaded textures, no audio. `RenderTexture2D` is used internally for the virtual resolution pipeline and optional bloom, but no image files are loaded. The visual style is **neon vector** (inspired by Geometry Wars): bright outlines on black, additive-blended glows, particle trails, and optional bloom.

**Window size:** 720x960 (scales the virtual resolution cleanly at 1.5x).

**Virtual resolution:** 480x640 (3:4 portrait, 2x the original 240x320). Rendered to a `RenderTexture2D`, then scaled to the window with letterboxing so all coordinate math is resolution-independent.

## 2. Visual Design

### Neon Three-Pass Technique

Every entity is drawn with three layers:

1. **Glow** — `BLEND_ADDITIVE`, `DrawCircleGradient` at 2-3x entity size, entity color at ~15% alpha
2. **Fill** — entity-sized filled shape at ~40% alpha
3. **Edge** — `BLEND_ALPHA`, outline at full brightness (`DrawTriangleLines`, `DrawRectangleLinesEx`, etc.)

### Neon Color Palette

| Entity | Color |
|---|---|
| Player | Cyan-green `(0, 255, 200)` |
| Blue enemy | Electric blue `(50, 120, 255)` |
| Purple enemy | Vivid purple `(180, 80, 255)` |
| Red enemy | Hot red `(255, 60, 60)` |
| Flagship | Gold `(255, 220, 50)` |
| Player bullet | Bright cyan `(100, 255, 255)` |
| Enemy bullet | Yellow `(255, 255, 100)` |
| HUD text | Soft blue-white `(200, 200, 255)` |

### Entity Shapes (all from primitives)

- **Player ship** — upward triangle (body) + two smaller wing triangles + rectangular engine block. Pulsing `DrawCircleGradient` at engine nozzle for thrust glow.
- **Blue enemy** — downward chevron (two triangles forming a V). Small, simple.
- **Purple enemy** — diamond/kite (four triangles, wider body).
- **Red enemy** — angular swept wings (triangle body + two rotated wing triangles).
- **Flagship** — largest shape: `DrawRectangleRounded` body + side nacelle rectangles + dome via `DrawCircleSector`. Gold color.
- **Bullets** — small elongated gradient circles along velocity direction for a trail effect.

## 3. Gameplay Mechanics

### Player

- Left/right movement only, clamped to screen bounds
- **Single bullet at a time** — must wait for previous shot to hit or exit screen
- Bullet speed: traverses full screen in ~0.4 seconds
- **3 starting lives**, bonus life awarded at 7000 points (one-time)
- On death: brief explosion, then respawn after ~1.5 seconds with ~2 seconds of invincibility (ship blinks). Formation and remaining enemies persist through deaths.
- **Game over** when all lives are lost
- **Input:** Arrow keys or A/D for movement, Space to fire

### Formation

- **46 enemies** arranged in a 10-column, 6-row grid:
  - Row 0: 2 Flagships (centered, columns 4-5)
  - Row 1: 6 Red (columns 2-7)
  - Row 2: 8 Purple (columns 1-8)
  - Rows 3-5: 10 Blue each (30 total, columns 0-9)
- **Stage entry animation:** enemies fly in from the top of the screen in groups and settle into their formation slots before gameplay begins
- Formation sways horizontally (sinusoidal oscillation)
- Sway speed and amplitude increase with difficulty

### Diving AI

- A cooldown timer triggers dives; when it fires, an enemy is selected to break formation
- **Max concurrent divers:** 2-4 (scales with difficulty), not counting returning enemies
- Priority: flagships first, then lower-row enemies weighted higher
- Flagship dives are escorted by up to 2 adjacent Red enemies (escorts don't count toward diver limit)
- **Diving enemies kill the player on direct body contact** (kamikaze). The enemy is also destroyed (no points awarded for kamikaze).
- Enemies that exit the screen bottom wrap to the top and return to their formation slot

### Dive Paths

- **Piecewise cubic Bezier curves** (evaluated via `GetSplinePointBezierCubic`)
- Defined in formation-relative coordinates (one template works from any column)
- Path templates:
  - `SWOOP_LEFT` — 3-segment S-curve sweeping left then down
  - `SWOOP_RIGHT` — mirror
  - `LOOP` — 4-segment loop-de-loop then downward plunge
  - `FLAGSHIP_CENTER` — wider, slower, majestic arc
  - `RETURN_ARC` — dynamically generated: smooth Bezier from current position back to formation slot
- Enemy rotation derived from path tangent (finite difference)
- Diving enemies fire bullets aimed at player's X position with slight randomness

### Scoring

| Enemy | In Formation | Diving |
|---|---|---|
| Blue | 30 | 60 |
| Purple | 40 | 80 |
| Red | 50 | 100 |
| Flagship (diving alone, no escorts) | 60 | 150 |
| Flagship (diving with 1 escort) | — | 200 |
| Flagship (diving with 2 escorts) | — | 300 |
| Flagship (escorts killed during same dive, then flagship killed) | — | 800 |

Score popups float upward and fade out over ~1 second.

### Stage Progression

- All enemies destroyed → next stage
- Stage counter displayed as flags at screen bottom (up to 48, then loops)
- Difficulty scales linearly from stage 1 to 48 (then caps)

## 4. Difficulty Scaling (Stage 1 → 48)

| Parameter | Stage 1 | Stage 48 |
|---|---|---|
| Formation sway speed | 1.0 | 2.5 |
| Formation sway amplitude | 30px | 50px |
| Dive cooldown | 3.0s | 1.0s |
| Max concurrent divers | 2 | 4 |
| Aggression | 0.3 | 1.0 |
| Enemy shot frequency | 0.5/s | 2.5/s |
| Dive speed multiplier | 1.0x | 1.8x |

## 5. Effects

### Particles

- **Explosion** — 20-40 sparks per enemy kill. Random directions, speed 50-200 px/s, life 0.3-0.8s. Color matches enemy type with hue variation via `ColorFromHSV`. Drawn as `DrawCircleGradient` with `BLEND_ADDITIVE`. 3-6 larger debris particles as rotating rectangles (`DrawRectanglePro`).
- **Thrust** — 1-2 particles/frame at engine position. Downward velocity, orange-yellow, life 0.1-0.3s.
- **Score popup** — `DrawText` at particle position, floats up, alpha fades with life.

### Screen Shake

- Triggered on explosions. Random offset per frame, intensity decays over duration.
- Applied via `rlTranslatef` before all game drawing.

### Screen Flash

- White overlay on player death. Alpha fades from 1.0 to 0.0.

### Bloom (Optional Polish)

- Render scene to `RenderTexture2D`, downsample to 1/4 resolution, apply blur shader, composite with `BLEND_ADDITIVE`.
- **Fallback** (no shaders): redraw bright entities (bullets, explosions) as larger/faded copies with `BLEND_ADDITIVE`.

### Starfield

- 200 stars across 3 parallax speed layers. Drawn as tiny `DrawCircleGradient` (radius 0.5-2.0). Slight color variation (white, light blue, light yellow). Wrap vertically.

## 6. Architecture

### File Structure

```
src/
  main.c           — Entry point, window, render texture pipeline, master loop
  config.h         — All constants (dimensions, speeds, counts, colors)
  game.h/game.c    — Game struct, init/update/draw dispatch, screen state machine
  player.h/.c      — Player state, input, movement, drawing
  enemy.h/.c       — Enemy struct/types, state machine, update, drawing
  formation.h/.c   — Formation grid, sway, dive initiation, slot positions
  bullet.h/.c      — Bullet pools (player + enemy), fire/update/draw
  particle.h/.c    — Particle system, emitters (explosion, thrust, score popup)
  starfield.h/.c   — Parallax scrolling stars
  effects.h/.c     — Screen shake, flash, bloom post-processing
  draw_utils.h/.c  — Neon shape primitives, per-type ship drawing functions
  path.h/.c        — Dive path Bezier library, evaluation, dynamic return paths
  collision.h/.c   — Circle-based collision checks between entity groups
  ui.h/.c          — HUD, title screen, game over screen, stage flags
```

### Timing

All movement and animation uses delta-time (`GetFrameTime()`) for frame-rate independence. Target is 60 FPS via `SetTargetFPS(60)`.

### Screen State Machine

```
SCREEN_TITLE → SCREEN_GAMEPLAY → SCREEN_GAMEOVER → (restart) SCREEN_TITLE
```

### Update Order (gameplay)

1. Effects (decay shake/flash)
2. Starfield
3. Player input + movement
4. Formation sway + dive initiation
5. Enemy state machine (formation/diving/returning)
6. Bullets (player + enemy)
7. Particles
8. Collision checks → scoring, deaths, effects
9. Stage clear / game over checks

### Draw Order (gameplay)

1. `rlTranslatef` for screen shake
2. Starfield (background)
3. Bullets
4. Enemies
5. Player
6. Particles (`BLEND_ADDITIVE`, drawn on top)
7. HUD
8. Flash overlay

### Collision

- All checks use `CheckCollisionCircles` with implicit collision radii per entity type. Simpler and more forgiving than rectangle collision.

## 7. Implementation Order

Each step produces a runnable, visually verifiable result:

1. **Window + render pipeline** — `main.c`, `config.h`, `game.h/.c`. Black 480x640 scene scaled to window with letterboxing.
2. **Starfield** — Scrolling parallax stars. Immediate visual life.
3. **Neon drawing utils** — Three-pass glow technique. Test with a static triangle.
4. **Player** — Movement, ship drawing, thrust glow animation.
5. **Player bullet** — Single-bullet firing, upward movement, glow trail.
6. **Particles** — Thrust particles first, then explosion emitter.
7. **Enemies + formation** — Grid layout, sway animation, all 4 enemy type drawings.
8. **Dive paths** — Bezier library, path evaluation. Debug: draw curves on screen.
9. **Diving AI** — Timer-based dive triggers, flagship escorts, state machine.
10. **Enemy bullets** — Aimed at player during dives.
11. **Collision + scoring** — Kill detection, explosions, score popups, flagship bonuses.
12. **HUD + UI** — Score, lives, stage flags.
13. **Effects** — Screen shake, flash.
14. **Title + game over screens**
15. **Bloom** (optional polish)
16. **Difficulty tuning** — Balance across 48 stages.
