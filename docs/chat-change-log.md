# Chat Change Log

This document summarizes the game changes requested and implemented during the ChatGPT-assisted development session.

## Pull Requests and Branches

### Fire Mario and projectile gameplay

A Fire Mario power-up flow was added so that when Mario reaches the white/fire state, he can shoot fireballs similar to the original Super Mario behavior.

Implemented behavior:

- Fireballs are spawned from Mario when he is in the fire-powered state.
- Fireballs move horizontally in Mario's facing direction.
- Fireballs bounce instead of flying in a straight line.
- Gravity is applied to fireballs.
- Fireball jump steps were reduced to make movement smoother.
- Fireball bounce height was tuned down, then slightly increased after gameplay testing.
- Fireballs can damage enemies.
- Fireballs affect Koopas even while Koopas are in their first death stage, including sliding shell / attack mode.
- When a Koopa is hit by a fireball, it now dies like a Goomba by jumping upward and then falling.

Related fixes:

- Added or selected projectile/bullet assets for the fireball object.
- Updated fireball behavior so it interacts with enemy state transitions more consistently.

## Dynamic background clouds

The old fixed cloud background was replaced by a dynamic cloud layer.

Implemented behavior:

- The background image was removed/replaced with a flat blue background color.
- Dynamic clouds are rendered over the blue background.
- Existing cloud tile assets are used from `assets/sprites/objects/cloud`.
- Supported cloud asset files:
  - `cloud_center_bot.bmp`
  - `cloud_left_bot.bmp`
  - `cloud_right_bot.bmp`
  - `cloud_center_top.bmp`
  - `cloud_left_top.bmp`
  - `cloud_right_top.bmp`
- Purple BMP backgrounds in cloud assets were treated as transparent so cloud sprites render correctly.

## Debug visuals

A debug overlay was added to inspect runtime game state and collision behavior.

Implemented behavior:

- Added debug visuals for objects.
- Added debug labels/status for dynamic and static objects.
- Added physics/collision boundary visualization.
- Added a key toggle to enable or disable debug mode.
- Debug mode can show visible dynamic object counts and body names.
- Static grouped collision bodies are labeled as a single body instead of many duplicate tile labels.
- Non-convex body boundaries are drawn in debug mode.

Purpose:

- Inspect player, enemies, power-ups, fireballs, and static collision geometry.
- Verify platform, pipe, block, and non-convex collision shapes.
- Debug cases where Mario or moving objects enter, stick to, or pass through static bodies.

## Collision system refactor

The collision system was refactored to group adjacent static tiles into higher-level collision bodies.

Implemented behavior:

- Adjacent static tiles of the same type are grouped into one collision body.
- Pipes are treated as one object/body in debug and broad-phase collision logic.
- Blocks, bricks, ground, pipes, and containers can be grouped when adjacent.
- Original map objects remain available for rendering and per-tile behavior.
- Collision still checks the individual parts inside the grouped body, so non-convex shapes remain accurate.
- Debug mode shows one grouped body instead of many repeated static block labels.

Important design detail:

- Static bodies use one outer bounding box for broad-phase checks.
- Inside that broad-phase body, collision is resolved against the actual tile parts.
- This avoids simplifying stairs or non-convex platforms into an incorrect solid rectangle.

## Pipe and block collision fixes

Several collision bugs were addressed after testing with debug mode.

Fixed issues:

- Mario could stand on invalid inner corners of non-convex block shapes.
- Mario could stick to walls and lose air control while jumping alongside vertical surfaces.
- Mario could sometimes enter static bodies from edges.
- Mario could enter pipe top edges when the bottom edge of the player overlapped the top of a pipe cap.
- Enemies and moving objects could get stuck against pipes when collision separation was too aggressive.

Implemented corrections:

- Side separation was restricted so hard horizontal correction is used only where needed for the player.
- Static body top-surface penetration handling was added to prevent Mario from sinking into pipe tops.
- Collision exposed-face detection was added for grouped collision parts.
- Collision checks use exposed top, bottom, left, and right faces so internal edges of grouped bodies do not behave like solid outer walls.

## Enemy and block interaction

Implemented behavior:

- If Mario jumps under a platform or destroys/hits a block, enemies standing on that platform can be hit.
- Enemies above bumped blocks can be killed by the block impact.
- This applies to relevant static platform objects such as bricks, blocks, ground, and containers.

## Player damage and invincibility

Implemented behavior:

- When Mario is hit by an enemy, he flashes for a temporary invincibility period.
- During invincibility, Mario does not collide with enemies for damage purposes.
- This prevents immediate repeated hits after taking damage.

## Mushroom and power-up collision fixes

