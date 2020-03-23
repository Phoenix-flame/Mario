#include "object.hpp"


class Ground: public Object{
public:
    Ground(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/clay.png"){}
};