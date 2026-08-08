#include "flower.hpp"

// Physics can report the same contact twice in a frame (once while the player
// moves, once while the flower is checked), so the pickup is applied only once.
bool Flower::consumeBy(Object *obj)
{
    if (obj->getType() != PLAYER)
    {
        return false;
    }
    if (!ghost_dead)
    {
        ((Player *)obj)->powerup();
        ghost_dead = true;
    }
    return true;
}

// Collision Notification
void Flower::notifyCollisionLeft(Object *obj)
{
    if (consumeBy(obj))
    {
        return;
    }
}
void Flower::notifyCollisionRight(Object *obj)
{
    if (consumeBy(obj))
    {
        return;
    }
}
void Flower::notifyCollisionTop(Object *obj)
{
    if (consumeBy(obj))
    {
        return;
    }
}
void Flower::notifyCollisionBottom(Object *obj)
{
    if (consumeBy(obj))
    {
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