# Primitive-Only Modern Defender

## Overview

This project recreates the core feel of Defender (1981) as a 2D side-scrolling arcade action game in C using raylib. The target is a faithful-plus remake: preserve the signature mechanics, pacing, and tension of the original while modernizing readability, visual polish, and code organization.

The game must remain primitive-only. All visuals are built from raylib drawing APIs such as lines, triangles, circles, rectangles, and text. The project must not use sprites, textures, render textures, shaders, audio, music, or sound effects. The game is Windows-first, single-player, and designed to build cleanly through the existing CMake setup.

This document defines the full target game and the implementation architecture expected for the codebase.

## Design Pillars

1. Preserve Defender's identity.
   The player should feel fast, vulnerable, and responsible for protecting humans across a dangerous wraparound battlefield.

2. Modernize readability, not genre.
   Effects, silhouettes, HUD clarity, and camera behavior should improve comprehension without turning the game into a different style of shooter.

3. Keep the codebase small and disciplined.
   Use straightforward C with explicit ownership of data and logic, fixed-capacity containers, and a clean input/update/draw flow.

4. Make primitive visuals look intentional.
   The game should feel stylish through color, layering, trails, flashes, and motion design rather than asset complexity.

## Constraints

- Language: C
- Framework: raylib only, via the vendored dependency already in the repository
- Platform priority: Windows desktop first
- Rendering: primitive-only raylib drawing; no sprites, textures, shaders, or render textures
- Audio: none
- Runtime assets: none
- Networking: none
- Save/load: none
- Scope: one local player
- Build system: keep the existing CMake flow; modify `CMakeLists.txt` only if implementation truly requires it
- Vendor policy: do not modify `vendor/`

## Core Gameplay Loop

The player pilots a fast ship over a horizontally wrapping terrain band while enemies enter, attack, abduct humans, and evolve into more dangerous threats if ignored.

The main gameplay loop is:

1. Start a wave with a set of enemies and a population of humans placed across the terrain.
2. Fly freely through the world using thrust-based movement with immediate combat pressure.
3. Intercept landers before they abduct humans.
4. Rescue falling humans or carry them to safe ground.
5. Destroy enemies to score points and reduce pressure.
6. Survive escalating enemy combinations until all wave objectives are cleared.
7. Advance to the next wave with increased intensity and enemy density.

Failure pressure comes from:

- direct collisions
- enemy fire
- losing too many humans
- allowing landers to escape with abducted humans and create mutants
- poor use of hyperspace and smart bombs

## Player Abilities

The player ship must support the following actions:

- Move left and right with strong responsiveness
- Apply upward thrust to gain altitude
- Apply reverse/brake input for tighter control
- Fire primary shots in the facing direction
- Use a limited smart bomb that clears or heavily damages active threats on screen
- Activate hyperspace for a risky reposition
- Pick up falling or grounded humans by passing through them with valid state
- Carry a rescued human until safely released near the ground

Behavior defaults:

- Movement uses acceleration plus damping rather than tile-like stepping
- The ship has a clear facing direction and readable thrust state
- Primary fire has a short cooldown and modest projectile speed advantage over enemies
- Smart bombs are limited per life or per wave and replenishment rules must be explicit in implementation
- Hyperspace is instant relocation with deliberate risk, such as unsafe arrival or self-damage chance

The implementation should expose these systems through a `Player` type and an `UpdatePlayer` function that consumes `InputState`, mutates the player, and emits gameplay events such as shots fired, human pickup, and special ability use.

## Humans And Rescue Rules

Humans are a central gameplay obligation, not decorative background entities.

Required rules:

- Humans spawn standing on designated terrain zones at wave start
- Landers can target nearby humans and begin abduction
- An abducted human is lifted upward with the attacking lander
- If the abduction completes, that lander transforms into or is replaced by a mutant
- If a human falls, the player can catch and carry them
- If the player drops a carried human onto safe terrain at low enough speed, the human survives
- If a human falls too far or is dropped unsafely, the human dies
- Human survival influences score, end-of-wave results, or both

The implementation should model humans explicitly with a `Human` type and track state such as grounded, abducted, falling, carried, rescued, and dead.

## Enemy Roster

The full classic-inspired enemy lineup should be included in the target spec, even if implementation lands in phases.

Required enemy categories:

- Lander: seeks humans, abducts them, and becomes a major failure trigger
- Mutant: aggressive fast attacker created from failed rescues
- Bomber: slow threat that releases projectile or mine patterns
- Baiter: anti-stall hunter that pressures the player if a wave runs long
- Swarmer: small fast threat created from specific wave events or enemy types
- Pod: durable carrier enemy that splits into swarmers when destroyed

Enemy design rules:

