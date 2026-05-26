#include "koopa_troopa.hpp"
#include "text.hpp"

namespace
{
    const int SHELL_KILL_SCORE = 150;

    bool isShellKillTarget(Object *obj)
    {
        return obj->getType() == GOOMBA || obj->getType() == KOOPA;
    }

    void rewardShellKill(Object *source, Object *target)
    {
        Text *text = new Text(target->getPos().x, target->getPos().y - 10);
        text->setPos(target->getPos().x, target->getPos().y - 10);
        text->ghost_dead = false;
        text->text = "+ " + std::to_string(SHELL_KILL_SCORE);
        text->score = SHELL_KILL_SCORE;

        source->ghost.push_back(text);
        source->has_ghost = true;
    }

    void killWithShell(Object *source, Object *target)
    {
        if (target == source || target->dead || !isShellKillTarget(target))
        {
            return;
        }

        rewardShellKill(source, target);
        if (target->getType() == KOOPA)
        {
            ((Koopa *)target)->fireballDeath();
        }
        else
        {
            target->death();
        }
    }
}

void Koopa::update()
{
    if (!visited && state != KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }

    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        fireballDeathAnimation();
        updateFigure();
        return;
    }

    if (state == KOOPA_ATTACK_STATE)
    {
        updateFigure();
        return;
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE)
    {
        if (dir == LEFT)
        {
            _moveX(-4);
        }
        else if (dir == RIGHT)
        {
            _moveX(+4);
        }
        if (state == KOOPA_FALL_STATE)
        {
            falling();
        }

        updateFigure();
        return;
    }

    // Normal state
    if (dir == LEFT)
    {
        _moveX(-1);
    }
    else if (dir == RIGHT)
    {
        _moveX(+1);
    }

    if (state == KOOPA_FALL_STATE)
    {
        falling();
    }

    updateFigure();
}

void Koopa::updateFigure()
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        image = KOOPA_DEAD;
    }
    else if (state == KOOPA_WALK_STATE)
    {
        if (dir == RIGHT)
        {
            if (right_walk_state == 1)
            {
                image = KOOPA_WALK_RIGHT1;
                if (!RW_Timer.isStarted())
                {
                    RW_Timer.start();
                }
                if (RW_Timer.getTime() > 80)
                {
                    right_walk_state = 2;
                    RW_Timer.reset();
                }
            }
            else if (right_walk_state == 2)
            {
                image = KOOPA_WALK_RIGHT2;
                if (!RW_Timer.isStarted())
                {
                    RW_Timer.start();
                }
                if (RW_Timer.getTime() > 80)
                {
                    right_walk_state = 1;
                    RW_Timer.reset();
                }
            }
        }
        else if (dir == LEFT)
        {
            if (left_walk_state == 1)
            {
                image = KOOPA_WALK_LEFT1;
                if (!LW_Timer.isStarted())
                {
                    LW_Timer.start();
                }
                if (LW_Timer.getTime() > 200)
                {
                    left_walk_state = 2;
                    LW_Timer.reset();
                }
            }
            else if (left_walk_state == 2)
            {
                image = KOOPA_WALK_LEFT2;
                if (!LW_Timer.isStarted())
                {
                    LW_Timer.start();
                }
                if (LW_Timer.getTime() > 200)
                {
                    left_walk_state = 1;
                    LW_Timer.reset();
                }
            }
        }
    }
    else if (state == KOOPA_ATTACK_STATE || state == KOOPA_FAST_AND_FURIOUS_STATE)
    {
        image = KOOPA_DEAD;
    }
}

void Koopa::startFall()
{
    if (state == KOOPA_ATTACK_STATE || state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    state = KOOPA_FALL_STATE;
    fall_speed_vertical = 3;
}

void Koopa::falling()
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

void Koopa::endFall()
{
    if (state == KOOPA_ATTACK_STATE || state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    if (state == KOOPA_FALL_STATE)
    {
        state = KOOPA_WALK_STATE;
    }
}

void Koopa::death()
{
    // Very simple death
    dead = true;
    if (state == KOOPA_WALK_STATE)
    {
        state = KOOPA_ATTACK_STATE;
    }
    else if (state == KOOPA_ATTACK_STATE)
    {
        attack = true;
        dir = RIGHT;
        state = KOOPA_FAST_AND_FURIOUS_STATE;
    }
}

void Koopa::fireballDeath()
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }

    Object::dead = true;
    dead = true;
    visited = true;
    state = KOOPA_FIREBALL_DEATH_STATE;
    image = KOOPA_DEAD;
    fall_speed_vertical = -7;
    fall_speed_horizontal = (dir == LEFT) ? -1 : 1;
    fall_cycles = 0;
}

void Koopa::fireballDeathAnimation()
{
    _moveY(fall_speed_vertical);
    _moveX(fall_speed_horizontal);

    fall_speed_vertical += (fall_cycles % 2 == 0) ? 1 : 0;
    if (fall_speed_vertical > 12)
    {
        fall_speed_vertical = 12;
    }
    fall_cycles += 1;

    if (pos.y > 520)
    {
        ghost_dead = true;
    }
}

// Collision Notification
void Koopa::notifyCollisionLeft(Object *obj)
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    if (obj->getType() == G_BULLET)
    {
        fireballDeath();
        return;
    }
    if (state == KOOPA_WALK_STATE)
    {
        if (obj->getType() == PLAYER)
        {
            obj->death();
        }
        else if (dir == LEFT)
        {
            dir = RIGHT;
        }
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE)
    {
        if (isShellKillTarget(obj))
        {
            killWithShell(this, obj);
        }
        else if (obj->getType() == PLAYER)
        {
            obj->death();
        }
        else
        {
            if (dir == LEFT)
            {
                dir = RIGHT;
            }
        }
    }
}
void Koopa::notifyCollisionRight(Object *obj)
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    if (obj->getType() == G_BULLET)
    {
        fireballDeath();
        return;
    }
    if (state == KOOPA_WALK_STATE)
    {
        if (obj->getType() == PLAYER)
        {
            obj->death();
        }
        else if (dir == RIGHT)
        {
            dir = LEFT;
        }
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE)
    {
        if (isShellKillTarget(obj))
        {
            killWithShell(this, obj);
        }
        else if (obj->getType() == PLAYER)
        {
            obj->death();
        }
        else
        {
            if (dir == RIGHT)
            {
                dir = LEFT;
            }
        }
    }
}
void Koopa::notifyCollisionTop(Object *obj)
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    if (obj->getType() == G_BULLET)
    {
        fireballDeath();
    }
}
void Koopa::notifyCollisionBottom(Object *obj)
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    if (obj->getType() == G_BULLET)
    {
        fireballDeath();
        return;
    }
    endFall();
}

void Koopa::notifyFreeLeft() {}
void Koopa::notifyFreeRight() {}
void Koopa::notifyFreeTop() {}
void Koopa::notifyFreeBottom()
{
    if (state != KOOPA_FALL_STATE && state != KOOPA_FIREBALL_DEATH_STATE)
    {
        startFall();
    }
}

void Koopa::notifyDistToPlatform(int d)
{
    if (state == KOOPA_FIREBALL_DEATH_STATE)
    {
        return;
    }
    if (fall_speed_vertical > d)
    {
        fall_speed_vertical = d;
    }
}

void Koopa::notifyDistToCeil(int d)
{
}
