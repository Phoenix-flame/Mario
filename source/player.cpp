#include "player.hpp"

void Player::update(int _dir)
{
    Dir trans_dir = (_dir == 1) ? RIGHT : LEFT;
    bool stop = (_dir == 0) ? true : false;

    // Movement profile
    if (state == DEAD)
    {
        speed = 0;
        funcToRun = &Player::death_animation;
    }

    if (state != DEAD)
    {
        if (state == STAND && stop)
        {
            funcToRun = &Player::stand;
        }
        else if (state == STAND && !stop)
        {
            dir = trans_dir;
            if (dir == RIGHT && collidedR)
            {
                state = STAND;
                speed = 0;
                funcToRun = &Player::stand;
            }
            else if (dir == LEFT && collidedL)
            {
                state = STAND;
                speed = 0;
                funcToRun = &Player::stand;
            }
            else
            {
                startMove();
                funcToRun = &Player::normalMove;
            }
        }
        else if (state == WALK && stop)
        {
            endMove();
            funcToRun = &Player::normalMove;
        }
        else if (state == JUMP)
        {
            if (stop)
            {
                speed = 0;
            }
            else if (!collidedR && !collidedL)
            {
                dir = (stop) ? STOP : trans_dir;
                if (dir == LEFT)
                {
                    speed = -5;
                }
                else if (dir == RIGHT)
                {
                    speed = +5;
                }
                else
                {
                    speed = 0;
                }
            }

            funcToRun = &Player::jump;
        }
        else if (state == FALL)
        {
            if (stop)
            {
                speed = 0;
            }
            else if (!collidedL && !collidedR)
            {
                dir = (stop) ? STOP : trans_dir;
                if (dir == LEFT)
                {
                    speed = -5;
                }
                else if (dir == RIGHT)
                {
                    speed = 5;
                }
                else
                {
                    speed = 0;
                }
            }

            funcToRun = &Player::falling;
        }
    }
    if (state == WON)
    {
        funcToRun = &Player::parade;
    }
    // Update Player State
    (this->*funcToRun)();
    updateFigure();
}

void Player::updateFigure()
{
    static int walk_left = 1;
    static int walk_right = 1;
    switch (level)
    {
    case NORMAL:
        if (state == STAND)
        {
            image = (dir == LEFT) ? NORM_STAND_LEFT : NORM_STAND_RIGHT;
        }
        else if (state == WALK || state == WON)
        {
            if (dir == LEFT)
            {
                walk_right = 1;
                if (walk_left == 1)
                {
                    image = NORM_WALK_LEFT1;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 2;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 2)
                {
                    image = NORM_WALK_LEFT2;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 3;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 3)
                {
                    image = NORM_WALK_LEFT3;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 1;
                        LW_Timer.reset();
                    }
                }
            }
            else if (dir == RIGHT)
            {
                walk_left = 1;
                if (walk_right == 1)
                {
                    image = NORM_WALK_RIGHT1;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 2;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 2)
                {
                    image = NORM_WALK_RIGHT2;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 3;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 3)
                {
                    image = NORM_WALK_RIGHT3;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 1;
                        RW_Timer.reset();
                    }
                }
            }
        }
        else if (state == DEAD)
        {
            image = NORM_DEAD;
        }
        else if (state == JUMP)
        {
            image = (dir == LEFT) ? NORM_JUMP_LEFT : NORM_JUMP_RIGHT;
        }
        else if (state == SLIDE)
        {
            image = (dir == LEFT) ? NORM_SLIDE_LEFT : NORM_SLIDE_RIGHT;
        }
        else if (state == FALL)
        {
            image = (dir == LEFT) ? NORM_JUMP_LEFT : NORM_JUMP_RIGHT;
        }
        break;
    case BIG:
        if (state == STAND)
        {
            image = (dir == LEFT) ? BIG_STAND_LEFT : BIG_STAND_RIGHT;
        }
        else if (state == WALK || state == WON)
        {
            if (dir == LEFT)
            {
                walk_right = 1;
                if (walk_left == 1)
                {
                    image = BIG_WALK_LEFT1;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 2;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 2)
                {
                    image = BIG_WALK_LEFT2;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 3;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 3)
                {
                    image = BIG_WALK_LEFT3;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 1;
                        LW_Timer.reset();
                    }
                }
            }
            else if (dir == RIGHT)
            {
                walk_left = 1;
                if (walk_right == 1)
                {
                    image = BIG_WALK_RIGHT1;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 2;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 2)
                {
                    image = BIG_WALK_RIGHT2;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 3;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 3)
                {
                    image = BIG_WALK_RIGHT3;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 1;
                        RW_Timer.reset();
                    }
                }
            }
        }
        else if (state == DEAD)
        {
        }
        else if (state == JUMP)
        {
            image = (dir == LEFT) ? BIG_JUMP_LEFT : BIG_JUMP_RIGHT;
        }
        else if (state == SLIDE)
        {
            image = (dir == LEFT) ? BIG_SLIDE_LEFT : BIG_SLIDE_RIGHT;
        }
        else if (state == FALL)
        {
            image = (dir == LEFT) ? BIG_JUMP_LEFT : BIG_JUMP_RIGHT;
        }
        break;
    case POWER:
        if (state == WALK || state == WON)
        {
            if (dir == LEFT)
            {
                walk_right = 1;
                if (walk_left == 1)
                {
                    image = WHITE_WALK_LEFT1;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 2;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 2)
                {
                    image = WHITE_WALK_LEFT2;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 3;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 3)
                {
                    image = WHITE_WALK_LEFT3;
                    if (!LW_Timer.isStarted())
                        LW_Timer.start();
                    if (LW_Timer.getTime() > 30)
                    {
                        walk_left = 1;
                        LW_Timer.reset();
                    }
                }
            }
            else if (dir == RIGHT)
            {
                walk_left = 1;
                if (walk_right == 1)
                {
                    image = WHITE_WALK_RIGHT1;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 2;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 2)
                {
                    image = WHITE_WALK_RIGHT2;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 3;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 3)
                {
                    image = WHITE_WALK_RIGHT3;
                    if (!RW_Timer.isStarted())
                    {
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30)
                    {
                        walk_right = 1;
                        RW_Timer.reset();
                    }
                }
            }
        }
        else if (state == STAND)
        {
            image = (dir == LEFT) ? WHITE_STAND_LEFT : WHITE_STAND_RIGHT;
        }

        else if (state == JUMP)
        {
            image = (dir == LEFT) ? WHITE_JUMP_LEFT : WHITE_JUMP_RIGHT;
        }
        else if (state == SLIDE)
        {
            image = (dir == LEFT) ? WHITE_SLIDE_LEFT : WHITE_SLIDE_RIGHT;
        }
        else if (state == FALL)
        {
            image = (dir == LEFT) ? WHITE_JUMP_LEFT : WHITE_JUMP_RIGHT;
        }
        break;
    default:
        break;
    }
}