- Each enemy must have a distinct silhouette readable from primitive geometry
- Each enemy must have a distinct movement profile and pressure role
- Waves should mix enemy behaviors rather than only scaling counts
- Enemy spawn logic should be data-driven through a `SpawnDirector` and `WaveState`

The implementation should represent hostiles through an `Enemy` type with an `EnemyType` enum and per-type update behavior inside `UpdateEnemies`.

## Wave Progression

Wave progression should escalate both density and cognitive load while staying fair.

Wave system requirements:

- Waves begin with a defined set of humans and enemy composition
- Early waves prioritize lander rescue pressure and learning space control
- Mid waves add bombers and pods to increase battlefield management
- Later waves introduce baiters sooner and increase mutant risk
- Endless progression is acceptable after a designed sequence of authored early waves

Wave completion should occur when the active hostile requirement is cleared and no critical abduction sequence remains unresolved.

Wave progression should track:

- current wave index
- remaining enemy counts by type
- living humans
- rescue/loss results from the previous wave
- timers for pressure events such as baiter spawns

Implementation should center this in `WaveState` and `SpawnDirector`, with a dedicated `SpawnWave` entry point.

## Controls

The game must support keyboard and gamepad at launch through shared action-based input mapping.

Required gameplay actions:

- Move horizontal
- Thrust
- Reverse/brake
- Fire
- Smart bomb
- Hyperspace
- Pause
- Confirm/start

Keyboard default mapping:

- `A` / `D` or Left / Right for horizontal movement
- `W` / Up for thrust
- `S` / Down for reverse/brake
- `Space` for fire
- `Left Shift` for smart bomb
- `Tab` or `E` for hyperspace
- `Enter` for start/confirm
- `Escape` for pause or exit to title where appropriate

Gamepad default mapping:

- left stick or d-pad for horizontal movement
- south face button for thrust
- west or right trigger for fire
- left shoulder for smart bomb
- right shoulder for hyperspace
- start for pause/confirm

The input system should expose an `InputAction` enum and an `InputState` structure with pressed, held, and analog data normalized for both devices.

## Visual Direction

The game should feel modern through motion and layering while staying entirely primitive-based.

Visual goals:

- Deep dark background with subtle color gradients implied through layered shape fields rather than textures
- Multi-layer starfield with slow parallax
- Stylized terrain silhouette with glowing edge accents
- Player ship rendered as a crisp vector-like silhouette with canopy and engine geometry
- Distinct enemy silhouettes using triangles, lines, circles, and segmented hull shapes
- Bright projectile trails and impact flashes
- Geometric explosion bursts using rings, shards, sparks, and expanding outlines
- Engine exhaust, rescue-beam accents, and shield-hit flashes through alpha-blended primitives
- Readable HUD with score, lives, smart bombs, wave number, and human status
- Compact radar or minimap to support world awareness
- Camera easing and restrained screen shake on impacts and bomb use

Approved effect techniques:

- alpha blending
- layered line and triangle silhouettes
- additive-feeling color stacking achieved through repeated bright primitive passes
- temporal trails via short-lived particle history
- world-space particles and HUD-space flashes
- camera offset impulses and smooth follow interpolation

Disallowed visual techniques:

- imported art assets
- texture atlases
- post-processing shaders
- bloom passes using render textures

The visual system should centralize palette choices in a `RenderPalette` type and camera behavior in a `CameraState` type.

## Technical Architecture

The implementation must move away from a monolithic `src/main.c` and into a lightweight modular C layout. `main.c` should bootstrap the window, own the main loop, and call top-level game functions only.

Recommended module split:

- `main.c`: startup, shutdown, fixed-step loop, high-level mode transitions
- game module: owns `Game`, `GameMode`, lifetime, and orchestration
- input module: device polling and action mapping into `InputState`
- world/simulation module: persistent world state, terrain bounds, wraparound, collisions
- player module: player movement, weapons, carried human logic, cooldowns
- enemy module: enemy update logic, targeting, attacks, transformations
- wave/spawn module: authored wave data, escalation, pressure timers
- particle/effects module: trails, flashes, explosions, smart bomb visuals
- render module: world draw, entity silhouettes, HUD, radar, camera transforms
- shared types/constants module: enums, capacities, gameplay constants, helper structs

Required implementation-facing types:

- `Game`
- `GameMode`
- `InputAction`
- `InputState`
- `WorldState`
- `WaveState`
- `Player`
- `Human`
- `Enemy`
- `EnemyType`
- `Projectile`
- `Particle`
- `CameraState`
- `SpawnDirector`
- `RenderPalette`

Required top-level functions or equivalent entry points:

