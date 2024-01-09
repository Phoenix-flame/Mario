#include "object.hpp"
#include "mushroom.hpp"

#define HEALTH_BLOCK_FULL "assets/sprites/objects/bricks_blocks/question-3.png"
#define HEALTH_BLOCK_EMPTY "assets/sprites/objects/bricks_blocks/question-empty.png"

enum HealthState
{
    HEALTH_NORMAL,
    HEALTH_RELEASED,
    HEALTH_EMPTY
};

class HealthContainer : public Object
{
public:
    HealthContainer(int x, int y) : Object(Point(x, y),
                                           Point(24, 24),
                                           HEALTH_BLOCK_FULL,
                                           HEALTH_CONTAINER)
    {
        state = HEALTH_NORMAL;
    }

    void update()
    {
        if (startAnimation)
        {
            if (state == HEALTH_NORMAL)
            {
                if (!animationTimer.isStarted())
                {
                    animationTimer.start();
                    prevPos = pos;

                    Mushroom *mushroom = new Mushroom(M_HEALTH, pos);
                    ghost.push_back(mushroom);

                    has_ghost = true;
                }
                if (animationTimer.getTime() < 150)
                {
                    _moveY(-1);
                }
                else if (prevPos.y != pos.y)
                {
                    image = HEALTH_BLOCK_EMPTY;
                    _moveY(+1);
                }
                else
                {
                    animationTimer.reset();
                    state = HEALTH_RELEASED;
                }
            }
        }
    }

private:
    HealthState state;
    Timer animationTimer;
    Point prevPos;
};