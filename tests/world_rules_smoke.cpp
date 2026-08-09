// Gameplay rules that are easy to break from the collision or scoring code:
// the fireball limit, the fireball kill score, and the power-up pickup.
#include <cassert>
#include <iostream>

#include "objects/bullet.hpp"
#include "world.hpp"

#ifndef MARIO_SOURCE_DIR
#define MARIO_SOURCE_DIR "."
#endif

namespace
{
    void tick(World &world, int frames)
    {
        for (int i = 0; i < frames; i++)
        {
            Timer::advanceSimulatedTime(33);
            world.getPlayer()->update(0);
            world.loop();
        }
    }

    Object *firstOfType(World &world, Type type)
    {
        for (auto object : world.getObjects())
        {
            if (object->getType() == type)
            {
                return object;
            }
        }
        return nullptr;
    }

    // Mirrors what Core and the RL environment do when the fire button is held.
    bool tryShoot(World &world)
    {
        if (!world.canShoot())
        {
            return false;
        }
        Point position = world.getPlayer()->getPos();
        world.addGhost(new Bullet(position.x + 24, position.y + 8, RIGHT));
        return true;
    }

    int failures = 0;

    void check(bool condition, const std::string &description)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << description << std::endl;
            failures++;
        }
    }

    void testFireballLimit()
    {
        World world(1, MARIO_SOURCE_DIR);
        tick(world, 2);

        int fired = 0;
        int refused = 0;
        for (int attempt = 0; attempt < 12; attempt++)
        {
            (tryShoot(world) ? fired : refused)++;
            tick(world, 1);
            check(
                world.countActiveBullets() <= World::MAX_ACTIVE_BULLETS,
                "more fireballs in flight than the limit allows"
            );
        }
        check(fired == World::MAX_ACTIVE_BULLETS, "holding fire spawned the wrong number of shots");
        check(refused > 0, "the fireball limit never rejected a shot");

        // Once the shots leave the map their slots must come back.
        tick(world, 220);
        check(world.countActiveBullets() == 0, "spent fireballs were not collected");
        check(tryShoot(world), "could not shoot again after the fireballs expired");
    }

    void testFireballKillScoresLikeOtherEnemies()
    {
        World world(1, MARIO_SOURCE_DIR);
        Object *koopa = firstOfType(world, KOOPA);
        assert(koopa != nullptr);

        int before = world.getGameState()->score;
        Point position = koopa->getPos();
        world.addGhost(new Bullet(position.x - 18, position.y + 4, RIGHT));
        tick(world, 6);
        check(
            world.getGameState()->score - before == 150,
            "a fireball kill on a koopa did not award 150 points"
        );
    }

    void testPowerUpIsEatenOnceAndScores()
    {
        World world(1, MARIO_SOURCE_DIR);
        Object *block = firstOfType(world, HEALTH_CONTAINER);
        assert(block != nullptr);

        int before = world.getGameState()->score;
        block->mark();
        tick(world, 20);

        Object *mushroom = nullptr;
        for (auto ghost : world.getGhosts())
        {
            if (ghost->getType() == G_MUSHROOM)
            {
                mushroom = ghost;
            }
        }
        check(mushroom != nullptr, "the health block released no mushroom");
        if (mushroom == nullptr)
        {
            return;
        }

        Point at = mushroom->getPos();
        world.getPlayer()->setPos(at.x + 24, at.y - 6);
        tick(world, 60);
        check(world.getPlayer()->getLevel() == BIG, "the mushroom was not consumed exactly once");
        check(
            world.getGameState()->score - before == 1000,
            "eating a mushroom did not award exactly 1000 points"
        );
    }
}

int main()
{
    Timer::useSimulatedTime(0);
    testFireballLimit();
    testFireballKillScoresLikeOtherEnemies();
    testPowerUpIsEatenOnceAndScores();

    if (failures > 0)
    {
        std::cerr << failures << " gameplay rule check(s) failed" << std::endl;
        return 1;
    }
    std::cout << "world rules smoke test passed" << std::endl;
    return 0;
}
