# AGENTS.md

## Purpose

This repository is a modular Raylib + CMake Defender remake in C. Keep changes small, readable, and easy to build on Windows first. Preserve the current primitive-only art direction and avoid turning the codebase back into a monolithic prototype.

## Repository Layout

- `src/main.c`: window bootstrap and fixed-step main loop
- `src/game.c`, `src/game.h`: top-level game flow, state transitions, scoring, wave progression
- `src/input.c`, `src/input.h`: keyboard/gamepad action mapping
- `src/world.c`, `src/world.h`: wraparound world and terrain helpers
- `src/player.c`, `src/player.h`: player movement, weapons, rescue interactions
- `src/enemy.c`, `src/enemy.h`: enemy behaviors and collision pressure
- `src/wave.c`, `src/wave.h`: wave composition and spawning
- `src/effects.c`, `src/effects.h`: particles, palette, camera shake/follow
- `src/render.c`, `src/render.h`: primitive-only world rendering, HUD, radar, menus
- `src/game_types.h`: shared constants, enums, and structs
- `SPEC.md`: product and architecture spec for the current game target
- `CMakeLists.txt`: top-level build configuration
- `vendor/raylib`: vendored dependency; do not modify unless explicitly asked
- `build/`: local build output

## Working Agreement

- Prefer gameplay and rendering changes in `src/`
- Keep responsibilities in the existing modules instead of piling logic back into `main.c`
- Update `CMakeLists.txt` only when the build actually needs to change
- Avoid introducing new dependencies unless requested
- Do not edit files under `vendor/` unless the task specifically requires it
- Preserve the current straightforward C style and explicit data flow
- Favor fixed-capacity arrays/pools and stack/local data over heap allocation during gameplay

## Build And Verify

Use the existing CMake flow from the repository root.

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build --config Debug
```

Run:

```powershell
.\build\defender\Debug\defender.exe
```

If the build directory was generated from another repository or generator, delete `build/` and reconfigure before rebuilding.

If the user wants the Visual Studio generator explicitly, this repo currently builds with:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev
cmake --build build --config Debug
```

## Code Guidelines

- Stick to straightforward C and the existing brace/style conventions
- Keep the fixed-step loop easy to scan: input, update, and draw should stay clearly separated
- Keep gameplay rules explicit rather than hidden in rendering code
- Prefer Raylib APIs directly over adding abstraction layers too early
- Keep warning output clean under the current compiler flags
- Add comments only when they clarify non-obvious behavior

## Gameplay And Rendering Constraints

- Primitive-only visuals: use Raylib shapes, lines, circles, triangles, rectangles, and built-in text drawing
- No sprites, imported textures, render textures, shaders, audio, music, or sound effects unless explicitly requested
- No runtime asset pipeline is assumed
- Preserve horizontal wraparound gameplay and the current radar/HUD readability
- Keep game state behavior deterministic under the fixed 60 Hz simulation

## Assets And Platform Notes

- The current project assumes no runtime assets
- If assets are added later, update the build and working-directory assumptions accordingly
- The web build section still references preloading `resources`; only rely on that after intentionally adding the directory
- Windows desktop is the primary supported target today; treat other platforms as follow-up unless the task says otherwise

## Scope Safety

- Do not remove or rewrite unrelated user changes
- Favor minimal patches over broad cleanup
- Preserve the current modular architecture unless the user explicitly asks for a refactor
- When reviewing or optimizing, prioritize gameplay correctness, warning cleanliness, and hot-path simplicity
