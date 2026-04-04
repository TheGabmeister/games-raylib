# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (first time or after CMakeLists.txt changes)
cmake -S . -B build

# Build (debug)
cmake --build build

# Build (release)
cmake --build build --config Release

# Run (Windows)
build/galaxian/Debug/galaxian.exe
```

After code changes, always rebuild. If build structure changes (new files, CMakeLists.txt edits), re-run cmake configure first.

## Project Overview

Galaxian is a modernized 2D recreation of Galaxian (1979) in C using raylib. See `SPEC.md` for the full game design specification, including gameplay mechanics, scoring, difficulty scaling, and implementation order.

**Hard constraints:**
- Primitives only — no sprites, no loaded textures, no audio/sound
- `RenderTexture2D` is allowed for the virtual resolution pipeline and bloom, but no image files are loaded
- Do not modify files under `vendor/raylib/` unless the task explicitly requires dependency changes

## Architecture

Single-target CMake project. **New `.c`/`.h` files must be added to the source list in `CMakeLists.txt` manually.** raylib is vendored in `vendor/raylib/` and linked statically.

Cross-platform targets: Windows (MSVC primary), macOS (framework linking), and Web (Emscripten).

### Virtual Resolution

The game runs at 480x640 virtual resolution rendered to a `RenderTexture2D`, then scaled to the 720x960 window. All gameplay coordinates use virtual resolution — never use window coordinates directly. The render texture Y-flip is handled in `main.c` via negative source height in `DrawTexturePro`.

### Central Game Struct

All game state lives in a single `Game` struct (`game.h`) which owns inline arrays for enemies (46), particles (512), stars (200), and bullet pools. Systems receive `Game*` and mutate directly — there is no event system. Headers that need `Game*` in function signatures use `struct Game;` forward declarations to avoid circular includes.

### Update/Draw Order

The update order in `game.c:GameUpdate` matches `SPEC.md` section 6 to avoid stale-state bugs:
1. Effects → 2. Starfield → 3. Player input → 4. Formation (sway + dive initiation) → 5. Enemy state machines → 6. Bullets → 7. Particles → 8. Collisions → 9. Stage/game-over checks

Draw order layers: shake transform → starfield → bullets → enemies → player → particles (additive) → HUD (after shake reset) → flash overlay.

### Enemy State Machine

Each enemy cycles through: `ENTERING` → `IN_FORMATION` → `DIVING` → `RETURNING` → `IN_FORMATION`. The `formation.c` manages dive initiation (timer-based, priority-weighted selection, flagship escort assignment). Dive movement uses piecewise cubic Bezier paths (`path.c`) with 4 templates; return paths are generated dynamically.

### Visual Style

All entities use a three-pass neon drawing technique (glow → fill → edge) with additive blending, implemented in `draw_utils.c`. Enemy shapes are drawn from rotated vertex offsets — use `RotatePoint()` for rotation support. `DrawCircleGradient` with `BLEND_ADDITIVE` produces all glow effects. Avoid nesting `BeginBlendMode`/`EndBlendMode` — exit additive mode before drawing text or UI elements.

### Difficulty Scaling

`GameDifficulty(stage)` returns a 0.0–1.0 `t` value linearly interpolated from stage 1 to 48. All tunable parameters (sway, dive cooldown, shot frequency, etc.) are defined as `_MIN`/`_MAX` pairs in `config.h` and interpolated with `Lerpf(min, max, t)`.

### Player Lifecycle

Player death/respawn is split across two files: `collision.c` sets `alive = false` and conditionally sets `respawn_timer` (only when `lives > 0`), while `player.c:PlayerUpdate` only triggers respawn when `respawn_timer > 0`. This two-site coordination is intentional — `PlayerUpdate` has no access to `lives`, so the collision layer gates whether respawn is even possible.

## Code Style

- C99 with `stdbool.h`
- MSVC builds use `/W4`, GCC/Clang use `-Wall -Wextra -Wpedantic` — preserve warning-clean builds
- Delta-time based movement via `GetFrameTime()`, target 60 FPS
- Fixed-size arrays with `#define` pool limits in `config.h` — no dynamic allocation
- Swap-with-last-element removal for particle and bullet pools
