# AGENTS.md

Agent guidance for working in this repository.

## Project

- Game: modernized Galaxian in C with raylib
- Rendering: primitive shapes only, no sprites, no loaded textures, no audio
- Build system: CMake
- Dependency model: raylib is vendored under `vendor/raylib`
- Design reference: `SPEC.md`

## Repository Layout

- `CMakeLists.txt`: single executable target and platform-specific build settings
- `src/main.c`: window setup, render texture pipeline, main loop
- `src/config.h`: global constants, pool sizes, tuning values, colors, helpers
- `src/game.h` and `src/game.c`: top-level game state, screen flow, update/draw dispatch
- `src/player.*`: player movement, firing, respawn logic
- `src/enemy.*`: enemy state and behavior
- `src/formation.*`: formation layout, sway, dive initiation
- `src/path.*`: Bezier dive paths and return paths
- `src/bullet.*`: player and enemy bullet pools
- `src/particle.*`: explosions, thrust, score popups
- `src/starfield.*`: scrolling background stars
- `src/effects.*`: screen shake and flash effects
- `src/draw_utils.*`: reusable neon drawing helpers and ship shapes
- `src/collision.*`: collision checks and hit resolution
- `src/ui.*`: HUD and screen overlays
- `vendor/raylib/`: third-party code, do not touch unless explicitly required

## Core Constraints

- Keep visuals primitive-only. `RenderTexture2D` is allowed for the virtual-resolution pipeline and post-processing, but do not add image assets or sprite loading.
- Do not add audio or sound unless the task explicitly changes the project constraints.
- Prefer edits in `src/` and the top-level `CMakeLists.txt`.
- Do not modify `vendor/raylib/` unless dependency work is explicitly requested.
- Keep the build warning-clean under the configured compiler flags.

## Build Workflow

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build
```

Release build:

```powershell
cmake --build build --config Release
```

Run on Windows:

```powershell
.\build\galaxian\galaxian.exe
```

If using a Visual Studio multi-config generator, the executable may instead be under:

```powershell
.\build\galaxian\Debug\galaxian.exe
```

After code changes, rebuild. If you change `CMakeLists.txt` or add source files, re-run configure first.

## Build-System Notes

- This is a single-target CMake project.
- New `.c` files must be added manually to `add_executable(...)` in `CMakeLists.txt`.
- Cross-platform support exists for Windows, macOS, and Web. Keep platform-specific changes isolated to the existing CMake branches when possible.
- The Web branch currently references `resources`; keep that aligned with the real project layout if assets or web packaging are introduced.

## Architecture Notes

- All gameplay coordinates should use the virtual resolution from `config.h`, not raw window coordinates.
- The game renders to a `RenderTexture2D` and scales that texture to the window in `main.c`.
- Most state is centralized in the `Game` struct in `src/game.h`. Systems receive `Game *` and mutate state directly.
- The project uses fixed-size arrays and pool limits from `config.h`. Avoid dynamic allocation unless the task clearly requires it.
- Headers use forward declarations where needed to avoid circular includes. Preserve that pattern.

## Expected Update/Draw Order

Keep logic consistent with the current architecture and `SPEC.md`:

1. Effects
2. Starfield
3. Player input and movement
4. Formation update and dive initiation
5. Enemy state updates
6. Bullet updates
7. Particle updates
8. Collision resolution
9. Stage clear and game-over handling

Draw order should keep shake/background/gameplay/HUD layering intact.

## Style And Implementation Conventions

- Use C99 and existing project conventions.
- Keep constants and tunables in `src/config.h`.
- Use delta-time-based motion and animation.
- Prefer small, direct functions over introducing extra abstraction layers.
- Keep comments sparse and useful.
- Match the existing naming style and file organization.

## Verification

- Minimum check after code changes: `cmake --build build`
- If behavior, rendering, or timing changes, run the game when practical and confirm it starts cleanly.
- If you add or move source files, verify CMake still configures successfully.

## Related Docs

- `SPEC.md`: gameplay design, scoring, difficulty, architecture intent
- `CLAUDE.md`: parallel project guidance already written for another coding agent
