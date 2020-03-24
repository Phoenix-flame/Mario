#include "object.hpp"

#define KOOPA_WALK_RIGHT1 "assets/sprites/enemies/koopa_troopa/walking-right-1.png"
#define KOOPA_WALK_RIGHT2 "assets/sprites/enemies/koopa_troopa/walking-right-2.png"
#define KOOPA_WALK_LEFT1 "assets/sprites/enemies/koopa_troopa/walking-left-1.png"
#define KOOPA_WALK_LEFT2 "assets/sprites/enemies/koopa_troopa/walking-left-2.png"
#define KOOPA_DEAD "assets/sprites/enemies/koopa_troopa/dead.png"


class Koopa: public Object{
public:
    Koopa(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        KOOPA_WALK_LEFT1,
        KOOPA){}

private:

};