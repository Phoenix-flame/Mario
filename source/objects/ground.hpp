#include "object.hpp"

#define GROUND_BLOCK_IMAGE "assets/sprites/objects/bricks_blocks/clay.png"

class Ground : public Object
{
public:
    Ground(int x, int y) : Object(Point(x, y),
                                  Point(24, 24),
                                  GROUND_BLOCK_IMAGE,
                                  GROUND) {}
};