#include "object.hpp"

#define GOOMBA_WALK1 "assets/sprites/enemies/little_goomba/walking-1.png"
#define GOOMBA_WALK2 "assets/sprites/enemies/little_goomba/walking-2.png"
#define GOOMBA_DEAD "assets/sprites/enemies/little_goomba/dead.png"

class Goomba: public Object{
public:
    Goomba(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        GOOMBA_WALK1, 
        GOOMBA){}

private:

};