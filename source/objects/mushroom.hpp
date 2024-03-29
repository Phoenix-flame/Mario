#ifndef _MUSHROOM_HPP_
#define _MUSHROOM_HPP_

#include "object.hpp"
#include "../player.hpp"
#define RED_MUSHROOM_IMAGE "assets/sprites/objects/mushroom/red.png"
#define HEALTH_MUSHROOM_IMAGE "assets/sprites/objects/mushroom/health.png"

enum MushroomState
{
    M_RUN,
    M_FALL
};

class Mushroom : public Object
{
public:
    Mushroom(MushroomType _type, Point _pos) : Object(Point(0, 0),
                                                      Point(20, 20),
                                                      "",
                                                      G_MUSHROOM)
    {
        this->type = _type;
        image = (type == M_RED) ? RED_MUSHROOM_IMAGE : HEALTH_MUSHROOM_IMAGE;
        funcToRun = &Mushroom::riseLikeThePhoenix;

        pos.x = _pos.x;
        pos.y = _pos.y;

        y_target = pos.y - size.y;
        x_gravity_en = pos.x + size.x + 1;

        ghost_dead = false;
    }

    void update();

    // Collision Notification
    void notifyCollisionLeft(Object *) override;
    void notifyCollisionRight(Object *) override;
    void notifyCollisionTop(Object *) override;
    void notifyCollisionBottom(Object *) override;
    void notifyFreeLeft() override;
    void notifyFreeRight() override;
    void notifyFreeTop() override;
    void notifyFreeBottom() override;

    void notifyDistToPlatform(int d) override;
    void notifyDistToCeil(int d) override;

private:
    void (Mushroom::*funcToRun)();

    MushroomState state;

    void riseLikeThePhoenix();
    void run();

    MushroomType type;
    Timer riseTimer;

    int speed = 2;
    int fall_speed_vertical;
    int fall_cycles = 0;
    int terminal_speed = 10;
    Point min_dist_to_platform;

    int y_target;
    int x_gravity_en;
    bool gravity_en = false;

    // MovementProfile
    void startFall();
    void falling();
    void endFall();
};
#endif // !_MUSHROOM_HPP_