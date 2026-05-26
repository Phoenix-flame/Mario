#ifndef _PHYSICS_HPP_
#define _PHYSICS_HPP_

#include <functional>
#include <vector>
#include "objects/object.hpp"

struct CollisionBody;

class Physics
{
public:
    void collision(Object *obj,
                   const std::vector<CollisionBody> &collisionBodies,
                   const std::vector<Object *> &dynamicObjects,
                   const std::function<void(Object *)> &hitEnemiesAbove);

private:
};

#endif // !_PHYSICS_HPP_
