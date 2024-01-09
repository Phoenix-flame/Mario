#ifndef _COIN_HPP_
#define _COIN_HPP_

#include "object.hpp"

#define COIN_IMAGE "assets/sprites/objects/coin.png"

class Coin : public Object
{
public:
    Coin(int x, int y) : Object(Point(x, y),
                                Point(20, 20),
                                COIN_IMAGE,
                                G_COIN) {}

    void update()
    {
        if (!coinAnimation.isStarted())
        {
            coinAnimation.start();
            coinIsAvailable = true;
        }

        if (coinIsAvailable)
        {
            if (flip_cycle % 4 == 0)
            {
                flipped = !flipped;
            }
            flip_cycle += 1;
            if (coinAnimation.getTime() < 200)
            {
                _moveY(-10);
            }
            else if (coinAnimation.getTime() >= 200 && coinAnimation.getTime() < 350)
            {
                _moveY(+10);
            }
            else
            {
                coinIsAvailable = false;
                ghost_dead = true;
            }
        }
    }

    bool flipped = false;
    bool coinIsAvailable = false;

private:
    int flip_cycle = 0;
    Timer coinAnimation;
};
#endif // !_COIN_HPP_