#include "object.hpp"


class CoinContainer: public Object{
public:
    CoinContainer(int x, int y):Object(Point(x, y), Point(24, 24), "assets/sprites/objects/bricks_blocks/question-1.png"){}


private:
    std::string image_empty = "assets/sprites/objects/bricks_blocks/question-empty.png";
};