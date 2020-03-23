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

    std::vector<Object*> getObjects();
    Camera* camera;
private:
    Map* map;
    
    Physics* physics;

};