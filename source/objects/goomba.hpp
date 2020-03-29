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
    Goomba(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        GOOMBA_WALK1_IMAGE, 
        GOOMBA){

        state = GOOMBA_WALK_STATE;
        int speed = -1;
        dir = LEFT;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<int> distribution(1,2);
        walk_state = distribution(generator);
    }

    void update(std::vector<Object*> objs);
    void gravity(std::vector<Object*> objs);
    void collision(std::vector<Object*> objs);
    void updateFigure();

    void startFall();
    void falling();
    void endFall();

    void seen(){
        visited = true;
    }


    void death() override;
    Dir dir;
private:
    GoombaState state;
    

    int checkDistToPlatform(std::vector<Object*> objs);
    int collisionGravity(Rectangle o1, Rectangle o2);

    bool visited = false; // goombas start moving when they are seen once.

    int speed;
    int fall_speed_vertical;
    int fall_speed_horizontal;
    int fall_cycles = 0;
    
    int walk_state;
    // Timers
    Timer walkTimer;
    Timer deathAnimation;

};