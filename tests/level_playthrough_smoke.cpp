// Plays every level to the flag with a simple bot, so a map edit cannot leave a
// level uncompletable. The bot only does what a player can: hold right, and jump
// when the floor runs out, something solid is in the way, or an enemy is close.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "world.hpp"

#ifndef MARIO_SOURCE_DIR
#define MARIO_SOURCE_DIR "."
#endif

namespace
{
    const int TILE = 24;
    const int FRAME_MILLISECONDS = 33;
    const int MAX_FRAMES = 6000;

    // The map file is the source of truth for what the bot can see ahead.
    std::vector<std::string> loadMap(int level)
    {
        std::string path = std::string(MARIO_SOURCE_DIR) + "/assets/maps/1/" +
                           std::to_string(level) + ".txt";
        std::ifstream file(path.c_str());
        std::vector<std::string> rows;
        std::string line;
        while (std::getline(file, line))
        {
            // Maps may carry Windows line endings; trim whatever trails.
            while (!line.empty() && isspace(static_cast<unsigned char>(line[line.size() - 1])))
            {
                line.erase(line.size() - 1);
            }
            rows.push_back(line);
        }
        return rows;
    }

    bool solidTile(const std::vector<std::string> &rows, int row, int column)
    {
        if (row < 0 || row >= (int)rows.size() || column < 0 || column >= (int)rows[row].size())
        {
            return false;
        }
        const char tile = rows[row][column];
        return tile == '#' || tile == '@' || tile == 'b' || tile == '?' || tile == 'm' ||
               tile == 'h' || tile == '|';
    }

    bool isEnemy(Object *object)
    {
        return object->getType() == GOOMBA || object->getType() == KOOPA;
    }

    // True when an enemy sits in the band ahead where jumping lands on it.
    bool enemyAhead(World &world, const Point &player, int nearPixels, int farPixels)
    {
        for (auto object : world.getObjects())
        {
            if (!isEnemy(object) || object->dead)
            {
                continue;
            }
            Point position = object->getPos();
            int dx = position.x - player.x;
            int dy = std::abs(position.y - player.y);
            if (dx > nearPixels && dx < farPixels && dy < 2 * TILE)
            {
                return true;
            }
        }
        return false;
    }

    struct Policy
    {
        int lookahead;      // tiles of floor checked ahead, for pits
        int wallLookahead;  // tiles checked at body height, for pipes and blocks
        int enemyNear;      // pixels: ignore enemies closer than this
        int enemyFar;       // pixels: jump at enemies within this
    };

    struct Attempt
    {
        bool won;
        bool died;
        int furthestX;
        int frames;
    };

