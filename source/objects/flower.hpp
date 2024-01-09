#ifndef _FLOWER_HPP_
#define _FLOWER_HPP_

#include "object.hpp"
#include "../player.hpp"
#define FLOWER_IMAGE "assets/sprites/objects/flower.png"

class Flower : public Object
{
public:
    Flower(Point _pos) : Object(Point(0, 0),
                                Point(20, 20),
                                "",
                                G_FLOWER)
    {
        image = FLOWER_IMAGE;

        pos.x = _pos.x;
        pos.y = _pos.y;

        y_target = pos.y - size.y;

        ghost_dead = false;
    }

    void update()
    {
        if (pos.y >= y_target)
        {
            _moveY(-1);
        }
    }

    // Collision Notification
    void notifyCollisionLeft(Object *) override;
    void notifyCollisionRight(Object *) override;
    void notifyCollisionTop(Object *) override;
    void notifyCollisionBottom(Object *) override;
    void notifyFreeLeft() override;
    void notifyFreeRight() override;
    void notifyFreeTop() override;
    void notifyFreeBottom() override;

    void notifyDistToPlatform(int d) override;
    void notifyDistToCeil(int d) override;

private:
    void (Flower::*funcToRun)();

    Timer riseTimer;
    int y_target;
};
#endif // !_FLOWER_HPP_