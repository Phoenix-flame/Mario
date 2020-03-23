#include <string>
#include "rsdl.hpp"


class Object{
public:
    Object(Point pos, Point size, std::string image);

private:
    Point pos;

    int xMin;
    int yMin;

    int xMax;
    int yMax;

    Point size;
    

    std::string image;
};