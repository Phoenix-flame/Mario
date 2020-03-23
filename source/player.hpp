#include "objects/object.hpp"

#define NORM_DEAD "assets/sprites/mario/normal/dead.png"
#define NORM_JUMP_LEFT "assets/sprites/mario/normal/jumping-left.png"
#define NORM_JUMP_RIGHT "assets/sprites/mario/normal/jumping-right.png"
#define NORM_SLIDE_LEFT "assets/sprites/mario/normal/sliding-left.png"
#define NORM_SLIDE_RIGHT "assets/sprites/mario/normal/sliding-right.png"
#define NORM_STAND_LEFT "assets/sprites/mario/normal/standing-left.png"
#define NORM_STAND_RIGHT "assets/sprites/mario/normal/standing-right.png"
#define NORM_WALK_LEFT1 "assets/sprites/mario/normal/walking-left-1.png"
#define NORM_WALK_LEFT2 "assets/sprites/mario/normal/walking-left-2.png"
#define NORM_WALK_LEFT3 "assets/sprites/mario/normal/walking-left-3.png"
#define NORM_WALK_RIGHT1 "assets/sprites/mario/normal/walking-right-1.png"
#define NORM_WALK_RIGHT2 "assets/sprites/mario/normal/walking-right-2.png"
#define NORM_WALK_RIGHT3 "assets/sprites/mario/normal/walking-right-3.png"

#define WHITE_JUMP_LEFT "assets/sprites/mario/white/jumping-left.png"
#define WHITE_JUMP_RIGHT "assets/sprites/mario/white/jumping-right.png"
#define WHITE_SLIDE_LEFT "assets/sprites/mario/white/sliding-left.png"
#define WHITE_SLIDE_RIGHT "assets/sprites/mario/white/sliding-right.png"
#define WHITE_STAND_LEFT "assets/sprites/mario/white/standing-left.png"
#define WHITE_STAND_RIGHT "assets/sprites/mario/white/standing-right.png"
#define WHITE_WALK_LEFT1 "assets/sprites/mario/white/walking-left-1.png"
#define WHITE_WALK_LEFT2 "assets/sprites/mario/white/walking-left-2.png"
#define WHITE_WALK_LEFT3 "assets/sprites/mario/white/walking-left-3.png"
#define WHITE_WALK_RIGHT1 "assets/sprites/mario/white/walking-right-1.png"
#define WHITE_WALK_RIGHT2 "assets/sprites/mario/white/walking-right-2.png"
#define WHITE_WALK_RIGHT3 "assets/sprites/mario/white/walking-right-3.png"

#define BIG_JUMP_LEFT "assets/sprites/mario/big/jumping-left.png"
#define BIG_JUMP_RIGHT "assets/sprites/mario/big/jumping-right.png"
#define BIG_SLIDE_LEFT "assets/sprites/mario/big/sliding-left.png"
#define BIG_SLIDE_RIGHT "assets/sprites/mario/big/sliding-right.png"
#define BIG_STAND_LEFT "assets/sprites/mario/big/standing-left.png"
#define BIG_STAND_RIGHT "assets/sprites/mario/big/standing-right.png"
#define BIG_WALK_LEFT1 "assets/sprites/mario/big/walking-left-1.png"
#define BIG_WALK_LEFT2 "assets/sprites/mario/big/walking-left-2.png"
#define BIG_WALK_LEFT3 "assets/sprites/mario/big/walking-left-3.png"
#define BIG_WALK_RIGHT1 "assets/sprites/mario/big/walking-right-1.png"
#define BIG_WALK_RIGHT2 "assets/sprites/mario/big/walking-right-2.png"
#define BIG_WALK_RIGHT3 "assets/sprites/mario/big/walking-right-3.png"

enum State{
    NORMAL,
    POWER,
    BIG
};

class Player: public Object{
public:
    Player(int x, int y):Object(Point(x, y), Point(24, 32), ""){
        std::cout << x << " " << y << std::endl;
        state = NORMAL;
        image = NORM_STAND_RIGHT;
        size = Point(24, 32);
    }


private:
    State state;

};