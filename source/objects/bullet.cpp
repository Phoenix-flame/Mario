#include "bullet.hpp"
#include "koopa_troopa.hpp"
#include "text.hpp"

namespace
{
    const int BULLET_KILL_SCORE = 150;

    bool isBulletKillTarget(Object *obj)
    {
        return obj->getType() == GOOMBA || obj->getType() == KOOPA;
    }

    void rewardBulletKill(Object *obj)
    {
        Text *text = new Text(obj->getPos().x, obj->getPos().y - 10);
        text->setPos(obj->getPos().x, obj->getPos().y - 10);
        text->ghost_dead = false;
        text->text = "+ 150";
        text->score = BULLET_KILL_SCORE;

        obj->ghost.push_back(text);
        obj->has_ghost = true;
    }
}

Bullet::Bullet(int x, int y, Dir _dir) : Object(Point(0, 0), Point(16, 16), BULLET_IMAGE, G_BULLET)
{
    pos = Point(x, y);
    size = Point(16, 16);
    xMin = pos.x;
    xMax = pos.x + size.x;
    yMin = pos.y;
    yMax = pos.y + size.y;

    dir = (_dir == LEFT) ? LEFT : RIGHT;
    speed = (dir == LEFT) ? -6 : 6;
    vertical_speed = 2;
}

void Bullet::update()
{
    _moveX(speed);
    _moveY(vertical_speed);

    vertical_speed += 1;
    if (vertical_speed > terminal_speed)
    {
        vertical_speed = terminal_speed;
    }
    fall_cycles += 1;

    if (pos.y > 500 || pos.x < -100 || pos.x > 5200)
    {
        ghost_dead = true;
    }
}

void Bullet::hit(Object *obj)
{
    if (obj->getType() == PLAYER || obj->getType() == G_BULLET)
    {
        return;
    }

    if (isBulletKillTarget(obj) && !obj->dead)
    {
        rewardBulletKill(obj);
    }

    if (obj->getType() == GOOMBA)
    {
        obj->death();
    }
    else if (obj->getType() == KOOPA)
    {
        ((Koopa *)obj)->fireballDeath();
    }

    ghost_dead = true;
}

void Bullet::notifyCollisionLeft(Object *obj)
{
    hit(obj);
}

void Bullet::notifyCollisionRight(Object *obj)
{
    hit(obj);
}

void Bullet::notifyCollisionTop(Object *obj)
{
    hit(obj);
}

void Bullet::notifyCollisionBottom(Object *obj)
{
    if (obj->getType() == PLAYER || obj->getType() == GOOMBA || obj->getType() == KOOPA)
    {
        hit(obj);
        return;
    }

    vertical_speed = -5;
}

void Bullet::notifyFreeBottom()
{
}

void Bullet::notifyDistToPlatform(int d)
{
    if (vertical_speed > d)
    {
        vertical_speed = d;
    }
}
