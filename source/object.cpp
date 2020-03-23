#include "object.hpp"

Object::Object(Point pos, Point size, std::string image){
    this->pos = pos;

    this->size = size;

    this->image = image;
}