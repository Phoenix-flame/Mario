#include "rsdl.hpp"


class Camera{
public:
    Camera();

    void move(int offset);
    Point getPos();

private:
    Point pos;
};