#include "world.hpp"
#include <algorithm>

namespace
{
    const int SIDE_COLLISION_TOLERANCE = 5;
    const int SIDE_PENETRATION_TOLERANCE = 8;
    const int TOP_PENETRATION_TOLERANCE = 16;
    const int TOP_COLLISION_TOLERANCE = 5;
    const int BOTTOM_COLLISION_TOLERANCE = 3;
    const int EDGE_INSET = 2;
    const int BLOCK_BUMP_ENEMY_Y_TOLERANCE = 6;

    enum CollisionSide { COLLISION_NONE, COLLISION_LEFT, COLLISION_RIGHT, COLLISION_TOP, COLLISION_BOTTOM };

    bool pointInsideHorizontalSpan(int x, const Rectangle &rect){ return x >= rect.left_bottom.x && x <= rect.right_bottom.x; }
    bool hasVerticalOverlapForSideCollision(const Rectangle &moving, const Rectangle &solid){ return moving.y < solid.y + solid.h && moving.y + moving.h > solid.y; }
    bool supportsTopCollision(Object *obj){ return obj->getType() == PLAYER || obj->getType() == G_BULLET; }
    bool shouldApplyTopCornerCollision(Object *obj){ return obj->getType() == G_BULLET || ((Player *)obj)->getState() == JUMP; }
    bool isEnemy(Object *obj){ return obj->getType() == GOOMBA || obj->getType() == KOOPA; }
    bool isStaticCollisionTile(Object *obj){ Type t = obj->getType(); return t == BLOCK || t == BRICK || t == GROUND || t == PIPE || t == COIN_CONTAINER || t == FIRE_CONTAINER || t == HEALTH_CONTAINER; }
    bool isPlayerEnemyPair(Object *a, Object *b){ return (a->getType() == PLAYER && isEnemy(b)) || (b->getType() == PLAYER && isEnemy(a)); }
    bool shouldIgnoreEnemyCollision(Object *moving_obj, Object *solid_obj){ if (!isPlayerEnemyPair(moving_obj, solid_obj)) return false; Player *player = (moving_obj->getType() == PLAYER) ? (Player *)moving_obj : (Player *)solid_obj; return player->isInvincible(); }
    bool hasHorizontalOverlap(const Rectangle &a, const Rectangle &b){ return a.left_top.x < b.right_top.x && a.right_top.x > b.left_top.x; }
    bool horizontalOverlap(const Rectangle &a, const Rectangle &b){ return a.x < b.x + b.w && a.x + a.w > b.x; }
    bool verticalOverlap(const Rectangle &a, const Rectangle &b){ return a.y < b.y + b.h && a.y + a.h > b.y; }

    bool expandedBoundsOverlap(const Rectangle &moving, const Rectangle &bounds)
    {
        const int MARGIN = 16;
        Rectangle expanded(Point(bounds.x - MARGIN, bounds.y - MARGIN), Point(bounds.x + bounds.w + MARGIN, bounds.y + bounds.h + MARGIN));
        return moving.x < expanded.x + expanded.w && moving.x + moving.w > expanded.x && moving.y < expanded.y + expanded.h && moving.y + moving.h > expanded.y;
    }

    bool rectanglesTouchOrOverlap(const Rectangle &a, const Rectangle &b)
    {
        return a.x <= b.x + b.w && a.x + a.w >= b.x && a.y <= b.y + b.h && a.y + a.h >= b.y;
    }

    Rectangle objectRect(Object *obj){ return Rectangle(obj->getPos(), obj->getPos() + obj->getSize()); }

    Rectangle mergeBounds(const Rectangle &a, const Rectangle &b)
    {
        int left = std::min(a.x, b.x); int top = std::min(a.y, b.y);
        int right = std::max(a.x + a.w, b.x + b.w); int bottom = std::max(a.y + a.h, b.y + b.h);
        return Rectangle(Point(left, top), Point(right, bottom));
    }

    bool isStandingOnPlatform(const Rectangle &enemy, const Rectangle &platform)
    {
        return hasHorizontalOverlap(enemy, platform) && abs(enemy.bottom_center.y - platform.top_center.y) <= BLOCK_BUMP_ENEMY_Y_TOLERANCE;
    }

    bool canBumpEnemies(Object *obj)
    {
        Type t = obj->getType();
        return t == BRICK || t == COIN_CONTAINER || t == FIRE_CONTAINER || t == HEALTH_CONTAINER || t == BLOCK || t == GROUND;
    }

    bool canUseTopPenetrationResolution(Object *obj)
    {
        Type t = obj->getType();
        return t == PLAYER || t == GOOMBA || t == KOOPA ||
               t == G_MUSHROOM || t == G_FLOWER || t == G_BULLET;
    }

