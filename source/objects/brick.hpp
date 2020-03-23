#include "object.hpp"


class Brick: public Object{
public:
    Brick(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/brick.png"){}

private:
    
};