- `GameInit`
- `GameShutdown`
- `GameUpdate`
- `GameDraw`
- `UpdatePlayer`
- `UpdateEnemies`
- `SpawnWave`
- `RenderWorld`
- `RenderHud`

Runtime rules:

- Simulation target is fixed 60 Hz
- Rendering may interpolate visually if desired, but gameplay state updates on fixed ticks
- Use clear input, update, and draw phases every frame
- Use enum-driven game states such as title, playing, paused, wave clear, and game over
- Use fixed-capacity arrays or pools for enemies, projectiles, humans, and particles
- Avoid heap churn during active gameplay
- Prefer explicit ownership and direct data flow over callback-heavy patterns

Collision and world rules:

- Horizontal world wraps seamlessly for entities and camera presentation
- Vertical space is bounded, with death or recovery rules defined for leaving the play band
- Terrain collision matters for humans and low-altitude ship risk, but terrain should remain implementation-light
- Radar/minimap should represent off-screen threats and humans consistently

## Milestones

### Milestone 1: First Playable Flight And Combat Loop

- Window boots into a title screen
- Player ship moves, thrusts, turns, and fires
- World scrolls horizontally with wraparound
- Basic landers spawn and can be destroyed
- Score, lives, and wave number appear in HUD

### Milestone 2: Rescue And Abduction Loop

- Humans populate the terrain
- Landers abduct humans
- Falling/carrying/rescue behavior works
- Failed rescues produce mutants
- Radar/minimap communicates human danger

### Milestone 3: Full Enemy And Wave Systems

- Bombers, pods, swarmers, baiters, and mutants all function
- Waves escalate through authored early patterns and endless scaling
- Smart bomb and hyperspace rules are complete
- Game over and restart loop are complete

### Milestone 4: Visual Polish And Effects

- Ship and enemy silhouettes are finalized
- Particle trails, explosions, flashes, and camera shake are tuned
- HUD callouts and wave transition presentation are polished
- Busy scenes remain readable

### Milestone 5: Balancing And QA

- Score values, enemy counts, and cooldowns are tuned
- Rescue rules feel fair and learnable
- Keyboard and gamepad parity is verified
- Build warnings remain clean

## Acceptance Criteria

The project is complete for this spec when all of the following are true:

- The game boots into a title flow and transitions cleanly into gameplay
- Keyboard and gamepad both support the full launch action set
- The player can move, fire, use smart bomb, and use hyperspace
- Humans can be abducted, fall, be caught, be carried, be safely dropped, and die from failed rescue
- Lander abductions can produce mutants
- The target enemy roster exists with distinct behavior roles
- Waves escalate and can be completed repeatedly
- HUD updates score, lives, wave status, and human-related information correctly
- Radar or minimap provides readable battlefield awareness
- Primitive-only visuals deliver trails, flashes, explosions, and camera feedback without textures or shaders
- The simulation remains stable at a fixed 60 Hz update model
- The codebase is organized into small modules rather than one large game file
- The project builds cleanly with the current compiler warning settings

## Test Cases And Scenarios

Functional scenarios:

1. From title screen, start a game, play a wave, pause, resume, lose all lives, and return to title.
2. Perform the same core actions with keyboard and gamepad and confirm parity.
3. Allow a lander to abduct a human and verify the sequence completes into mutant pressure.
4. Catch a falling human, carry them, and release them safely onto terrain.
5. Drop or miss a falling human and verify death handling and any score or status impact.
6. Use smart bomb during a dense threat moment and confirm correct clearing behavior and resource depletion.
7. Use hyperspace repeatedly and confirm reposition plus defined risk behavior.
8. Clear a wave and verify next-wave escalation, resets, and carryover state.
9. Stress a visually dense scene and confirm important entities remain readable.

Technical checks:

- No sprites, textures, shaders, render textures, music, or sound APIs are used in gameplay implementation
- No runtime asset files are required
- No code under `vendor/` is modified
- Main loop preserves clear input/update/draw separation
- Fixed-capacity containers do not overflow under expected wave caps

## Defaults And Non-Goals

Implementation defaults:

- Target window: 1280x720 logical design target in 16:9
- Target framerate: 60 FPS with fixed 60 Hz simulation
- Player count: 1
- Modes at launch: title, playing, paused, wave clear, game over
- Rendering style: primitive-only neon sci-fi/vector aesthetic

Non-goals for this version:

- online features
- local multiplayer
- save profiles
- replay system
- configurable key rebinding UI
- campaign narrative
- asset pipeline support

## Assumptions

- "Modernized" means improved clarity, stronger visual feedback, and better onboarding of information while preserving Defender's fundamental identity.
- "No textures" is strict and excludes shader-based or render-texture-based post-processing shortcuts.
- The spec defines the full intended game, but implementation is expected to arrive through the milestone order above.
