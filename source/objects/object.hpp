#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "../rsdl.hpp"
#include "../timer.hpp"


class Object{
public:
    Object(Point pos, Point size, std::string image);
    std::string getImage();
    Point getPos();
    Point getSize();
    Point getXRange();
    Point getYRange();

    void mark(){
        startAnimation = true;
    }

    bool selected = false;

    void _moveX(int dx){
        pos.x += dx;
        xMin += dx;
        xMax += dx;
    }
    void _moveY(int dy){
        pos.y += dy;
        yMin += dy;
        yMax += dy;
    }

protected:
    Point pos;

    int xMin;
    int yMin;

    int xMax;
    int yMax;

    Point size;
    
    bool startAnimation = false;

    std::string image;
};

#endif // !_OBJECT_HPP_