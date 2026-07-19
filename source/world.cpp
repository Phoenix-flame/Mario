#include "world.hpp"
#include <algorithm>

namespace
{
    const int BLOCK_BUMP_ENEMY_Y_TOLERANCE = 6;
    const int ENEMY_KILL_SCORE = 150;

    bool hasHorizontalOverlap(const Rectangle &a, const Rectangle &b)
    {
        return a.left_top.x < b.right_top.x && a.right_top.x > b.left_top.x;
    }

    bool horizontalOverlap(const Rectangle &a, const Rectangle &b)
    {
        return a.x < b.x + b.w && a.x + a.w > b.x;
    }

    bool verticalOverlap(const Rectangle &a, const Rectangle &b)
    {
        return a.y < b.y + b.h && a.y + a.h > b.y;
    }

    bool rectanglesTouchOrOverlap(const Rectangle &a, const Rectangle &b)
    {
        bool horizontal_touch = a.x <= b.x + b.w && a.x + a.w >= b.x;
        bool vertical_touch = a.y <= b.y + b.h && a.y + a.h >= b.y;
        return horizontal_touch && vertical_touch;
    }

    Rectangle objectRect(Object *obj)
    {
        return Rectangle(obj->getPos(), obj->getPos() + obj->getSize());
    }

    Rectangle mergeBounds(const Rectangle &a, const Rectangle &b)
    {
        int left = std::min(a.x, b.x);
        int top = std::min(a.y, b.y);
        int right = std::max(a.x + a.w, b.x + b.w);
        int bottom = std::max(a.y + a.h, b.y + b.h);
        return Rectangle(Point(left, top), Point(right, bottom));
    }

    bool isEnemy(Object *obj)
    {
        return obj->getType() == GOOMBA || obj->getType() == KOOPA;
    }

    bool isStaticCollisionTile(Object *obj)
    {
        Type t = obj->getType();
        return t == BLOCK || t == BRICK || t == GROUND || t == PIPE ||
               t == COIN_CONTAINER || t == FIRE_CONTAINER || t == HEALTH_CONTAINER;
    }

    bool isStandingOnPlatform(const Rectangle &enemy, const Rectangle &platform)
    {
        return hasHorizontalOverlap(enemy, platform) &&
               abs(enemy.bottom_center.y - platform.top_center.y) <= BLOCK_BUMP_ENEMY_Y_TOLERANCE;
    }

    bool canBumpEnemies(Object *obj)
    {
        Type t = obj->getType();
        return t == BRICK || t == COIN_CONTAINER || t == FIRE_CONTAINER || t == HEALTH_CONTAINER ||
               t == BLOCK || t == GROUND;
    }

    Text *makeScoreText(Object *obj, int score)
    {
        Text *text = new Text(obj->getPos().x, obj->getPos().y - 10);
        text->setPos(obj->getPos().x, obj->getPos().y - 10);
        text->ghost_dead = false;
        text->text = "+ " + std::to_string(score);
        text->score = score;
        return text;
    }

    void calculateExposedFaces(CollisionBody &body)
    {
        for (unsigned int i = 0; i < body.parts.size(); i++)
        {
            CollisionPart &part = body.parts[i];
            part.exposed_top = true;
            part.exposed_bottom = true;
            part.exposed_left = true;
            part.exposed_right = true;

            for (unsigned int j = 0; j < body.parts.size(); j++)
            {
                if (i == j)
                {
                    continue;
                }

                const Rectangle &other = body.parts[j].rect;
                if (other.x + other.w == part.rect.x && verticalOverlap(part.rect, other))
                {
                    part.exposed_left = false;
                }
                if (part.rect.x + part.rect.w == other.x && verticalOverlap(part.rect, other))
                {
                    part.exposed_right = false;
                }
                if (other.y + other.h == part.rect.y && horizontalOverlap(part.rect, other))
                {
                    part.exposed_top = false;
                }
                if (part.rect.y + part.rect.h == other.y && horizontalOverlap(part.rect, other))
                {
                    part.exposed_bottom = false;
                }
            }
        }
    }
}

