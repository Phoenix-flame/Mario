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
    void collision();
    void gravity();

    std::vector<Object*> getObjects();
    Camera* camera;
private:
    Map* map;
    
    Physics* physics;

};