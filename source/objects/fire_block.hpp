#include "object.hpp"


class FireContainer: public Object{
public:
    FireContainer(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/question-2.png"){}

private:
    std::string image_empty = "assets/sprites/objects/bricks_blocks/question-empty.png";
};