    CollisionSide detectSideCollision(const Rectangle &moving, const CollisionPart &part)
    {
        const Rectangle &solid = part.rect;
        if (!hasVerticalOverlapForSideCollision(moving, solid)) return COLLISION_NONE;
        int overlap_y = std::min(moving.y + moving.h, solid.y + solid.h) - std::max(moving.y, solid.y);
        if (overlap_y <= EDGE_INSET) return COLLISION_NONE;

        int penetration_from_left = moving.x + moving.w - solid.x;
        int penetration_from_right = solid.x + solid.w - moving.x;
        if (part.exposed_left && moving.x < solid.x && penetration_from_left > 0 && penetration_from_left <= SIDE_PENETRATION_TOLERANCE) return COLLISION_RIGHT;
        if (part.exposed_right && moving.x + moving.w > solid.x + solid.w && penetration_from_right > 0 && penetration_from_right <= SIDE_PENETRATION_TOLERANCE) return COLLISION_LEFT;
        if (part.exposed_left && abs(moving.right_center.x - solid.left_center.x) < SIDE_COLLISION_TOLERANCE) return COLLISION_RIGHT;
        if (part.exposed_right && abs(moving.left_center.x - solid.right_center.x) < SIDE_COLLISION_TOLERANCE) return COLLISION_LEFT;
        return COLLISION_NONE;
    }

    bool isLandingOnTop(const Rectangle &moving, const Rectangle &solid)
    {
        return moving.bottom_center.y <= solid.top_center.y && abs(moving.bottom_center.y - solid.top_center.y) < BOTTOM_COLLISION_TOLERANCE;
    }

    bool isShallowTopPenetration(Object *obj, const Rectangle &moving, const Rectangle &solid)
    {
        if (!canUseTopPenetrationResolution(obj)) return false;
        int penetration = moving.y + moving.h - solid.y;
        return moving.y < solid.y && penetration > 0 && penetration <= TOP_PENETRATION_TOLERANCE;
    }

    bool detectBottomCollision(const Rectangle &moving, const Rectangle &solid)
    {
        int left_probe = moving.left_bottom.x + EDGE_INSET;
        int right_probe = moving.right_bottom.x - EDGE_INSET;
        return pointInsideHorizontalSpan(moving.bottom_center.x, solid) || pointInsideHorizontalSpan(left_probe, solid) || pointInsideHorizontalSpan(right_probe, solid);
    }

    bool hitsSolidCeiling(const Rectangle &moving, const Rectangle &solid)
    {
        return moving.top_center.y >= solid.bottom_center.y && moving.top_center.y - solid.bottom_center.y < TOP_COLLISION_TOLERANCE;
    }

    bool detectTopCenterCollision(const Rectangle &moving, const Rectangle &solid){ return moving.top_center.x >= solid.left_top.x && moving.top_center.x <= solid.right_top.x; }
    bool detectTopLeftCornerCollision(const Rectangle &moving, const Rectangle &solid){ int left_probe = moving.left_top.x + EDGE_INSET; return left_probe >= solid.left_top.x && left_probe <= solid.right_top.x; }
    bool detectTopRightCornerCollision(const Rectangle &moving, const Rectangle &solid){ int right_probe = moving.right_top.x - EDGE_INSET; return right_probe >= solid.left_top.x && right_probe <= solid.right_top.x; }

    void separateSideCollision(Object *moving_obj, const Rectangle &solid, CollisionSide side)
    {
        Point pos = moving_obj->getPos(); Point size = moving_obj->getSize();
        if (side == COLLISION_RIGHT) moving_obj->setPos(solid.x - size.x, pos.y);
        else if (side == COLLISION_LEFT) moving_obj->setPos(solid.x + solid.w, pos.y);
    }

    void separateTopCollision(Object *moving_obj, const Rectangle &solid)
    {
        Point pos = moving_obj->getPos(); Point size = moving_obj->getSize();
        moving_obj->setPos(pos.x, solid.y - size.y);
    }

    void notifySideCollision(Object *moving_obj, Object *solid_obj, const Rectangle &solid_rect, CollisionSide side)
    {
        if (side == COLLISION_RIGHT)
        {
            if (moving_obj->getType() == PLAYER) separateSideCollision(moving_obj, solid_rect, side);
            moving_obj->notifyCollisionRight(solid_obj); solid_obj->notifyCollisionLeft(moving_obj);
        }
        else if (side == COLLISION_LEFT)
        {
            if (moving_obj->getType() == PLAYER) separateSideCollision(moving_obj, solid_rect, side);
            moving_obj->notifyCollisionLeft(solid_obj); solid_obj->notifyCollisionRight(moving_obj);
        }
    }

