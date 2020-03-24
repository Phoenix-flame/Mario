#include "object.hpp"

#define FIRE_BLOCK_FULL "assets/sprites/objects/bricks_blocks/question-2.png"
#define FIRE_BLOCK_EMPTY "assets/sprites/objects/bricks_blocks/question-empty.png"

class FireContainer: public Object{
public:
    FireContainer(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        FIRE_BLOCK_FULL,
        FIRE_CONTAINER){}

private:

};