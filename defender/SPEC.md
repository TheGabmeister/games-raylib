# Primitive-Only Modern Defender

## Overview

This project recreates the core feel of Defender (1981) as a 2D side-scrolling arcade action game in C using raylib. The target is a faithful-plus remake: preserve the signature mechanics, pacing, and tension of the original while modernizing readability, visual polish, and code organization.

The game must remain primitive-only. All gameplay art and effects are built from raylib drawing APIs such as lines, triangles, circles, rectangles, and text. The project must not use imported sprites, textures, render textures, shaders, audio, music, or sound effects. Built-in raylib text drawing is allowed for HUD and menu text because it does not introduce a project-managed asset pipeline. The game is Windows-first, single-player, and designed to build cleanly through the existing CMake setup.

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
- Rendering: primitive-only gameplay art and effects; no imported sprites, no project-managed texture assets, no gameplay shaders, and no render textures
- Audio: none
- Runtime assets: none
- Networking: none
- Save/load: none
- Scope: one local player
- Build system: keep the existing CMake flow; modify `CMakeLists.txt` only if implementation truly requires it
- Vendor policy: do not modify `vendor/`

## Game Flow

The game should use a small, explicit state machine with these launch states:

1. Title
2. Playing
3. Paused
4. Wave Clear
5. Game Over

Required flow behavior:

- Boot to Title
- Press confirm/start to begin a new run at wave 1
- Transition to Playing immediately with a short wave-intro HUD callout
- Enter Paused without advancing simulation
- Enter Wave Clear once a wave has been fully resolved
- Advance automatically from Wave Clear to the next wave after a short delay or on confirm
- Enter Game Over when the player has no remaining lives and no respawn is pending
- From Game Over, allow restart to a fresh run or return to Title

Respawn behavior defaults:

- The player starts with 3 lives
- After death, consume 1 life and respawn after a short delay if lives remain
- Respawning does not reset the current wave
- Respawn grants 1.5 seconds of invulnerability
- Smart bombs reset to the life default on respawn

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

Wave completion is defined as:

- no living enemies remain
- no lander is currently carrying a human
- no human is currently falling
- all delayed split or spawn events for the current wave have been resolved

## World Layout And Defaults

The game world uses a single horizontally wrapping playfield.

Required defaults:

- Viewport target: 1280x720
- World width: 6400 world units
- World height: 720 world units
- Horizontal wrap: seamless for player, enemies, projectiles, humans, and camera presentation
- Terrain band: occupies the lower portion of the world as a continuous heightfield silhouette

Terrain implementation defaults:

- Terrain should be represented as a simple sampled height curve or polyline, not tile maps
- Terrain is used for human placement, safe drop checks, and low-altitude collision risk
- The player is destroyed on hard terrain collision
- Humans are safe only when placed onto valid terrain surface with low downward speed

Vertical bounds defaults:

- The upper play bound should leave a small buffer above visible flight space
- Crossing the upper bound clamps or gently pushes entities back into valid space
- Falling below safe terrain resolution kills the player or human as appropriate

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
- Facing follows the last non-zero horizontal movement intent
- The ship has a clear facing direction and readable thrust state
- Primary fire has a short cooldown and modest projectile speed advantage over enemies
- Primary fire cooldown default: 0.12 seconds
- Primary shots expire after 1.5 seconds if they do not hit a target
- Smart bombs default to 3 per life
- Smart bomb effect destroys all active enemy projectiles and all visible enemies except pods, which instead take lethal damage and split normally
- Hyperspace has a 4.0 second cooldown
- Hyperspace instantly teleports the player to a random safe horizontal location and mid-altitude band
- Hyperspace has a 15% chance to destroy the player on use, increased by 10% for each additional hyperspace use during the same life
- Hyperspace failure chance resets on respawn

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
- Human survival affects both score and end-of-wave outcomes
- If the player dies while carrying a human, the human immediately enters the falling state

Rescue defaults:

- Each wave starts with 10 humans
- Catching a falling human awards 250 points
- Safely returning a carried human awards 500 points
- Completing a wave awards 1000 bonus points for each surviving human
- If all humans die, the wave continues, but subsequent waves spawn with the normal human count

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

Behavior defaults by type:

- Lander: patrols, selects humans, descends to abduct, and fires occasional direct shots at the player
- Mutant: fast hunter created from completed abductions; ignores humans and aggressively chases the player with direct fire
- Bomber: slower mover that drops stationary or drifting mines into the player's path
- Baiter: direct-response hunter that spawns after extended wave duration and pressures stalling play
- Swarmer: very fast short-life attacker spawned from pod destruction; no human interaction
- Pod: medium-speed durable enemy that splits into 3 swarmers on death