Several regressions around mushroom spawning and platform edges were investigated.

Observed issues:

- A mushroom spawning from a brick/container could reverse direction immediately instead of moving right.
- A mushroom could appear to collide with its own spawn brick/container.
- A mushroom reaching the edge of a platform could get stuck or be pulled back toward the platform.
- Falling animation around platform edges could become buggy.
- Some attempted pipe/player edge fixes caused regressions in mushroom side collision behavior.

Implemented/attempted corrections:

- Added mushroom fall handling for platform edges.
- Added mushroom side-collision checks to avoid reversing while emerging from a platform/container.
- Added logic to avoid side collision with static platforms while the mushroom is still in the rise/spawn phase.
- Investigated regression caused by global side-penetration correction and identified that it should not apply to mushrooms/enemies.

Current caution:

- Mushroom spawn behavior should be tested carefully after collision changes.
- If the mushroom reverses immediately after popping up, check whether global side collision penetration logic is still being applied to non-player objects.

## README updates

The project README was updated on the master branch to better describe the codebase.

Covered areas:

- General project structure.
- Game loop responsibilities.
- Object and collision system overview.
- Debugging notes.
- Static and dynamic object behavior.

## Scoring fixes and reinforcement-learning power-ups

Two scoring gaps that hurt both the game and the RL reward signal were closed.

Implemented behavior:

- Killing a Koopa with a fireball now shows the "+ 150" label and adds the points, matching Goombas. The Koopa used to react to the bullet from its own collision pass and mark itself dead before `Bullet::hit` could award anything, so the score was silently skipped. The award now lives in `Koopa::fireballDeath()`, which is the single path every fireball and shell kill goes through.
- Eating a mushroom or flower awards 1000 points with a floating label, as in the original game. Previously a power-up was worth nothing, so a score-driven RL agent had no reason to take one.
- Power-ups can no longer be consumed twice in a frame; physics can report the same contact from both the player's and the power-up's collision pass.
- The score label helper is shared in `objects/text.hpp` instead of being copied into `world.cpp`, `bullet.cpp`, and `koopa_troopa.cpp`.

RL environment changes:

- Reward for a power-up appearing from a block, a larger reward for gaining a power level, and a matching penalty for losing one to a hit. Power ranking is computed explicitly because the `Level` enum is declared `NORMAL, POWER, BIG`.
- The score term of the reward is capped per step so a 1000-point pickup cannot dwarf progress and the win bonus.

## RL training ergonomics and a pixel-based agent

- Training shows tqdm progress bars (episode bar with rolling metrics, nested per-episode step bar), with `--progress plain|none` for log files and quiet runs.
- Training verifies the compute device before the first episode: PyTorch build, CUDA availability, GPU name and memory, and where a probe forward pass really ran. `--require-cuda` aborts instead of silently training on the CPU.
- The native environment can rasterize the visible view into an 84x84 grayscale frame (`mario_rl_frame`), with one gray level per object class and Mario the brightest. No SDL surface or display is involved.
- `MarioEnv(observation_mode="pixels")` returns `(frame_stack, 84, 84)` `uint8` stacks, and `PixelDQNAgent` trains on them with a Nature-DQN convolutional dueling network, reusing prioritized replay, n-step returns, and Double DQN targets. `rl.evaluate` and `rl.play` detect pixel checkpoints and switch the environment automatically.

## Soft Actor-Critic agent

- `--algorithm sac` trains a discrete Soft Actor-Critic agent (`rl/sac_agent.py`) on either observation space; `--algorithm dqn` stays the default.
- Discrete formulation: a categorical actor, twin critics scoring all nine actions, exact expectations over the policy in both the soft target and the actor loss, and the minimum of the two critics to curb overestimation.
- Exploration comes from the policy's entropy rather than epsilon. The temperature is tuned automatically towards `--target-entropy-ratio * log(9)`, with `--initial-alpha`, `--alpha-learning-rate`, and `--fixed-alpha` as manual controls. Greedy evaluation and playback take the most likely action.
- Prioritized replay, three-step returns, soft target updates, and both encoders (tile grid and Nature CNN) are shared with DQN through `_build_network`/`_build_replay` hooks, so nothing in the DQN path changed.
- `rl/agents.py` now dispatches checkpoints to the agent class that produced them, so `rl.evaluate` and `rl.play` load DQN, pixel-DQN, and SAC checkpoints with no extra flag.
- Runs log `actor_loss`, `alpha`, and `policy_entropy` as extra columns; the learning-curve image plots temperature and entropy where a DQN run plots epsilon.

## Screenshots

