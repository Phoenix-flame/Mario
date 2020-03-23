#include "object.hpp"


class HealthContainer: public Object{
public:
    HealthContainer(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/question-3.png"){}

private:
    std::string image_empty = "assets/sprites/objects/bricks_blocks/question-empty.png";
};