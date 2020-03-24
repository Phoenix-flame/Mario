#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "../rsdl.hpp"
#include "../timer.hpp"

enum Type {
    BLOCK,
    BRICK,
    GROUND,
    FIRE_CONTAINER,
    HEALTH_CONTAINER,
    COIN_CONTAINER,
    PIPE,
    GOOMBA,
    KOOPA,
    PLAYER
};

class Object{
public:
    Object(Point pos, Point size, std::string image, Type _type);
    std::string getImage();
    Point getPos();
    Point getSize();
    Point getXRange();
    Point getYRange();


    // Mark to start animation
    void mark();

    // Debug
    bool selected = false;

    void _moveX(int dx);
    void _moveY(int dy);

protected:
    Point pos;

    int xMin;
    int yMin;

    int xMax;
    int yMax;

    Point size;
    
    bool startAnimation = false;

    std::string image;

    Type type;
};

#endif // !_OBJECT_HPP_