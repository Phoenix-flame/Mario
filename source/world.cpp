#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
}


void World::loop(){
    for(auto b:map->bricks){
        b->update();
    }
    for(auto c:map->coins){
        c->update();
    }
    for(auto g:map->goombas){
        if ((g->getPos() + camera->getPos()).x < 700){
            g->seen();
        } 
        g->update(map->objects);
    }
}


std::vector<Object*> World::getObjects(){
    return map->objects;
}




