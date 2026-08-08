#include "goomba.hpp"

Goomba::Goomba(int x, int y) : Object(Point(x, y),
                                      Point(24, 24),
                                      GOOMBA_WALK1_IMAGE,
                                      GOOMBA)
{

    state = GOOMBA_WALK_STATE;
    speed = -1;
    dir = LEFT;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(1, 2);
    walk_state = distribution(generator);
}

void Goomba::update()
{
    if (!visited)
    {
        return;
    }

    if (state == GOOMBA_DEAD_STATE)
    {
        funcToRun = &Goomba::death_animation;
    }
    else
    {

        funcToRun = &Goomba::normal_behavior;
    }

    // Update Player State
    (this->*funcToRun)();
    updateFigure();
}

void Goomba::normal_behavior()
{
    _moveX(speed);

    if (state == GOOMBA_FALL_STATE)
    {
        falling();
    }
}

void Goomba::updateFigure()
{
    if (state == GOOMBA_WALK_STATE)
    {
        if (walk_state == 1)
        {
            image = GOOMBA_WALK1_IMAGE;
            if (!walkTimer.isStarted())
            {
                walkTimer.start();
            }
            if (walkTimer.getTime() > 200)
            {
                walk_state = 2;
                walkTimer.reset();
            }
        }
        else if (walk_state == 2)
        {
            image = GOOMBA_WALK2_IMAGE;
            if (!walkTimer.isStarted())
            {
                walkTimer.start();
            }
            if (walkTimer.getTime() > 200)
            {
                walk_state = 1;
                walkTimer.reset();
            }
        }
    }
    else if (state == GOOMBA_DEAD_STATE)
    {
        image = GOOMBA_DEAD_IMAGE;
    }
}

void Goomba::startFall()
{
    state = GOOMBA_FALL_STATE;
    fall_speed_vertical = 3;
}

void Goomba::falling()
{
    _moveY(fall_speed_vertical);
    fall_speed_vertical += (fall_cycles % 2 == 0) ? 1 : 0;
    if (fall_speed_vertical >= 10)
    {
        fall_speed_vertical = 10;
    }
    fall_cycles += 1;
    _moveX(0);
}

void Goomba::endFall()
{
    if (state == GOOMBA_FALL_STATE)
    {
        state = GOOMBA_WALK_STATE;
    }
}

void Goomba::death_animation()
{
    if (!deathAnimation.isStarted())
    {
        deathAnimation.start();
    }
    if (deathAnimation.getTime() < 100)
    {
        fall_speed_vertical = 3;
        _moveY(-5);
    }
    else if (deathAnimation.getTime() >= 200)
    {
        falling();
    }
    updateFigure();
}

void Goomba::death()
{
    // Very simple death
    state = GOOMBA_DEAD_STATE;
    dead = true; // Don't check collisions anymore
}

// Collision Notification
void Goomba::notifyCollisionLeft(Object *obj)
{
    if (state == GOOMBA_DEAD_STATE)
    {
        return;
    }
    if (state == GOOMBA_WALK_STATE)
    {
        if (dir == LEFT)
        {
            if (obj->getType() == PLAYER)
            {
                obj->death();
            }
            else
            {
                dir = RIGHT;
                speed *= -1;
            }
        }
    }
}
void Goomba::notifyCollisionRight(Object *obj)
{
    if (state == GOOMBA_DEAD_STATE)
    {
        return;
    }
    if (state == GOOMBA_WALK_STATE)
    {
        if (dir == RIGHT)
        {
            if (obj->getType() == PLAYER)
            {
                obj->death();
            }
            else
            {
                dir = LEFT;
                speed *= -1;
            }
        }
    }
}
void Goomba::notifyCollisionTop(Object *obj)
{
}
void Goomba::notifyCollisionBottom(Object *obj)
{
    if (state == GOOMBA_DEAD_STATE)
    {
        return;
    }
    if (state != GOOMBA_DEAD_STATE)
        endFall();
    if (obj->getType() == PLAYER)
    {
        obj->death();
    }
}

void Goomba::notifyFreeLeft() {}
void Goomba::notifyFreeRight() {}
void Goomba::notifyFreeTop() {}
void Goomba::notifyFreeBottom()
{
    if (state == GOOMBA_DEAD_STATE)
    {
        return;
    }
    if (state != GOOMBA_FALL_STATE)
    {
        startFall();
    }
}

void Goomba::notifyDistToPlatform(int d)
{
    if (state == GOOMBA_DEAD_STATE)
    {
        return;
    }
    if (fall_speed_vertical > d)
    {
        fall_speed_vertical = d;
    }
}

void Goomba::notifyDistToCeil(int d)
{
}