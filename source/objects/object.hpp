#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "../rsdl.hpp"
#include "../timer.hpp"
#include <vector>




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

enum Dir{
    STOP,
    LEFT,
    RIGHT
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

    Type getType(){
        return type;
    }

    virtual void death();
    virtual void kill();
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

    double dist(Point& p1, Point& p2){
        return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
    }
};

#endif // !_OBJECT_HPP_