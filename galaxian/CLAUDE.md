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
```

The executable outputs to `build/galaxian/Debug/galaxian.exe` (or `Release/`).

## Project Overview

Galaxian is a C game built with raylib. All game source lives in `src/`, raylib is vendored in `vendor/raylib/` and built as a CMake subdirectory.

## Architecture

Single-target CMake project. Source files and headers are listed explicitly in CMakeLists.txt — new files must be added there. raylib is linked statically.

Cross-platform targets: Windows (MSVC primary), macOS (framework linking), and Web (Emscripten with `--preload-file resources`).

## Code Style

- C99
- MSVC builds use `/W4`, GCC/Clang use `-Wall -Wextra -Wpedantic`
