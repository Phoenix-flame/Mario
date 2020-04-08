#include "object.hpp"
#include <vector>
#include <random>
#include <chrono>

#define GOOMBA_WALK1_IMAGE "assets/sprites/enemies/little_goomba/walking-1.png"
#define GOOMBA_WALK2_IMAGE "assets/sprites/enemies/little_goomba/walking-2.png"
#define GOOMBA_DEAD_IMAGE "assets/sprites/enemies/little_goomba/dead.png"

enum GoombaState{
    GOOMBA_WALK_STATE,
    GOOMBA_DEAD_STATE,
    GOOMBA_FALL_STATE
};

class Goomba: public Object{
public:
    Goomba(int x, int y);

    void update();
    void updateFigure();

    void startFall();
    void falling();
    void endFall();

    void normal_behavior();
    void death_animation();

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

    Dir dir;
private:
    void (Goomba::*funcToRun)();
    GoombaState state;
    

    bool visited = false; // goombas start moving when they are seen once.

    int speed;
    int fall_speed_vertical;
    int fall_speed_horizontal;
    int fall_cycles = 0;

    int min_dist_to_platform;
    
    int walk_state;
    // Timers
    Timer walkTimer;
    Timer deathAnimation;

};