    void calculateExposedFaces(CollisionBody &body)
    {
        for (unsigned int i = 0; i < body.parts.size(); i++)
        {
            CollisionPart &part = body.parts[i];
            part.exposed_top = part.exposed_bottom = part.exposed_left = part.exposed_right = true;
            for (unsigned int j = 0; j < body.parts.size(); j++)
            {
                if (i == j) continue;
                const Rectangle &other = body.parts[j].rect;
                if (other.x + other.w == part.rect.x && verticalOverlap(part.rect, other)) part.exposed_left = false;
                if (part.rect.x + part.rect.w == other.x && verticalOverlap(part.rect, other)) part.exposed_right = false;
                if (other.y + other.h == part.rect.y && horizontalOverlap(part.rect, other)) part.exposed_top = false;
                if (part.rect.y + part.rect.h == other.y && horizontalOverlap(part.rect, other)) part.exposed_bottom = false;
            }
        }
    }
}

World::World()
{
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
    this->gameState = new GameState();
    rebuildCollisionBodies();
}

void World::loop()
{
    rebuildCollisionBodies();
    ghostCollector();
    collision(map->player);

    for (unsigned int i = 0; i < map->objects.size(); i++)
    {
        Type t = map->objects[i]->getType();
        if (t == PLAYER) continue;
        else if (t == BRICK)
        {
            ((Brick *)map->objects[i])->update();
            if (((Brick *)map->objects[i])->broken){ map->objects.erase(map->objects.begin() + i); rebuildCollisionBodies(); }
        }
        else if (t == COIN_CONTAINER) ((CoinContainer *)map->objects[i])->update();
        else if (t == FIRE_CONTAINER) ((FireContainer *)map->objects[i])->update();
        else if (t == HEALTH_CONTAINER) ((HealthContainer *)map->objects[i])->update();
        else if (t == GOOMBA)
        {
            if ((map->objects[i]->getPos() + camera->getPos()).x < 700) ((Goomba *)map->objects[i])->seen();
            collision(map->objects[i]);
            if (map->objects[i]->getPos().y > 500){ map->objects.erase(map->objects.begin() + i); rebuildCollisionBodies(); continue; }
            ((Goomba *)map->objects[i])->update();
        }
        else if (t == KOOPA)
        {
            if ((map->objects[i]->getPos() + camera->getPos()).x < 700) ((Koopa *)map->objects[i])->seen();
            collision(map->objects[i]);
            if (map->objects[i]->getPos().y > 500){ map->objects.erase(map->objects.begin() + i); rebuildCollisionBodies(); continue; }
            ((Koopa *)map->objects[i])->update();
        }
    }

    for (unsigned int i = 0; i < ghosts.size(); i++)
    {
        if (ghosts[i]->ghost_dead){ ghosts.erase(ghosts.begin() + i); continue; }
        if (ghosts[i]->getType() == G_COIN) ((Coin *)ghosts[i])->update();
        else if (ghosts[i]->getType() == G_TEXT) ((Text *)ghosts[i])->update();
        else if (ghosts[i]->getType() == G_MUSHROOM){ collision(ghosts[i]); ((Mushroom *)ghosts[i])->update(); }
        else if (ghosts[i]->getType() == G_FLOWER){ collision(ghosts[i]); ((Flower *)ghosts[i])->update(); }
        else if (ghosts[i]->getType() == G_BULLET){ collision(ghosts[i]); ((Bullet *)ghosts[i])->update(); }
    }
}

void World::ghostCollector()
{
    for (auto obj : map->objects)
    {
        if (!obj->has_ghost) continue;
        for (auto g : obj->ghost)
        {
            ghosts.push_back(g);
            if (g->getType() == G_TEXT) gameState->score += ((Text *)g)->score;
        }
        obj->has_ghost = false;
        obj->ghost.clear();
    }
}

void World::addGhost(Object *obj){ ghosts.push_back(obj); }
std::vector<Object *> World::getObjects(){ return map->objects; }
std::vector<CollisionBody> World::getCollisionBodies(){ return collisionBodies; }
GameState *World::getGameState(){ return gameState; }
Player *World::getPlayer(){ return map->player; }
std::vector<Object *> World::getGhosts(){ return ghosts; }

