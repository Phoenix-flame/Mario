#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
}


void World::loop(){
    // map->player->stand();
    collision();
}


std::vector<Object*> World::getObjects(){
    return map->objects;
}




void World::collision(){
    Player* player = map->player;
}

void World::gravity(){
    Player* player = map->player;
}