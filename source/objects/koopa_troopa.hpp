#include "object.hpp"
#include <vector>

#define KOOPA_WALK_RIGHT1 "assets/sprites/enemies/koopa_troopa/walking-right-1.png"
#define KOOPA_WALK_RIGHT2 "assets/sprites/enemies/koopa_troopa/walking-right-2.png"
#define KOOPA_WALK_LEFT1 "assets/sprites/enemies/koopa_troopa/walking-left-1.png"
#define KOOPA_WALK_LEFT2 "assets/sprites/enemies/koopa_troopa/walking-left-2.png"
#define KOOPA_DEAD "assets/sprites/enemies/koopa_troopa/dead.png"


enum KoopaState{
    KOOPA_WALK_STATE,
    KOOPA_FALL_STATE,
    KOOPA_ATTACK_STATE,
    KOOPA_FAST_AND_FURIOUS_STATE
};

class Koopa: public Object{
public:
    Koopa(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        KOOPA_WALK_LEFT1,
        KOOPA){

            state = KOOPA_WALK_STATE;
            dir = LEFT;
            int speed = -1;
    }  



    void update();
    void updateFigure();

    void startFall();
    void falling();
    void endFall();

    void seen(){
        visited = true;
    }

    // Collision Notification
    void notifyCollisionLeft(Object*) override;
    void notifyCollisionRight(Object*) override;
    void notifyCollisionTop(Object*) override;
    void notifyCollisionBottom(Object*) override;
    void notifyFreeLeft() override;
    void notifyFreeRight() override;
    void notifyFreeTop() override;
    void notifyFreeBottom() override;

    void notifyDistToPlatform(int d) override;
    void notifyDistToCeil(int d) override;


    void death() override;

    bool hitted = false;
private:
    KoopaState state;
    Dir dir;

    bool visited = false; // koopa starts moving when it's seen once.
    bool dead = false;
    bool attack = false;

    int speed;
    int fall_speed_vertical;
    int fall_speed_horizontal;
    int fall_cycles = 0;
    
    int right_walk_state = 1;
    int left_walk_state = 1;

    // Timers
    Timer RW_Timer;
    Timer LW_Timer;
    Timer deathAnimation;
};