void World::rebuildCollisionBodies()
{
    collisionBodies.clear();
    std::vector<Object *> tiles;
    for (auto obj : map->objects) if (!obj->dead && isStaticCollisionTile(obj)) tiles.push_back(obj);

    std::vector<bool> visited(tiles.size(), false);
    for (unsigned int i = 0; i < tiles.size(); i++)
    {
        if (visited[i]) continue;
        Type type = tiles[i]->getType();
        CollisionBody body(type, objectRect(tiles[i]));
        std::queue<unsigned int> q; q.push(i); visited[i] = true;

        while (!q.empty())
        {
            unsigned int idx = q.front(); q.pop();
            Object *tile = tiles[idx];
            Rectangle tile_rect = objectRect(tile);
            body.parts.push_back(CollisionPart(tile, tile_rect));
            body.bounds = mergeBounds(body.bounds, tile_rect);

            for (unsigned int j = 0; j < tiles.size(); j++)
            {
                if (visited[j] || tiles[j]->getType() != type) continue;
                Rectangle candidate_rect = objectRect(tiles[j]);
                if (rectanglesTouchOrOverlap(tile_rect, candidate_rect)){ visited[j] = true; q.push(j); }
            }
        }

        calculateExposedFaces(body);
        collisionBodies.push_back(body);
    }
}

void World::hitEnemiesAbove(Object *platform)
{
    if (!canBumpEnemies(platform)) return;
    Rectangle platform_rect(platform->getPos(), platform->getPos() + platform->getSize());
    for (auto obj : map->objects)
    {
        if (obj->dead || !isEnemy(obj)) continue;
        Rectangle enemy_rect(obj->getPos(), obj->getPos() + obj->getSize());
        if (isStandingOnPlatform(enemy_rect, platform_rect)) obj->death();
    }
}

void World::collision(Object *obj)
{
    if (obj->dead) return;

    bool on_the_floor = false;
    bool top_center_collision = false;
    bool top_corner_collision = false;
    Object *top_corner_object = nullptr;

    obj->notifyFreeLeft();
    obj->notifyFreeRight();

    auto processCollisionTarget = [&](const CollisionPart &part) -> bool {
        Object *target = part.object;
        const Rectangle &solid_rect = part.rect;
        if (target->dead || target == obj || shouldIgnoreEnemyCollision(obj, target)) return false;

        Rectangle moving_rect(obj->getPos(), obj->getPos() + obj->getSize());
        notifySideCollision(obj, target, solid_rect, detectSideCollision(moving_rect, part));

        moving_rect = Rectangle(obj->getPos(), obj->getPos() + obj->getSize());
        if (part.exposed_top && detectBottomCollision(moving_rect, solid_rect))
        {
            if (isLandingOnTop(moving_rect, solid_rect) || isShallowTopPenetration(obj, moving_rect, solid_rect))
            {
                separateTopCollision(obj, solid_rect);
                on_the_floor = true;
                obj->notifyCollisionBottom(target);
                moving_rect = Rectangle(obj->getPos(), obj->getPos() + obj->getSize());
            }
            else obj->notifyDistToPlatform(abs(moving_rect.bottom_center.y - solid_rect.top_center.y));
        }

        if (!supportsTopCollision(obj)) return false;

        bool top_center_on_part = detectTopCenterCollision(moving_rect, solid_rect);
        bool top_corner_on_part = detectTopLeftCornerCollision(moving_rect, solid_rect) || detectTopRightCornerCollision(moving_rect, solid_rect);

        if (part.exposed_bottom && top_center_on_part)
        {
            if (hitsSolidCeiling(moving_rect, solid_rect))
            {
                obj->notifyCollisionTop(target);
                if (obj->getType() == PLAYER) hitEnemiesAbove(target);
                top_center_collision = true;
                top_corner_collision = true;
                return true;
            }
            obj->notifyDistToCeil(abs(moving_rect.top_center.y - solid_rect.bottom_center.y));
            return false;
        }

        if (top_center_collision || !part.exposed_bottom || !top_corner_on_part) return false;

        if (hitsSolidCeiling(moving_rect, solid_rect))
        {
            if (shouldApplyTopCornerCollision(obj)){ top_corner_collision = true; top_corner_object = target; }
        }
        else obj->notifyDistToCeil(abs(moving_rect.top_center.y - solid_rect.bottom_center.y));

        return false;
    };

    Rectangle moving_rect(obj->getPos(), obj->getPos() + obj->getSize());
    for (auto &body : collisionBodies)
    {
        if (!expandedBoundsOverlap(moving_rect, body.bounds)) continue;
        for (auto &part : body.parts) if (processCollisionTarget(part)) return;
    }

    for (auto o : getObjects())
    {
        if (isStaticCollisionTile(o)) continue;
        CollisionPart dynamic_part(o, objectRect(o));
        if (processCollisionTarget(dynamic_part)) return;
    }

    if (!on_the_floor) obj->notifyFreeBottom();

    if (supportsTopCollision(obj) && !top_center_collision && top_corner_collision && top_corner_object != nullptr)
    {
        obj->notifyCollisionTop(top_corner_object);
        if (obj->getType() == PLAYER) hitEnemiesAbove(top_corner_object);
    }
}
