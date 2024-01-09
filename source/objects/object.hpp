#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include <vector>
#include "../rsdl.hpp"
#include "../timer.hpp"

enum MushroomType
{
    M_RED,
    M_HEALTH
};

enum Type
{
    BLOCK,
    BRICK,
    GROUND,
    FIRE_CONTAINER,
    HEALTH_CONTAINER,
    COIN_CONTAINER,
    PIPE,
    FLAG,
    CLOUD,
    GOOMBA,
    KOOPA,
    PLAYER,
    G_COIN,
    G_TEXT,
    G_MUSHROOM,
    G_FLOWER,
    G_BULLET
};

inline const char *ToString(Type v)
{
    switch (v)
    {
    case BLOCK:
        return "BLOCK";
    case BRICK:
        return "BRICK";
    case GROUND:
        return "GROUND";
    case FIRE_CONTAINER:
        return "FIRE_CONTAINER";
    case HEALTH_CONTAINER:
        return "HEALTH_CONTAINER";
    case COIN_CONTAINER:
        return "COIN_CONTAINER";
    case PIPE:
        return "PIPE";
    case GOOMBA:
        return "GOOMBA";
    case KOOPA:
        return "KOOPA";
    case PLAYER:
        return "PLAYER";
    default:
        return "[Unknown State]";
    }
}

enum Dir
{
    STOP,
    LEFT,
    RIGHT
};

class Object
{
public:
    Object(Point pos, Point size, std::string image, Type _type);
    std::string getImage();
    Point getPos();
    Point getSize();
    Point getXRange();
    Point getYRange();

    virtual void update_new() {}

    // Mark to start animation
    void mark();
    bool flower = false;

    // Debug
    bool selected = false;

    void _moveX(int dx);
    void _moveY(int dy);

    Type getType()
    {
        return type;
    }

    void setPos(int x, int y)
    {
        pos.x = x;
        pos.y = y;
    }

    void setPos(Point p)
    {
        pos.x = p.x;
        pos.y = p.y;
    }

    // Collision Notification
    virtual void notifyCollisionLeft(Object *);
    virtual void notifyCollisionRight(Object *);
    virtual void notifyCollisionTop(Object *);
    virtual void notifyCollisionBottom(Object *);
    virtual void notifyFreeLeft();
    virtual void notifyFreeRight();
    virtual void notifyFreeTop();
    virtual void notifyFreeBottom();
    virtual void notifyDistToPlatform(int d);
    virtual void notifyDistToCeil(int d);

    virtual void death();
    virtual void kill();
    virtual void destroy();

    bool dead = false;

    std::vector<Object *> ghost;
    bool has_ghost = false;
    bool ghost_dead = false;

    Point posDuringAnimation;
    bool animationIsStarted()
    {
        return startAnimation;
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

    Type type;

    double dist(Point &p1, Point &p2)
    {
        return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
    }
};

#endif // !_OBJECT_HPP_