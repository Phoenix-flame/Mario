#include "world.hpp"

namespace
{
    const int SIDE_COLLISION_TOLERANCE = 5;
    const int TOP_COLLISION_TOLERANCE = 5;
    const int BOTTOM_COLLISION_TOLERANCE = 3;
    const int EDGE_INSET = 2;

    enum CollisionSide
    {
        COLLISION_NONE,
        COLLISION_LEFT,
        COLLISION_RIGHT,
        COLLISION_TOP,
        COLLISION_BOTTOM
    };

    bool pointInsideHorizontalSpan(int x, const Rectangle &rect)
    {
        return x >= rect.left_bottom.x && x <= rect.right_bottom.x;
    }

    bool pointInsideVerticalSpan(int y, const Rectangle &rect)
    {
        return y >= rect.y && y <= (rect.y + rect.h);
    }

    bool hasVerticalOverlapForSideCollision(const Rectangle &moving, const Rectangle &solid)
    {
        return pointInsideVerticalSpan(moving.right_center.y, solid) ||
               pointInsideVerticalSpan(moving.y, solid) ||
               pointInsideVerticalSpan(moving.y + moving.h - 1, solid);
    }

    bool supportsTopCollision(Object *obj)
    {
        return obj->getType() == PLAYER || obj->getType() == G_BULLET;
    }

    bool shouldApplyTopCornerCollision(Object *obj)
    {
        return obj->getType() == G_BULLET || ((Player *)obj)->getState() == JUMP;
    }

    CollisionSide detectSideCollision(const Rectangle &moving, const Rectangle &solid)
    {
        if (!hasVerticalOverlapForSideCollision(moving, solid))
        {
            return COLLISION_NONE;
        }

        if (abs(moving.right_center.x - solid.left_center.x) < SIDE_COLLISION_TOLERANCE)
        {
            return COLLISION_RIGHT;
        }
        if (abs(moving.left_center.x - solid.right_center.x) < SIDE_COLLISION_TOLERANCE)
        {
            return COLLISION_LEFT;
        }

        return COLLISION_NONE;
    }

    bool isLandingOnTop(const Rectangle &moving, const Rectangle &solid)
    {
        return moving.bottom_center.y <= solid.top_center.y &&
               abs(moving.bottom_center.y - solid.top_center.y) < BOTTOM_COLLISION_TOLERANCE;
    }

    bool bottomProbeOverlapsSolid(int probe_x, const Rectangle &solid)
    {
        return pointInsideHorizontalSpan(probe_x, solid);
    }

    bool detectBottomCollision(const Rectangle &moving, const Rectangle &solid)
    {
        int left_probe = moving.left_bottom.x + EDGE_INSET;
        int right_probe = moving.right_bottom.x - EDGE_INSET;

        return bottomProbeOverlapsSolid(moving.bottom_center.x, solid) ||
               bottomProbeOverlapsSolid(left_probe, solid) ||
               bottomProbeOverlapsSolid(right_probe, solid);
    }

    bool hitsSolidCeiling(const Rectangle &moving, const Rectangle &solid)
    {
        return moving.top_center.y >= solid.bottom_center.y &&
               moving.top_center.y - solid.bottom_center.y < TOP_COLLISION_TOLERANCE;
    }

    bool detectTopCenterCollision(const Rectangle &moving, const Rectangle &solid)
    {
        return moving.top_center.x >= solid.left_top.x && moving.top_center.x <= solid.right_top.x;
    }

    bool detectTopLeftCornerCollision(const Rectangle &moving, const Rectangle &solid)
    {
        int left_probe = moving.left_top.x + EDGE_INSET;
        return left_probe >= solid.left_top.x && left_probe <= solid.right_top.x;
    }

    bool detectTopRightCornerCollision(const Rectangle &moving, const Rectangle &solid)
    {
        int right_probe = moving.right_top.x - EDGE_INSET;
        return right_probe >= solid.left_top.x && right_probe <= solid.right_top.x;
    }

    void notifySideCollision(Object *moving_obj, Object *solid_obj, CollisionSide side)
    {
        if (side == COLLISION_RIGHT)
        {
            moving_obj->notifyCollisionRight(solid_obj);
            solid_obj->notifyCollisionLeft(moving_obj);
        }
        else if (side == COLLISION_LEFT)
        {
            moving_obj->notifyCollisionLeft(solid_obj);
            solid_obj->notifyCollisionRight(moving_obj);
        }
    }
}

World::World()
{
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
    this->gameState = new GameState();
}

