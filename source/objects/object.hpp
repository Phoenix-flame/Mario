#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "../rsdl.hpp"
#include "../timer.hpp"
#include <vector>


enum MushroomType{
    M_RED,
    M_HEALTH
};


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
    PLAYER,
    G_COIN,
    G_TEXT,
    G_MUSHROOM
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

    virtual void update_new(){}

    // Mark to start animation
    void mark();

    // Debug
    bool selected = false;

    void _moveX(int dx);
    void _moveY(int dy);

    Type getType(){
        return type;
    }

    void setPos(int x, int y){
        pos.x = x;
        pos.y = y;
    }

    Point getOffset(){
        return offsetAnimation;
    }

    // Collision Notification
    virtual void notifyCollisionLeft(Object*);
    virtual void notifyCollisionRight(Object*);
    virtual void notifyCollisionTop(Object*);
    virtual void notifyCollisionBottom(Object*);
    virtual void notifyFreeLeft();
    virtual void notifyFreeRight();
    virtual void notifyFreeTop();
    virtual void notifyFreeBottom();
    virtual void notifyDistToPlatform(int d);
    virtual void notifyDistToCeil(int d);

    virtual void death();
    virtual void kill();

    std::vector<Object*> ghost;
    bool has_ghost = false;
    bool ghost_dead = false;

protected:
    Point pos;
    Point offsetAnimation;
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