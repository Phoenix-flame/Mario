#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "../rsdl.hpp"


class Object{
public:
    Object(Point pos, Point size, std::string image);
    std::string getImage();
    Point getPos();
    Point getSize();
    Point getXRange();
    Point getYRange();
protected:
    Point pos;

    int xMin;
    int yMin;

    int xMax;
    int yMax;

    Point size;
    

    std::string image;
};

#endif // !_OBJECT_HPP_