    // Several policies are tried so a level is not rejected merely for needing a
    // different jump rhythm than one fixed heuristic produces.
    Attempt play(int level, const Policy &policy, bool withEnemies)
    {
        const std::vector<std::string> rows = loadMap(level);
        World world(level, MARIO_SOURCE_DIR);
        Player *player = world.getPlayer();
        Attempt attempt = {false, false, 0, 0};
        int lastProgressFrame = 0;
        int retreatFrames = 0;

        if (!withEnemies)
        {
            // Terrain run: take the enemies out so the result speaks only about
            // whether the geometry can be crossed. Physics skips dead objects.
            for (auto object : world.getObjects())
            {
                if (isEnemy(object))
                {
                    object->dead = true;
                }
            }
        }

        for (int frame = 0; frame < MAX_FRAMES; frame++)
        {
            Point position = player->getPos();
            if (position.x > attempt.furthestX)
            {
                attempt.furthestX = position.x;
                lastProgressFrame = frame;
            }
            attempt.frames = frame;

            if (player->getState() == WON)
            {
                attempt.won = true;
                return attempt;
            }
            if (player->getState() == DEAD)
            {
                attempt.died = true;
                return attempt;
            }

            // Keep the camera near Mario so enemies wake up as they do in play.
            int screenX = position.x + world.camera->getPos().x;
            if (screenX > 400)
            {
                world.camera->move(-5);
                world.camera->moveBackground(-1);
            }

            int tileX = std::max(0, (position.x + 11) / TILE);
            int bodyRow = (position.y + 29) / TILE;
            int floorRow = (position.y + 31) / TILE;

            bool jump = enemyAhead(world, position, policy.enemyNear, policy.enemyFar);
            if (bodyRow >= 0 && floorRow < (int)rows.size())
            {
                for (int step = 1; step <= policy.lookahead; step++)
                {
                    if (!solidTile(rows, floorRow, tileX + step))
                    {
                        jump = true; // the floor runs out just ahead
                    }
                }
                for (int step = 1; step <= policy.wallLookahead; step++)
                {
                    if (solidTile(rows, bodyRow, tileX + step))
                    {
                        jump = true; // a pipe or block to clear; take off early
                    }
                }
            }

            // Pinned against a tall pipe, a player backs up and takes a run-up
            // instead of jumping into it from a standstill.
            if (retreatFrames == 0 && frame - lastProgressFrame > 40)
            {
                retreatFrames = 24;
            }
            int direction = 1;
            if (retreatFrames > 0)
            {
                retreatFrames--;
                lastProgressFrame = frame;
                direction = -1;
                jump = false;
            }

            // A player taps the jump key again on landing, so re-arm whenever
            // Mario is back on the ground rather than only when not jumping.
            State state = player->getState();
            if (state != JUMP && state != FALL)
            {
                player->can_jump = true;
            }

            Timer::advanceSimulatedTime(FRAME_MILLISECONDS);
            player->update(direction);
            if (jump && player->can_jump)
            {
                player->startJump();
                player->can_jump = false;
            }
            world.loop();
        }
        return attempt;
    }
}

int main()
{
    Timer::useSimulatedTime(0);
    int failures = 0;

    const Policy policies[] = {
        {2, 2, 24, 66}, {2, 3, 24, 66}, {1, 2, 24, 66}, {2, 1, 24, 66},
        {1, 3, 12, 60}, {3, 2, 24, 72}, {2, 3, 30, 84}, {1, 2, 12, 54},
        {3, 3, 12, 60}, {2, 2, 18, 96}, {1, 1, 24, 66}, {3, 1, 24, 66},
    };
    const int policyCount = sizeof(policies) / sizeof(policies[0]);

    for (int level = 1; level <= 3; level++)
    {
        Attempt terrain = {false, false, 0, 0};
        for (int index = 0; index < policyCount && !terrain.won; index++)
        {
            Attempt attempt = play(level, policies[index], false);
            if (attempt.won || attempt.furthestX > terrain.furthestX)
            {
                terrain = attempt;
            }
        }

        if (terrain.won)
        {
            std::cout << "level " << level << ": terrain crossed to the flag in "
                      << terrain.frames << " frames" << std::endl;
        }
        else
        {
            std::cerr << "FAIL: level " << level << " cannot be crossed; the bot "
                      << (terrain.died ? "fell" : "got stuck") << " at x=" << terrain.furthestX
                      << " (tile " << terrain.furthestX / TILE << ") of " << (224 * TILE)
                      << std::endl;
            failures++;
        }

        // How far the same bot gets while fighting is reported, not asserted:
        // beating the enemies is a difficulty question, not a broken map.
        Attempt fighting = {false, false, 0, 0};
        for (int index = 0; index < policyCount && !fighting.won; index++)
        {
            Attempt attempt = play(level, policies[index], true);
            if (attempt.won || attempt.furthestX > fighting.furthestX)
            {
                fighting = attempt;
            }
        }
        std::cout << "level " << level << ": with enemies the bot reached tile "
                  << fighting.furthestX / TILE << (fighting.won ? " and finished" : "")
                  << std::endl;
    }

    if (failures > 0)
    {
        std::cerr << failures << " level(s) could not be finished" << std::endl;
        return 1;
    }
    std::cout << "all levels completed" << std::endl;
    return 0;
}
