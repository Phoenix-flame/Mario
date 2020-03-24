#include "object.hpp"

#define HEALTH_BLOCK_FULL "assets/sprites/objects/bricks_blocks/question-3.png"
#define HEALTH_BLOCK_EMPTY "assets/sprites/objects/bricks_blocks/question-empty.png"

class HealthContainer: public Object{
public:
    HealthContainer(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        HEALTH_BLOCK_FULL, 
        HEALTH_CONTAINER){}

private:
};