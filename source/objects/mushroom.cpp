#include "mushroom.hpp"

void Mushroom::update()
{
    if (pos.x > x_gravity_en)
    {
        gravity_en = true;
    }

    if (state == M_RISE)
    {
        funcToRun = &Mushroom::riseLikeThePhoenix;
    }
    else if (state == M_FALL)
    {
        funcToRun = &Mushroom::falling;
    }
    else if (state == M_RUN)
    {
        funcToRun = &Mushroom::run;
    }

    (this->*funcToRun)();
}

void Mushroom::riseLikeThePhoenix()
{
    if (pos.y > y_target)
    {
        _moveY(-1);
    }
    else
    {
        pos.y = y_target;
        state = M_RUN;
        funcToRun = &Mushroom::run;
    }
}

void Mushroom::run()
{
    _moveX(speed);
}

void Mushroom::bumpUp()
{
    if (state == M_RISE)
    {
        return;
    }

    gravity_en = true;
    state = M_FALL;
    fall_speed_vertical = -5;
    fall_cycles = 0;
}

void Mushroom::startFall()
{
    state = M_FALL;
    if (fall_speed_vertical < 1)
    {
        fall_speed_vertical = 1;
    }
    fall_cycles = 0;
}

void Mushroom::falling()
{
    _moveY(fall_speed_vertical);
    _moveX(speed);

    fall_speed_vertical += (fall_cycles % 2 == 0) ? 1 : 0;
    if (fall_speed_vertical > terminal_speed)
    {
        fall_speed_vertical = terminal_speed;
    }
    fall_cycles += 1;
}

void Mushroom::endFall()
{
    state = M_RUN;
    fall_speed_vertical = 0;
    fall_cycles = 0;
}

bool Mushroom::isStaticPlatform(Object *obj)
{
    Type t = obj->getType();
    return t == BLOCK || t == BRICK || t == GROUND || t == PIPE ||
           t == COIN_CONTAINER || t == FIRE_CONTAINER || t == HEALTH_CONTAINER;
}

bool Mushroom::isFallingBesidePlatformEdge(Object *obj)
{
    if (state != M_FALL || !isStaticPlatform(obj))
    {
        return false;
    }

    Rectangle mushroom_rect(pos, pos + size);
    Rectangle platform_rect(obj->getPos(), obj->getPos() + obj->getSize());

    bool near_platform_top = mushroom_rect.bottom_center.y <= platform_rect.top_center.y + terminal_speed;
    bool below_platform_top = mushroom_rect.bottom_center.y > platform_rect.top_center.y;

    return near_platform_top || below_platform_top;
}

// Collision Notification
void Mushroom::notifyCollisionLeft(Object *obj)
{
    Type t = obj->getType();
    if (t == KOOPA || t == GOOMBA)
    {
        return;
    }
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
    if (state == M_RISE && isStaticPlatform(obj))
    {
        return;
    }
    if (isFallingBesidePlatformEdge(obj))
    {
        return;
    }
    speed *= -1;
}
void Mushroom::notifyCollisionRight(Object *obj)
{
    Type t = obj->getType();
    if (t == KOOPA || t == GOOMBA)
    {
        return;
    }
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
    if (state == M_RISE && isStaticPlatform(obj))
    {
        return;
    }
    if (isFallingBesidePlatformEdge(obj))
    {
        return;
    }
    speed *= -1;
}
void Mushroom::notifyCollisionTop(Object *obj)
{
    Type t = obj->getType();
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
    if (state == M_RISE && isStaticPlatform(obj))
    {
        return;
    }
}
void Mushroom::notifyCollisionBottom(Object *obj)
{
    Type t = obj->getType();
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }

    if (!isStaticPlatform(obj) || state == M_RISE || fall_speed_vertical < 0)
    {
        return;
    }

    endFall();
}

void Mushroom::notifyFreeLeft() {}
void Mushroom::notifyFreeRight() {}
void Mushroom::notifyFreeTop() {}
void Mushroom::notifyFreeBottom()
{
    if (!gravity_en || state == M_RISE)
    {
        return;
    }
    if (state != M_FALL)
    {
        startFall();
    }
}

void Mushroom::notifyDistToPlatform(int d)
{
    if (state == M_FALL && d >= 0 && fall_speed_vertical > d)
    {
        fall_speed_vertical = d;
    }
}

void Mushroom::notifyDistToCeil(int d)
{
}
