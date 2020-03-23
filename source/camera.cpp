#include "camera.hpp"


Camera::Camera(){
    this->pos = Point(0, 0);
}

void Camera::move(int offset){
    this->pos.x += offset;
}


Point Camera::getPos(){
    return pos;
}
