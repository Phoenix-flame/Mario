#ifndef _BULLET_HPP_
#define _BULLET_HPP_


#include "object.hpp"
#include "../player.hpp"



class Bullet: public Object{
public:
    Bullet(Point _pos):Object(Point(0, 0),
        Point(20, 20),
        "",
        G_MUSHROOM){
            funcToRun = &Bullet::riseLikeThePhoenix;

            pos.x = _pos.x;
            pos.y = _pos.y;

            y_target = pos.y - size.y;
            x_gravity_en = pos.x + size.x + 1;
            
            ghost_dead = false;
        }

    void update();

    
    // Collision Notification
    void notifyCollisionLeft(Object*) override;
    void notifyCollisionRight(Object*) override;
    void notifyCollisionTop(Object*) override;
    void notifyCollisionBottom(Object*) override;
    void notifyFreeLeft() override;
    void notifyFreeRight() override;
    void notifyFreeTop() override;
    void notifyFreeBottom() override;

    void notifyDistToPlatform(int d) override;
    void notifyDistToCeil(int d) override;

    
private:
    void (Bullet::*funcToRun)();

    void riseLikeThePhoenix();
    void run();

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

#endif // !_BULLET_HPP