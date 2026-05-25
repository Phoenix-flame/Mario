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

struct CollisionPart{
    Object *object;
    Rectangle rect;

    CollisionPart(Object *_object, Rectangle _rect) : object(_object), rect(_rect) {}
};

struct CollisionBody{
    Type type;
    Rectangle bounds;
    std::vector<CollisionPart> parts;

    CollisionBody(Type _type, Rectangle _bounds) : type(_type), bounds(_bounds) {}
};

class World{
public:
    World();
    void loop();



    std::vector<Object*> getObjects();
    std::vector<CollisionBody> getCollisionBodies();
    Camera* camera;
    Map* map;


    Player* getPlayer();
    std::vector<Object*> getGhosts();
    GameState* getGameState();
    void addGhost(Object *obj);
private:
    GameState* gameState;
    void gravity();
    int collisionGravity(Rectangle o1, Rectangle o2);

    void ghostCollector();
    std::vector<Object*> ghosts;

    std::vector<CollisionBody> collisionBodies;
    void rebuildCollisionBodies();

    void collision(Object* obj);
    void hitEnemiesAbove(Object *platform);
    void preciseCollisionDetector(Rectangle& o1, Rectangle& o2);

    Physics* physics;

};