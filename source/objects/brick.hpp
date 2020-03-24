#include "object.hpp"

#define BRICK_IMAGE "assets/sprites/objects/bricks_blocks/brick.png"

class Brick: public Object{
public:
    Brick(int x, int y):Object(Point(x, y),
        Point(24, 24),
        BRICK_IMAGE,
        BRICK){}

    void update(){
        if (startAnimation){
            if (!animationTimer.isStarted()){
                animationTimer.reset();
                animationTimer.start();
                prev_pos = pos;
            }
            if (animationTimer.getTime() < 150){
                _moveY(-1);
            }
            else if (pos.y != prev_pos.y){
                _moveY(+1);
            }
            else {
                startAnimation = false;
                animationTimer.reset();
            }
        }
    }



private:
    Timer animationTimer;
    Point prev_pos;
};