#include "flower.hpp"

// Collision Notification
void Flower::notifyCollisionLeft(Object *obj)
{
    Type t = obj->getType();
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
}
void Flower::notifyCollisionRight(Object *obj)
{
    Type t = obj->getType();
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
}
void Flower::notifyCollisionTop(Object *obj)
{
    Type t = obj->getType();
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
}
void Flower::notifyCollisionBottom(Object *obj)
{
    Type t = obj->getType();
    if (t == PLAYER)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
        return;
    }
}

void Flower::notifyFreeLeft() {}
void Flower::notifyFreeRight() {}
void Flower::notifyFreeTop() {}
void Flower::notifyFreeBottom()
{
}

void Flower::notifyDistToPlatform(int d)
{
}

void Flower::notifyDistToCeil(int d)
{
}