Score defaults by type:

- Lander: 150
- Mutant: 150
- Bomber: 250
- Swarmer: 150
- Baiter: 200
- Pod: 1000

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

Authored wave defaults:

- Wave 1: 5 landers, rescue tutorial pressure
- Wave 2: 6 landers and 1 bomber
- Wave 3: 6 landers, 2 bombers, and 1 pod
- Wave 4: 8 landers, 2 bombers, and 2 pods
- Wave 5+: introduce baiter pressure timers and denser mutant recovery situations
- Initial baiter timer default: 25 seconds after wave start
- Endless scaling after wave 5 increases total enemy count gradually and shortens baiter timers by 2 seconds per wave to a floor of 10 seconds

Wave completion should follow the earlier wave-resolution rule and should not require an additional hidden objective.

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
- right trigger for fire
- west face button as alternate fire
- left shoulder for smart bomb
- right shoulder for hyperspace
- start for pause/confirm

The input system should expose an `InputAction` enum and an `InputState` structure with pressed, held, and analog data normalized for both devices.

Deadzone default:

- Gamepad horizontal analog input should use a 0.2 deadzone before normalization

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

HUD defaults:

- Top-left: score and wave
- Top-right: lives and smart bomb stock
- Bottom or top-center strip: human survival status
- Radar: compact horizontal strip showing player, humans, and enemies across full world width

Camera defaults:

- Camera follows the player with look-ahead in facing direction
- Camera smoothing should be strong enough to avoid jitter but light enough to preserve responsiveness
- Screen shake should be short and low amplitude, reserved for deaths, bomb use, and major impacts

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

Recommended source files:

- `src/main.c`
- `src/game.c` and `src/game.h`
- `src/input.c` and `src/input.h`
- `src/world.c` and `src/world.h`
- `src/player.c` and `src/player.h`
- `src/enemy.c` and `src/enemy.h`
- `src/wave.c` and `src/wave.h`
- `src/render.c` and `src/render.h`
- `src/effects.c` and `src/effects.h`
- `src/types.h` or `src/game_types.h`

Runtime rules:

- Simulation target is fixed 60 Hz
- Rendering may interpolate visually if desired, but gameplay state updates on fixed ticks
- Use clear input, update, and draw phases every frame
- Use enum-driven game states such as title, playing, paused, wave clear, and game over
- Use fixed-capacity arrays or pools for enemies, projectiles, humans, and particles
- Avoid heap churn during active gameplay
- Prefer explicit ownership and direct data flow over callback-heavy patterns

Fixed-capacity defaults:

- Humans: 16 max active
- Enemies: 64 max active
- Projectiles: 128 max active
- Particles: 512 max active
- Floating score or HUD callouts: 32 max active

Collision and world rules:

- Horizontal world wraps seamlessly for entities and camera presentation
- Vertical space is bounded, with death or recovery rules defined for leaving the play band
- Terrain collision matters for humans and low-altitude ship risk, but terrain should remain implementation-light
- Radar/minimap should represent off-screen threats and humans consistently

Implementation guidance:

- Use circle or capsule approximations for gameplay collisions rather than pixel-perfect silhouette testing
- Treat carry, rescue, and abduction interactions as explicit state transitions, not emergent overlap tricks
- Keep render-only flourish data separate from gameplay-critical state where practical

## Scoring And Progression Defaults

These defaults should be implemented unless later balancing proves them harmful:

- Starting lives: 3
- Extra life threshold: every 10000 points
- Starting smart bombs per life: 3
- Wave-end human survivor bonus: 1000 per living human
- No score multiplier system
- No permanent upgrades

The score and resource model should stay arcade-simple and transparent.

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
- HUD and menus are readable without relying on imported assets
- No unresolved design-critical ambiguity remains in the implementation plan

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

- No imported sprites, texture assets, gameplay shaders, render textures, music, or sound APIs are used in gameplay implementation
- No runtime asset files are required
- No code under `vendor/` is modified
- Main loop preserves clear input/update/draw separation
- Fixed-capacity containers do not overflow under expected wave caps
- HUD text uses only built-in raylib text functionality or a documented primitive-only fallback

## Defaults And Non-Goals

Implementation defaults:

- Target window: 1280x720 logical design target in 16:9
- Target framerate: 60 FPS with fixed 60 Hz simulation
- Player count: 1
- Modes at launch: title, playing, paused, wave clear, game over
- Rendering style: primitive-only neon sci-fi/vector aesthetic
- Build target: desktop Windows first, with other platforms as non-blocking follow-up work

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
