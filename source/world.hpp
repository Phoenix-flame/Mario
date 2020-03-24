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

    Player* getPlayer(){return map->player;}

    std::vector<Object*> getObjects();
    Camera* camera;
    Map* map;
private:
    
    
    Physics* physics;

};