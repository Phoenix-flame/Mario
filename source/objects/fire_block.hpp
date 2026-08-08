#ifndef _FIRE_BLOCK_HPP_
#define _FIRE_BLOCK_HPP_

#include "object.hpp"
#include "mushroom.hpp"
#include "flower.hpp"

#define FIRE_BLOCK_FULL "assets/sprites/objects/bricks_blocks/question-2.png"
#define FIRE_BLOCK_EMPTY "assets/sprites/objects/bricks_blocks/question-empty.png"

enum FireState
{
    FIRE_NORMAL,
    FIRE_RELEASED,
    FIRE_EMPTY
};

class FireContainer : public Object
{
public:
    FireContainer(int x, int y) : Object(Point(x, y),
                                         Point(24, 24),
                                         FIRE_BLOCK_FULL,
                                         FIRE_CONTAINER)
    {
        state = FIRE_NORMAL;
    }

    void update()
    {
        if (startAnimation)
        {
            if (flower)
            {
                if (state == FIRE_NORMAL)
                {
                    if (!animationTimer.isStarted())
                    {
                        animationTimer.start();
                        prevPos = pos;

                        Flower *flower = new Flower(pos);
                        ghost.push_back(flower);

                        has_ghost = true;
                        posDuringAnimation = pos;
                    }
                    if (animationTimer.getTime() < 150)
                    {
                        _moveY(-1);
                    }
                    else if (prevPos.y != pos.y)
                    {
                        image = FIRE_BLOCK_EMPTY;
                        _moveY(+1);
                    }
                    else
                    {
                        animationTimer.reset();
                        state = FIRE_RELEASED;
                        startAnimation = false;
                        flower = false;
                    }
                }
            }
            else
            {
                if (state == FIRE_NORMAL)
                {
                    if (!animationTimer.isStarted())
                    {
                        animationTimer.start();
                        prevPos = pos;

                        Mushroom *mushroom = new Mushroom(M_RED, pos);
                        ghost.push_back(mushroom);

                        has_ghost = true;
                        posDuringAnimation = pos;
                    }
                    if (animationTimer.getTime() < 150)
                    {
                        _moveY(-1);
                    }
                    else if (prevPos.y != pos.y)
                    {
                        image = FIRE_BLOCK_EMPTY;
                        _moveY(+1);
                    }
                    else
                    {
                        animationTimer.reset();
                        state = FIRE_RELEASED;
                        startAnimation = false;
                    }
                }
            }
        }
    }

private:
    FireState state;
    Timer animationTimer;

    Point prevPos;
};

#endif // !_FIRE_BLOCK_