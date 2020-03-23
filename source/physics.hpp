#include <vector>
#include "objects/object.hpp"

class Physics{
public:
    void gravity();
    void collision(std::vector<Object*> objects);
    void friction();

private:
};