void Player::startMove()
{
    state = WALK;
    slide_enable = 0;
}

void Player::parade()
{
    _moveX(5);
}

void Player::normalMove()
{
    if (dir == LEFT)
    {
        speed = -5;
        if (speed < -5)
        {
            speed = -5;
        }
    }
    else if (dir == RIGHT)
    {
        speed = 5;
        if (speed > 5)
        {
            speed = 5;
        }
    }
    if (state == SLIDE)
    {
        if (slideTimer.getTime() > 200)
        {
            state = STAND;
            speed = 0;
            slide_enable = 0;
        }
        else
        {
            _moveX(speed);
        }
    }
    else
    {
        _moveX(speed);
        slide_enable += 1;
    }
}

void Player::stand()
{
}

void Player::endMove()
{
    if (slide_enable < 20)
    {
        state = STAND;
        speed = 0;
        return;
    }
    else if (state == WALK)
    {
        slideTimer.reset();
        slideTimer.start();
        state = SLIDE;
        if (dir == LEFT)
        {
            speed = -5;
        }
        else if (dir == RIGHT)
        {
            speed = 5;
        }
    }
}

void Player::startJump()
{
    if (state == JUMP || state == FALL || state == DEAD)
    {
        return;
    } // You cannot initiate a jump during a jump

    // audio callback
    jump_a = true;

    if (state == STAND)
    {
        move_during_jump = false;
    }
    else
    {
        move_during_jump = true;
    }
    state = JUMP;
    jump_speed_vertical = -10;
    jumpTimer.reset();
    jumpTimer.start();
}

void Player::jump()
{
    static int jump_cycles = 0;

    if (jumpTimer.getTime() < 600)
    {
        _moveY(jump_speed_vertical);
        jump_speed_vertical += (jump_cycles % 2 == 0) ? 1 : 0;
        jump_cycles += 1;
        if (jump_speed_vertical > 0)
        {
            jump_speed_vertical = 0;
        }
    }
    else
    {
        state = FALL;
        fall_speed_vertical = -jump_speed_vertical;
    }
    _moveX(speed);
}

void Player::endJump()
{
    state = FALL;
}

void Player::startFall()
{
    if (state != JUMP)
    { // when mario is jumping, gravity has no effect on it
        state = FALL;
        fall_speed_vertical = 3;
    }
}

void Player::falling()
{
    _moveY(fall_speed_vertical);
    _moveX(speed);
    fall_speed_vertical += (fall_cycles % 2 == 0) ? 1 : 0;
    if (fall_speed_vertical >= terminal_speed)
    {
        fall_speed_vertical = terminal_speed;
    }
    fall_cycles += 1;
}

void Player::endFall()
{
    if (state == FALL)
    {
        state = STAND;
        speed = 0;
    }
}

void Player::kill(Object *obj)
{
    // Audio callback
    enemy_stomp_a = true;
    obj->death();
}

