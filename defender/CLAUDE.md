# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Primitive-only Defender (1981) remake in C using raylib. All visuals are drawn with lines, triangles, circles, and rectangles — no imported sprites, textures, shaders, render textures, or audio. Windows-first, single-player arcade game.

Full game spec is in `SPEC.md`.

## Build Commands

```bash
# Configure (run once, from project root)
cmake -B build

# Build
cmake --build build

# Run
./build/defender/defender
```

For Visual Studio generators on Windows, specify config at build time:
```bash
cmake --build build --config Debug
# Executable at build/defender/Debug/defender.exe
```

## Architecture

**Build system**: CMake (3.24+). Root `CMakeLists.txt` adds vendored raylib as a subdirectory and links it. Compiler warnings: `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC).

**Module layout** — all source lives in `src/`, split by responsibility:

- `main.c` — Window init, fixed-timestep loop (60 Hz simulation), high-level call to `GameUpdate`/`GameDraw`
- `game.c/h` — Owns the `Game` struct (all game state). Orchestrates mode transitions (title/playing/paused/wave clear/game over), collision resolution, spawning helpers, scoring, respawn logic
- `game_types.h` — All shared types, enums, constants, and inline math helpers. Central header included by every module. Defines fixed-capacity limits (`MAX_ENEMIES`, `MAX_PROJECTILES`, etc.)
- `input.c/h` — Polls keyboard and gamepad, maps to action-based `InputState` (pressed/held/axis)
- `world.c/h` — Terrain heightfield, horizontal world wrap math (`WorldWrapX`, `WorldWrapDeltaX`), safe position finding
- `player.c/h` — Player movement, firing, human pickup/carry/drop, smart bomb and hyperspace activation
- `enemy.c/h` — Per-type enemy AI update (lander abduction, mutant chasing, bomber mines, baiter pursuit, swarmer behavior, pod drifting)
- `wave.c/h` — Authored wave definitions and endless scaling. `SpawnWave` populates enemies and humans for the current wave index
- `effects.c/h` — Particles, camera shake, palette init, thruster particles, burst spawning
- `render.c/h` — All drawing: starfield parallax, terrain, entity silhouettes, projectiles, particles, HUD, radar minimap

**Data flow**: `main.c` runs input -> update -> draw each frame. All game state lives in a single `Game` struct passed by pointer. Fixed-capacity arrays (no heap allocation during gameplay). Modules communicate through `Game*` — no callbacks or global state.

**World model**: 6400-unit wide horizontally wrapping playfield at 1280x720 viewport. Terrain is a sampled heightfield (`TERRAIN_SAMPLE_COUNT` points). All entity positions wrap seamlessly via `WorldWrapX`.

## Constraints

- Do not modify files under `vendor/`
- No imported art assets, texture atlases, shaders, render textures, audio, or runtime asset files
- Only built-in raylib text drawing for HUD/menus
- Modify `CMakeLists.txt` only if the build actually needs to change
- When adding new `.c` files, add them to the `add_executable` list in `CMakeLists.txt`
