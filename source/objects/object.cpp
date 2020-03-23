#include "object.hpp"

Object::Object(Point pos, Point size, std::string image){
    this->pos = pos;

    this->size = size;

    this->pos.x *= this->size.x;
    this->pos.y *= this->size.y;

    this->xMin = this->pos.x;
    this->xMax = this->xMin + this->size.x;

    this->yMin = this->pos.y;
    this->yMax = this->yMin + this->size.y;

    this->image = image;
}


std::string Object::getImage(){
    return image;
}


Point Object::getPos(){
    return pos;
}
Point Object::getSize(){
    return size;
}


Point Object::getXRange(){
    return Point(xMin, xMax);
}
Point Object::getYRange(){
    return Point(yMin, yMax);
}