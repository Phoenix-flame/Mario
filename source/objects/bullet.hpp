#ifndef _BULLET_HPP_
#define _BULLET_HPP_

#include "object.hpp"

#define BULLET_IMAGE "assets/sprites/objects/coin.png"

class Bullet : public Object
{
public:
    Bullet(int x, int y, Dir dir);

    void update();

    void notifyCollisionLeft(Object *) override;
    void notifyCollisionRight(Object *) override;
    void notifyCollisionTop(Object *) override;
    void notifyCollisionBottom(Object *) override;
    void notifyFreeBottom() override;
    void notifyDistToPlatform(int d) override;

private:
    void hit(Object *obj);

    Dir dir;
    int speed;
    int vertical_speed;
    int fall_cycles = 0;
    int terminal_speed = 8;
};

#endif // !_BULLET_HPP_
