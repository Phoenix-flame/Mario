#include "objects/object.hpp"
#include <vector>
#include "timer.hpp"

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

enum Level{
    NORMAL,
    POWER,
    BIG
};
enum State{
    STAND, 
    WALK,
    DEAD,
    SLIDE,
    JUMP,
    FALL
};



inline const char* ToString(State v)
{
    switch (v)
    {
        case STAND:   return "STAND";
        case WALK:   return "WALK";
        case DEAD: return "DEAD";
        case SLIDE:   return "SLIDE";
        case JUMP:   return "JUMP";
        case FALL: return "FALL";
        default:      return "[Unknown State]";
    }
}

inline const char* ToString(Dir v)
{
    switch (v)
    {
        case STOP:   return "STOP";
        case LEFT:   return "LEFT";
        case RIGHT: return "RIGHT";
        default:      return "[Unknown Direction]";
    }
}

class Player: public Object{
public:
    Player(int x, int y):Object(Point(x, y), Point(12, 16), "", PLAYER){
        level = NORMAL;
        state = STAND;
        image = NORM_STAND_RIGHT;
        size = Point(24, 32);
    }

    void update(std::vector<Object*> objs, int dir);
    void updateFigure();

    // Move and slide
    void startMove();
    void move();
    void endMove();
    
    // Jump Methods
    void startJump();
    void jump();
    void endJump();
    bool can_jump = true; // Jumping only once when up arrow is holded

    // Falling Methods
    void startFall();
    void falling(Dir _dir, bool stop_horizontal_move);
    void move_during_fall();
    void endFall();

    // Base movment methods
    void _moveX(int dx);
    void _moveY(int dy);

    State getState(){return state;}
    Dir getDir(){return dir;}

    void gravity(std::vector<Object*> objs);
    void jumpCollision(std::vector<Object*> objs);

    void kill(Object* obj);

    void dead();


    // Debug 
    Point min_dist_to_platform;
private:
    Level level;
    State state;
    Dir dir;

    Point checkDistToPlatform(std::vector<Object*> objs);
    int collisionGravity(Rectangle o1, Rectangle o2);
    void collision(std::vector<Object*> objs);

    int slide_enable = 0;
    
    int speed = 0;

    bool move_during_jump;
    int jump_speed_vertical;
    int jump_speed_horizontal;

    int fall_speed_vertical;
    int fall_speed_horizontal;

    // Timers
    Timer slideTimer;
    Timer jumpTimer;
    Timer RW_Timer;
    Timer LW_Timer;
};