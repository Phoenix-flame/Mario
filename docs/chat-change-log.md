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

## Implementation warning

Collision logic is sensitive because player, enemies, fireballs, mushrooms, and static objects all share the same notification path. Fixes intended for Mario, especially side-penetration correction near pipes and static body edges, should be scoped carefully so they do not change enemy or mushroom behavior unexpectedly.
