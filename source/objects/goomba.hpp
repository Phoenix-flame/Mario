#include "object.hpp"


class Goomba: public Object{
public:
    Goomba(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/enemies/little_goomba/walking-1.png"){}

private:
    std::string image_walk1 = "assets/sprites/enemies/little_goomba/walking-1.png";
    std::string image_walk2 = "assets/sprites/enemies/little_goomba/walking-2.png";
    std::string image_dead = "assets/sprites/enemies/little_goomba/dead.png";
};