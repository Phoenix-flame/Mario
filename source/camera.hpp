#include "rsdl.hpp"


class Camera{
public:
    Camera();

    void move(int offset);
    void moveBackground(int offset);
    Point getPos();
    Point getPosBackground(){
        return posBackground;
    }

private:
    Point pos;
    Point posBackground;
    bool canMove = true;
};