#include "object.hpp"


class Koopa: public Object{
public:
    Koopa(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/enemies/koopa_troopa/walking-right-1.png"){}

private:
    std::string image_walk_r1 = "assets/sprites/enemies/koopa_troopa/walking-right-1.png";
    std::string image_walk_r2 = "assets/sprites/enemies/koopa_troopa/walking-right-2.png";
    std::string image_walk_l1 = "assets/sprites/enemies/koopa_troopa/walking-left-1.png";
    std::string image_walk_l2 = "assets/sprites/enemies/koopa_troopa/walking-left-2.png";
    std::string image_dead = "assets/sprites/enemies/koopa_troopa/dead.png";
};