# Mario

[![Readme Card](https://github-readme-stats.vercel.app/api/pin/?username=Phoenix-flame&repo=Mario&theme=vue-dark)](https://github.com/anuraghazra/github-readme-stats)

A small Super Mario Bros. style platformer written from scratch in C++ with SDL2. The project currently implements a playable World 1-1 style level, Mario movement, enemies, power-ups, map loading, Box2D-backed collision handling, sound, camera movement, and sprite-based rendering.

![Screenshot](https://github.com/Phoenix-flame/Mario/blob/master/images/1.png?raw=true)

## Requirements

Install the SDL2 development packages:

```bash
sudo apt install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-image-dev
```

Collision detection uses Box2D. CMake first tries to find an installed Box2D package and falls back to downloading Box2D v2.4.1 with `FetchContent` when it is missing.

## Build

```bash
mkdir -p build
cd build
cmake ..
make
./Mario
```

## Controls

| Key | Action |
| --- | --- |
| `O` | Move right |
| `P` | Move left |
| `R` or `Space` | Jump |
| `Z` | Shoot fireball when Mario has the fire power-up |
| `D` | Toggle debug visual overlay |
| `Q` | Quit |

## Codebase Overview

The game is intentionally implemented without a game engine. The main systems are organized under `source/`:

| Path | Purpose |
| --- | --- |
| `source/core.*` | Main game loop, input handling, frame timing, rendering orchestration, HUD, reset flow, debug overlay, and high-level player actions. |
| `source/world.*` | Owns the active map, camera, game state, ghosts/transient objects, object updates, and collision dispatch. |
| `source/map.*` | Loads the text-based level file and converts map symbols into game objects. |
| `source/player.*` | Mario state machine, movement, jumping, level/power-up state, death, and sprite selection. |
| `source/camera.*` | Camera and background parallax offsets. |
| `source/audio.*` | Sound/music update logic based on player state. |
| `source/physics.*` | Box2D-backed collision fixtures, contact listener, and object notification dispatch. |
| `source/objects/` | Base `Object` class and concrete gameplay objects such as blocks, bricks, pipes, enemies, mushrooms, flowers, coins, and fireballs. |
| `assets/` | Sprites, maps, sounds, fonts, and background/cloud assets. |
| `images/` | README/debug screenshots. |

## Runtime Architecture

`Core` creates the window, world, timers, and audio system. Each frame follows this flow:

1. Poll keyboard/window events.
2. Update player input state and camera movement.
3. Run `World::loop()` to update objects, collect spawned ghost objects, and process collisions.
4. Update audio.
5. Draw the background, objects, player, ghosts, HUD, and optional debug overlay.
6. Delay as needed to keep the frame timing stable.

## Map System

Levels are loaded from text files under `assets/maps/`. Each character in the map grid represents a tile or object. `Map` converts those symbols into strongly typed objects such as ground blocks, bricks, question blocks, pipes, enemies, flag pieces, and the player spawn.

This approach is simple and easy to edit, but complex objects like pipes are represented as multiple tiles, so they also become multiple collision objects.

## Object System

All gameplay objects inherit from `Object`. The base class stores shared data such as position, size, image path, object type, dead/ghost flags, and collision notification hooks.

Important object categories:

| Type | Description |
| --- | --- |
| `BLOCK`, `BRICK`, `GROUND` | Static level collision tiles. |
| `COIN_CONTAINER`, `FIRE_CONTAINER`, `HEALTH_CONTAINER` | Question blocks that spawn rewards. |
| `PIPE`, `FLAG` | Multi-tile world structures. |
| `GOOMBA`, `KOOPA` | Enemy objects with their own movement and death states. |
| `PLAYER` | Mario. |
| `G_COIN`, `G_TEXT`, `G_MUSHROOM`, `G_FLOWER`, `G_BULLET` | Ghost/transient objects spawned during gameplay. |

## Collision System

Collision handling is dispatched from `World::collision(Object *obj)` into `Physics::collision()`. `Physics` builds a lightweight Box2D world for the current query, creates sensor fixtures for the moving object, static collision tiles, enemies, and ghost objects, then uses a `b2ContactListener` to convert Box2D contacts back into the existing gameplay notification hooks:

- `notifyCollisionLeft()`
- `notifyCollisionRight()`
- `notifyCollisionTop()`
- `notifyCollisionBottom()`
- `notifyFreeBottom()`
- `notifyDistToPlatform()`
- `notifyDistToCeil()`

The object itself decides how to react. For example, Mario can land on enemies, enemies can turn around when hitting walls, mushrooms can move along platforms, and fireballs can bounce or disappear when they hit something.

## Ghost Objects

Ghost objects are temporary objects that are not part of the static map. They are collected by `World::ghostCollector()` and stored in the `ghosts` vector. Examples include coins spawned from blocks, score text, mushrooms, flowers, and fireballs.

This keeps temporary gameplay effects separate from the main map object list while still allowing them to be updated, drawn, and checked for collisions.

## Graphics

Rendering uses SDL2 through the local `rsdl` wrapper. The game draws:

1. Background/sky and clouds.
2. Ghost objects.
3. Map objects.
4. Player sprite.
5. HUD text.

Sprites are stored under `assets/sprites/`. Cloud sprites are stored in `assets/sprites/objects/cloud/`.

## Debugging

Press `D` while the game is running to toggle the debug visual overlay.

The overlay shows:

- FPS, score, timer, object count, and ghost count.
- Player state, direction, speed, world position, and camera position.
- Player collision box and center/collision guide lines.
- Screen/camera viewport boundaries.
- Visible object bounding boxes, object type labels, and dead-state markers.
- Ghost/transient object bounding boxes and labels.
- Physics helper markers such as object centers, top/bottom/left/right collision points, and the player's nearest-platform guide line.

![Debug screenshot](https://github.com/Phoenix-flame/Mario/blob/master/images/3.png?raw=true)

## Notes

This project was originally built as an Advanced Programming course project at the University of Tehran. The codebase favors simple custom systems over engine abstractions, which makes it useful for learning game loops, collision callbacks, SDL2 rendering, and tile-map driven gameplay.