World::World(int _level, const std::string &assetRoot)
{
    this->level = _level;
    this->camera = new Camera();
    std::string prefix = assetRoot;
    if (!prefix.empty() && prefix[prefix.size() - 1] != '/')
    {
        prefix += "/";
    }
    std::string mapPath = prefix + "assets/maps/1/" + std::to_string(_level) + ".txt";
    this->map = new Map(mapPath);
    this->gameState = new GameState();
    this->gameState->currentLevel = _level;
    this->physics = new Physics();

    int mapPixelWidth = map->getWidth() * 24;
    int maxScroll = -(mapPixelWidth - 640);
    camera->setMaxScroll(maxScroll);

    this->winX = map->getFlagX();
    if (this->winX <= 0)
    {
        this->winX = mapPixelWidth - 120;
    }
    map->player->setWinX(this->winX - 48);

    rebuildCollisionBodies();
}

World::~World()
{
    for (auto ghost : ghosts)
    {
        delete ghost;
    }
    delete physics;
    delete gameState;
    delete map;
    delete camera;
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
                delete map->objects[i];
                map->objects.erase(map->objects.begin() + i);
                i--;
                rebuildCollisionBodies();
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
                delete map->objects[i];
                map->objects.erase(map->objects.begin() + i);
                i--;
                rebuildCollisionBodies();
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
                delete map->objects[i];
                map->objects.erase(map->objects.begin() + i);
                i--;
                rebuildCollisionBodies();
                continue;
            }

            ((Koopa *)map->objects[i])->update();
        }
    }

    for (unsigned int i = 0; i < ghosts.size(); i++)
    {
        if (ghosts[i]->ghost_dead)
        {
            delete ghosts[i];
            ghosts.erase(ghosts.begin() + i);
            i--;
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

std::vector<CollisionBody> World::getCollisionBodies()
{
    return collisionBodies;
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

void World::rebuildCollisionBodies()
{
    collisionBodies.clear();

    std::vector<Object *> tiles;
    for (auto obj : map->objects)
    {
        if (!obj->dead && isStaticCollisionTile(obj))
        {
            tiles.push_back(obj);
        }
    }

    std::vector<bool> visited(tiles.size(), false);
    for (unsigned int i = 0; i < tiles.size(); i++)
    {
        if (visited[i])
        {
            continue;
        }

        Type type = tiles[i]->getType();
        Rectangle first_rect = objectRect(tiles[i]);
        CollisionBody body(type, first_rect);

        std::queue<unsigned int> q;
        q.push(i);
        visited[i] = true;

        while (!q.empty())
        {
            unsigned int idx = q.front();
            q.pop();

            Object *tile = tiles[idx];
            Rectangle tile_rect = objectRect(tile);
            body.parts.push_back(CollisionPart(tile, tile_rect));
            body.bounds = mergeBounds(body.bounds, tile_rect);

            for (unsigned int j = 0; j < tiles.size(); j++)
            {
                if (visited[j] || tiles[j]->getType() != type)
                {
                    continue;
                }

                Rectangle candidate_rect = objectRect(tiles[j]);
                if (rectanglesTouchOrOverlap(tile_rect, candidate_rect))
                {
                    visited[j] = true;
                    q.push(j);
                }
            }
        }

        calculateExposedFaces(body);
        collisionBodies.push_back(body);
    }
}

void World::hitEnemiesAbove(Object *platform)
{
    if (!canBumpEnemies(platform))
    {
        return;
    }

    Rectangle platform_rect(platform->getPos(), platform->getPos() + platform->getSize());

    for (auto obj : map->objects)
    {
        if (obj->dead || !isEnemy(obj))
        {
            continue;
        }

        Rectangle enemy_rect(obj->getPos(), obj->getPos() + obj->getSize());
        if (isStandingOnPlatform(enemy_rect, platform_rect))
        {
            ghosts.push_back(makeScoreText(obj, ENEMY_KILL_SCORE));
            gameState->score += ENEMY_KILL_SCORE;
            obj->death();
        }
    }
}

void World::collision(Object *obj)
{
    std::vector<Object *> dynamicObjects;
    for (auto o : getObjects())
    {
        if (!isStaticCollisionTile(o))
        {
            dynamicObjects.push_back(o);
        }
    }

    for (auto ghost : ghosts)
    {
        dynamicObjects.push_back(ghost);
    }

    physics->collision(obj, collisionBodies, dynamicObjects, [this](Object *platform) {
        hitEnemiesAbove(platform);
    });
}