void Player::death()
{
    if (immeunityTimer.isStarted() && immeunityTimer.getMilliseconds() < 1500)
    {
        return;
    }
    else
    {
        immeunityTimer.reset();
    }
    if (state == DEAD)
    {
        return;
    }
    if (level == NORMAL)
    {
        // Audio Callback
        death_a = true;
        state = DEAD;
        dead = true;
        speed = 0;
        terminal_speed = 100;
    }
    else
    {
        if (level == BIG)
        {
            level = NORMAL;
            size = Point(22, 30);
            pos.y += 18;
            immeunityTimer.start();
        }
        else if (level == POWER)
        {
            level = BIG;
            immeunityTimer.start();
        }
    }
}

void Player::immediate_death()
{
    // Player state
    level = NORMAL;
    state = DEAD;
    dead = true;

    // Sound effect
    death_a = true;

    // Player position and size
    size = Point(22, 30);
    pos.y += 18;

    // Falling speeds
    speed = 0;
    terminal_speed = 100;
}

void Player::death_animation()
{
    if (!deathAnimation.isStarted())
    {
        deathAnimation.start();
    }
    if (deathAnimation.getTime() < 300)
    {
        fall_speed_vertical = 3;
        _moveY(-5);
    }
    else if (deathAnimation.getTime() >= 350)
    {
        falling();
    }
}

void Player::powerupAnimation()
{
}

// Collision Notification
void Player::notifyCollisionLeft(Object *obj)
{
    if (state == WON)
    {
        return;
    }
    if (state == DEAD)
    {
        return;
    }
    if (dir == LEFT)
    {
        collidedL = true;
        speed = 0;
        if (state == WALK || state == SLIDE)
        {
            state = STAND;
        }
        else if (state == JUMP)
        {
            if (speed < 0)
                speed = 0;
        }
        if (obj->getType() == GOOMBA)
        {
            death();
        }

        if (obj->getType() == FLAG)
        {
            std::cout << "reached" << std::endl;
        }
    }
}
void Player::notifyCollisionRight(Object *obj)
{
    if (state == WON)
    {
        return;
    }
    if (state == DEAD)
    {
        return;
    }
    if (dir == RIGHT)
    {
        collidedR = true;
        speed = 0;
        if (state == WALK || state == SLIDE)
        {
            state = STAND;
        }
        else if (state == JUMP)
        {
            if (speed > 0)
                speed = 0;
        }
        if (obj->getType() == GOOMBA)
        {
            death();
        }
        if (this->getPos().x >= 4730 && this->getState() != WON)
        {
            std::cout << "reached" << std::endl;
            this->state = WON;
            win_a = true;
        }
    }
}
void Player::notifyCollisionTop(Object *obj)
{
    Type t = obj->getType();
    if (state == DEAD)
    {
        return;
    }
    if (state == JUMP)
    {
        std::cout << ToString(t) << std::endl;
        if (t == BRICK)
        {
            if (level == NORMAL)
                obj->mark();
            else
            {
                obj->destroy();
                brick_debris_a = true;
            }
        }
        else if (t == COIN_CONTAINER)
        {
            // Audio callback
            if (((CoinContainer *)obj)->coinIsAvailable)
            {
                obj->mark();
                coin_a = true;
            }
        }
        else if (t == FIRE_CONTAINER)
        {
            if (level == NORMAL)
            {
                obj->mark();
                powerup_appears_a = true;
            }
            else
            {
                obj->flower = true;
                obj->mark();
            }
        }
        else if (t == HEALTH_CONTAINER)
        {
            obj->mark();
        }
        endJump();
    }
}
void Player::notifyCollisionBottom(Object *obj)
{
    if (state == DEAD)
    {
        return;
    }
    fall_speed_vertical = 0;
    endFall();

    // Kill enemies by jumping on their head :)
    if (obj->getType() == GOOMBA)
    {
        kill(obj);
        Text *text = new Text(pos.x, pos.y - 10);
        text->setPos(pos.x, pos.y - 10);
        text->ghost_dead = false;
        text->text = "+ 150";
        text->score = 150;
        ghost.push_back(text);

        has_ghost = true;
    }
    else if (obj->getType() == KOOPA)
    {
        if (!koopa_hit)
        {
            kill(obj);
            koopa_hit = true;
        }
    }
}

void Player::notifyFreeLeft() { collidedL = false; }
void Player::notifyFreeRight() { collidedR = false; }
void Player::notifyFreeTop() {}
void Player::notifyFreeBottom()
{
    if (state == DEAD)
    {
        return;
    }

    koopa_hit = false;
    if (state != FALL)
    {
        startFall();
    }
    if (pos.y > 400)
    {
        immediate_death();
    }
}

void Player::notifyDistToPlatform(int d)
{
    if (state == DEAD)
    {
        return;
    }
    if (fall_speed_vertical > d)
    {
        fall_speed_vertical = d;
    }
}

void Player::notifyDistToCeil(int d)
{
    if (state == DEAD)
    {
        return;
    }
    if (abs(jump_speed_vertical) > d)
    {
        jump_speed_vertical = -d;
    }
}

void Player::powerup()
{
    powerup_a = true;
    if (level == NORMAL)
    {
        level = BIG;
        size = Point(22, 48);
        pos.y -= 18;
    }
    else if (level == BIG)
    {
        level = POWER;
    }
}

void Player::shoot()
{
    // TODO check ammo
}