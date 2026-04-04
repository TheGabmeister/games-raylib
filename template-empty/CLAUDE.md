# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Empty raylib game template in C. Single-file project (`src/main.c`) using raylib (vendored in `vendor/raylib/`) for graphics/input.

## Build Commands

```bash
# Configure (run once, from project root)
cmake -B build

# Build
cmake --build build

# Run (executable outputs to build/template_empty/)
./build/template_empty/template_empty
```

For Visual Studio generators on Windows, specify config at build time:
```bash
cmake --build build --config Debug
# Executable at build/template_empty/Debug/template_empty.exe
```

## Architecture

- **Build system**: CMake (3.24+). Root `CMakeLists.txt` adds raylib as a subdirectory and links it.
- **Source**: `src/main.c` — single entry point. Standard raylib game loop: `InitWindow` -> `while(!WindowShouldClose())` -> `CloseWindow`.
- **Vendor**: `vendor/raylib/` contains the full raylib source, built as part of the CMake project.
- **Compiler warnings**: `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC) are enabled.
- **Web support**: Set `-DPLATFORM=Web` with Emscripten toolchain for WebAssembly builds.