- `Window::save_screenshot()` writes the current frame to a PNG, exposed in the game as `Core::saveScreenshot()` and bound to the `C` key (files land in `screenshots/`). `MarioRLRenderer` has the same helper.
- The README screenshots (gameplay, debug overlay, RL playback, and both observation spaces) are generated from this path, headlessly with `SDL_VIDEODRIVER=dummy`, so they can be refreshed after visual changes.

## Sky, fireball sprite, and fireball limit

- The cloud layer moved into `source/cloud_layer.*`, shared by the game and the RL renderer instead of being duplicated in both.
- Clouds now come in three sizes (one, two, or three center pieces between the caps), on two parallax planes, each drifting at its own rate.
- The fireball no longer borrows `coin.png`. `assets/sprites/objects/fireball.svg` is the source - pixel rects on a 16x16 grid using the existing sprite palette - and `fireball.png` is what it rasterizes to.
- Mario can only keep `World::MAX_ACTIVE_BULLETS` (two) fireballs in flight, enforced for both the keyboard path and the RL `shoot` action. Spent fireballs release their slot.
- `tests/world_rules_smoke.cpp` is a second CTest target covering the fireball limit, the fireball kill score, and the single-pickup power-up rule.

## Level 2 and 3 rebuild

The maps were unplayable, so the engine's real limits were measured first by driving the physics directly: a running jump rises 99 px (4.1 tiles) and travels 195 px (8.1 tiles), the widest pit that can be cleared is 6 tiles, the tallest step that can be climbed is 4 tiles, and a question block is only punchable 3 to 6 rows above the floor beneath it.

What was wrong:

- Level 2 had an 8 tile pit and level 3 had seven 10 tile pits, none of which can be jumped.
- Level 3 also had 5 to 8 tile steps that cannot be climbed.
- 16 reward blocks on level 2 and 9 on level 3 sat on solid stacks, so they could not be hit from below.
- Both flags were a single tile with no pole; level 1 uses a nine tile pole with a head on top.
- Level 2 had four enemies spread over the whole map.

What replaced them, built to the measured limits and to level 1's layout conventions (ground on rows 16-17, low reward row 12, high reward row 8):

- Pits of 3 to 4 tiles on level 2 and 4 to 5 on level 3, always with clear ground before them.
- Every reward block has open air beneath it and sits 4 rows above the surface it is punched from.
- Ledges carry their own reward row above them, so high blocks are reachable from the ledge.
- Pipes, staircases with a plateau, and 15 and 18 enemies respectively.
- Proper flag poles matching level 1.

`tests/level_playthrough_smoke.cpp` plays every level to the flag with a scripted bot and fails if one cannot be crossed. It reports separately how far the same bot gets while fighting, which is a difficulty signal rather than a pass or fail.

## Starting level

- `./Mario --level 2` starts on a chosen level; `--level`, `-l`, `--level=2`, and a bare `./Mario 2` all work. Finishing a level still advances to the next from wherever the run started.

## Notes for future testing

Recommended manual test cases:

1. Fire Mario shoots fireballs left and right.
2. Fireballs bounce with gravity and hit Goombas.
3. Fireballs hit Koopas in walking, shell, and sliding/attack states.
4. Clouds render with transparent backgrounds over a blue sky.
5. Debug toggle shows object bounds and grouped static bodies.
6. Pipes appear as one debug body.
7. Non-convex block structures draw correct exposed boundaries.
8. Mario cannot stand on invalid inside corners of non-convex structures.
9. Mario can jump beside walls without sticking.
10. Mario cannot enter pipe caps from top edges.
11. Enemies and mushrooms do not get stuck against pipes.
12. Mushroom pops out of a brick/container and moves right as expected.
13. Mushroom falls cleanly from platform edges.
14. Enemies standing on a bumped block are killed.
15. Mario flashes and ignores enemy damage during temporary invincibility.
16. A fireball kill on a Koopa shows "+ 150" and raises the score, in walking and shell states.
17. Eating a mushroom or flower shows "+ 1000" once and raises Mario's power level once.
18. `--algorithm sac` trains on both observation spaces, and its checkpoints replay through `rl.play` without extra flags.
19. Holding the fire button never puts more than two fireballs on screen, and the third shot works once one expires.
20. Clouds of three different sizes drift at different speeds as the camera scrolls.
21. Levels 2 and 3 can be finished, and their question blocks can be hit from below.

## Implementation warning

Collision logic is sensitive because player, enemies, fireballs, mushrooms, and static objects all share the same notification path. Fixes intended for Mario, especially side-penetration correction near pipes and static body edges, should be scoped carefully so they do not change enemy or mushroom behavior unexpectedly.