void World::loop()
{
    ghostCollector();
    collision(map->player);
    for (unsigned int i = 0; i < map->objects.size(); i++)
    {
        Type t = map->objects[i]->getType();
        if (t == PLAYER)
        {
            continue;
        }
        else if (t == BRICK)
        {
            ((Brick *)map->objects[i])->update();
            if (((Brick *)map->objects[i])->broken)
            {
                map->objects.erase(map->objects.begin() + i);
            }
        }
        else if (t == COIN_CONTAINER)
        {
            ((CoinContainer *)map->objects[i])->update();
        }
        else if (t == FIRE_CONTAINER)
        {
            ((FireContainer *)map->objects[i])->update();
        }
        else if (t == HEALTH_CONTAINER)
        {
            ((HealthContainer *)map->objects[i])->update();
        }
        else if (t == GOOMBA)
        {
            if ((map->objects[i]->getPos() + camera->getPos()).x < 700)
            {
                ((Goomba *)map->objects[i])->seen();
            }
            collision(map->objects[i]);

            if (map->objects[i]->getPos().y > 500)
            {
                map->objects.erase(map->objects.begin() + i);
                continue;
            }

            ((Goomba *)map->objects[i])->update();
        }
        else if (t == KOOPA)
        {
            if ((map->objects[i]->getPos() + camera->getPos()).x < 700)
            {
                ((Koopa *)map->objects[i])->seen();
            }
            collision(map->objects[i]);
            if (map->objects[i]->getPos().y > 500)
            {
                map->objects.erase(map->objects.begin() + i);
                continue;
            }

            ((Koopa *)map->objects[i])->update();
        }
    }

    for (unsigned int i = 0; i < ghosts.size(); i++)
    {
        if (ghosts[i]->ghost_dead)
        {
            ghosts.erase(ghosts.begin() + i);
            continue;
        }
        if (ghosts[i]->getType() == G_COIN)
        {
            ((Coin *)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() == G_TEXT)
        {
            ((Text *)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() == G_MUSHROOM)
        {
            collision(ghosts[i]);
            ((Mushroom *)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() == G_FLOWER)
        {
            collision(ghosts[i]);
            ((Flower *)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() == G_BULLET)
        {
            collision(ghosts[i]);
            ((Bullet *)ghosts[i])->update();
        }
    }
}

void World::ghostCollector()
{
    for (auto obj : map->objects)
    {
        if (obj->has_ghost)
        {
            for (auto g : obj->ghost)
            {
                ghosts.push_back(g);
                if (g->getType() == G_TEXT)
                {
                    gameState->score += ((Text *)g)->score;
                }
            }
            obj->has_ghost = false;
            obj->ghost.clear();
        }
    }
}

void World::addGhost(Object *obj)
{
    ghosts.push_back(obj);
}

std::vector<Object *> World::getObjects()
{
    return map->objects;
}

GameState *World::getGameState()
{
    return gameState;
}

Player *World::getPlayer()
{
    return map->player;
}

std::vector<Object *> World::getGhosts()
{
    return ghosts;
}

void World::collision(Object *obj)
{
    if (obj->dead)
    {
        return;
    }

    bool on_the_floor = false;
    bool top_center_collision = false;
    bool top_corner_collision = false;
    Object *top_corner_object = nullptr;

    obj->notifyFreeLeft();
    obj->notifyFreeRight();

    for (auto o : getObjects())
    {
        if (o->dead || o == obj)
        {
            continue;
        }

        Rectangle moving_rect(obj->getPos(), obj->getPos() + obj->getSize());
        Rectangle solid_rect(o->getPos(), o->getPos() + o->getSize());

        notifySideCollision(obj, o, detectSideCollision(moving_rect, solid_rect));

        if (detectBottomCollision(moving_rect, solid_rect))
        {
            if (isLandingOnTop(moving_rect, solid_rect))
            {
                on_the_floor = true;
                obj->notifyCollisionBottom(o);
            }
            else
            {
                obj->notifyDistToPlatform(abs(moving_rect.bottom_center.y - solid_rect.top_center.y));
            }
        }

        if (!supportsTopCollision(obj))
        {
            continue;
        }

        if (detectTopCenterCollision(moving_rect, solid_rect))
        {
            if (hitsSolidCeiling(moving_rect, solid_rect))
            {
                obj->notifyCollisionTop(o);
                top_center_collision = true;
                top_corner_collision = true;
                return;
            }

            obj->notifyDistToCeil(abs(moving_rect.top_center.y - solid_rect.bottom_center.y));
            continue;
        }

        if (top_center_collision)
        {
            continue;
        }

        bool top_corner_overlaps = detectTopLeftCornerCollision(moving_rect, solid_rect) ||
                                   detectTopRightCornerCollision(moving_rect, solid_rect);
        if (!top_corner_overlaps)
        {
            continue;
        }

        if (hitsSolidCeiling(moving_rect, solid_rect))
        {
            if (shouldApplyTopCornerCollision(obj))
            {
                top_corner_collision = true;
                top_corner_object = o;
            }
        }
        else
        {
            obj->notifyDistToCeil(abs(moving_rect.top_center.y - solid_rect.bottom_center.y));
        }
    }

    if (!on_the_floor)
    {
        obj->notifyFreeBottom();
    }

    if (supportsTopCollision(obj) && !top_center_collision && top_corner_collision && top_corner_object != nullptr)
    {
        obj->notifyCollisionTop(top_corner_object);
    }
}
