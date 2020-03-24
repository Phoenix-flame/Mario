#include "object.hpp"

#define BLOCK_IMAGE "assets/sprites/objects/bricks_blocks/block.png"

class Block: public Object{
public:
    Block(int x, int y):Object(Point(x, y),
        Point(24, 24),
        BLOCK_IMAGE,
        BLOCK){}
};