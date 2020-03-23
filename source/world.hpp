#include "rsdl.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include "map.hpp"
#include "camera.hpp"
#include "physics.hpp"

class World{
public:
    World();
    void loop();

    // Physics
    bool collision(Object o1, Object o2);
    bool collisionGravity(Object o1, Object o2);
    void gravity();

    Player* getPlayer(){return map->player;}

    std::vector<Object*> getObjects();
    Camera* camera;
private:
    Map* map;
    
    Physics* physics;

};