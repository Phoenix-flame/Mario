#include "rsdl.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <queue>
#include "map.hpp"

#include "camera.hpp"
#include "physics.hpp"

struct GameState{
    double progress = 0.0;
    bool alive = true;
    bool marioLevel = 1;
    int score = 0;
};

class World{
public:
    World();
    void loop();

    

    std::vector<Object*> getObjects();
    Camera* camera;
    Map* map;

    
    Player* getPlayer();
    std::vector<Object*> getGhosts();
    GameState* getGameState();
    
private:
    GameState* gameState;
    void gravity();
    int collisionGravity(Rectangle o1, Rectangle o2);
    
    void ghostCollector();
    std::vector<Object*> ghosts;
    
    void collision(Object* obj);
    
    Physics* physics;

};