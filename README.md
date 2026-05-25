# Mario

[![Readme Card](https://github-readme-stats.vercel.app/api/pin/?username=Phoenix-flame&repo=Mario&theme=vue-dark)](https://github.com/anuraghazra/github-readme-stats)

A small Super Mario Bros. style platformer written from scratch in C++ with SDL2. The project currently implements a playable World 1-1 style level, Mario movement, enemies, power-ups, map loading, collision handling, sound, camera movement, and sprite-based rendering.

![Screenshot](https://github.com/Phoenix-flame/Mario/blob/master/images/1.png?raw=true)

## Requirements

Install the SDL2 development packages:

```bash
sudo apt install libsdl2-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-image-dev
```

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
| `Q` | Quit |

## Codebase Overview

The game is intentionally implemented without a game engine. The main systems are organized under `source/`:

| Path | Purpose |
| --- | --- |
| `source/core.*` | Main game loop, input handling, frame timing, rendering orchestration, HUD, reset flow, and high-level player actions. |
| `source/world.*` | Owns the active map, camera, game state, ghosts/transient objects, object updates, and collision dispatch. |
| `source/map.*` | Loads the text-based level file and converts map symbols into game objects. |
| `source/player.*` | Mario state machine, movement, jumping, level/power-up state, death, and sprite selection. |
| `source/camera.*` | Camera and background parallax offsets. |
| `source/audio.*` | Sound/music update logic based on player state. |
| `source/physics.*` | Physics helper code. |
| `source/objects/` | Base `Object` class and concrete gameplay objects such as blocks, bricks, pipes, enemies, mushrooms, flowers, coins, and fireballs. |
| `assets/` | Sprites, maps, sounds, fonts, and background/cloud assets. |
| `images/` | README/debug screenshots. |

## Runtime Architecture

`Core` creates the window, world, timers, and audio system. Each frame follows this flow:

1. Poll keyboard/window events.
2. Update player input state and camera movement.
3. Run `World::loop()` to update objects, collect spawned ghost objects, and process collisions.
4. Update audio.
5. Draw the background, objects, player, ghosts, and HUD.
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

### Example Sprites

| Type | Image |
| --- | --- |
| `BLOCK` | ![BLOCK](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/block.png) |
| `BRICK` | ![BRICK](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/brick.png) |
| `GROUND` | ![GROUND](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/clay.png) |
| `FIRE_CONTAINER` | ![FIRE_CONTAINER](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/question-3.png) |
| `HEALTH_CONTAINER` | ![HEALTH_CONTAINER](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/question-2.png) |
| `COIN_CONTAINER` | ![COIN_CONTAINER](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/bricks_blocks/question-1.png) |
| `PLAYER` | ![Mario](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/mario/normal/walking-right-1.png) ![Big Mario](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/mario/big/walking-right-1.png) ![Fire Mario](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/mario/white/walking-right-1.png) |
| `GOOMBA` | ![Goomba](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/little_goomba/walking-1.png) ![Goomba](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/little_goomba/walking-2.png) ![Goomba Dead](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/little_goomba/dead.png) |
| `KOOPA` | ![Koopa](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/walking-left-1.png) ![Koopa](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/walking-right-1.png) ![Koopa Shell](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/enemies/koopa_troopa/dead.png) |
| `G_COIN` | ![Coin](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/coin.png) |
| `G_MUSHROOM` | ![Health Mushroom](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/mushroom/health.png) ![Red Mushroom](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/mushroom/red.png) |
| `G_FLOWER` | ![Flower](https://github.com/Phoenix-flame/Mario/blob/master/assets/sprites/objects/flower.png) |

## Collision System

Collision handling is dispatched from `World::collision(Object *obj)`. Objects receive collision notifications through virtual hooks such as:

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

`Core::showDebug()` contains debug drawing helpers for FPS, player state, speed, collision rectangles, and selected objects. It is currently guarded by `#if 0` in `Core::draw()` and can be enabled during development.

![Debug screenshot](https://github.com/Phoenix-flame/Mario/blob/master/images/3.png?raw=true)

## Notes

This project was originally built as an Advanced Programming course project at the University of Tehran. The codebase favors simple custom systems over engine abstractions, which makes it useful for learning game loops, collision callbacks, SDL2 rendering, and tile-map driven gameplay.
