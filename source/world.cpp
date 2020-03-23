#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
}


void World::loop(){

}


std::vector<Object*> World::getObjects(){
    return map->objects;
}