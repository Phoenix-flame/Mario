#include "rsdl.hpp"


class Camera{
public:
    Camera();

    void move(int offset);
    void getPos();

private:
    Point pos;
};