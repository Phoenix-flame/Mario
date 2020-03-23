#include "object.hpp"


class Block: public Object{
public:
    Block(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/block.png"){}
};