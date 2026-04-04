# AGENTS.md

This file gives coding agents the minimum project context needed to work safely in this repository.

## Project Summary

- Project type: small C game prototype using raylib
- Build system: CMake
- Dependency model: raylib is vendored under `vendor/raylib`
- Current app shape: a minimal single-window program in `src/main.c`

## Repository Layout

- `CMakeLists.txt`: top-level build configuration
- `src/`: game source files
- `vendor/raylib/`: third-party dependency source, treat as vendored code
- `build/`: out-of-source build directory

## Working Rules

- Prefer changing code in `src/` and the top-level `CMakeLists.txt`.
- Do not modify files under `vendor/raylib/` unless the task explicitly requires dependency changes.
- If you add a new `.c` or `.h` file that should be compiled, update `CMakeLists.txt` accordingly. The current target only lists `src/main.c`.
- Keep changes small and consistent with the existing plain C style.
- Preserve warning-clean builds under the configured compiler flags.

## Build And Run

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build
```

Run on Windows:

```powershell
.\build\galaxian\galaxian.exe
```

## Verification Expectations

- After code changes, run `cmake --build build`.
- If gameplay logic or rendering changes, run the executable when practical and confirm it starts without errors.
- If you change build structure, re-run CMake configure before building.

## Notes

- There are currently no runtime assets/resources required for the native build.
- `compile_commands.json` is gitignored and is only for local editor/LSP tooling.
- The Web-specific CMake branch references `resources`; if assets are introduced later, keep that section aligned with the actual project layout.
