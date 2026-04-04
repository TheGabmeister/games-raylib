# AGENTS.md

## Purpose

This repository is a minimal Raylib + CMake template in C. Keep changes small, readable, and easy to build on Windows first.

## Repository Layout

- `src/main.c`: main application entry point and game loop
- `CMakeLists.txt`: top-level build configuration
- `vendor/raylib`: vendored dependency; do not modify unless explicitly asked
- `build/`: local build output

## Working Agreement

- Prefer making gameplay and rendering changes in `src/`
- Update `CMakeLists.txt` only when the build actually needs to change
- Avoid introducing new dependencies unless requested
- Do not edit files under `vendor/` unless the task specifically requires it
- Preserve the current simple style unless the user asks for a broader refactor

## Build And Verify

Use the existing CMake flow from the repository root.

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build
```

If the user wants the Visual Studio generator explicitly, this repo has previously used:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev
```

## Code Guidelines

- Stick to straightforward C and the existing brace/style conventions in `src/main.c`
- Keep the main loop easy to scan: input/update/draw should stay clear and separated
- Prefer Raylib APIs directly over building abstractions too early
- Keep warning output clean under the current compiler flags
- Add comments only when they clarify non-obvious behavior

## Assets And Platform Notes

- The template currently assumes primitive-only rendering and no runtime assets
- If you add assets, update the build and working-directory assumptions accordingly
- The web build section currently preloads `resources`; only rely on that after adding the directory intentionally

## Scope Safety

- Do not remove or rewrite unrelated user changes
- Favor minimal patches over broad cleanup
- When in doubt, keep the template easy for a new project to copy and extend
