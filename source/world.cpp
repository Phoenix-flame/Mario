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
    for(unsigned int i = 0 ; i < map->goombas.size() ; i ++){
        if ((map->goombas[i]->getPos() + camera->getPos()).x < 700){
            map->goombas[i]->seen();
        } 

        if (map->goombas[i]->getPos().y > 500){
            map->goombas.erase(map->goombas.begin() + i);
        }
        
        map->goombas[i]->update(map->objects);
    }

    for(unsigned int i = 0 ; i < map->koopas.size() ; i ++){
        if ((map->koopas[i]->getPos() + camera->getPos()).x < 700){
            map->koopas[i]->seen();
        } 

        if (map->koopas[i]->getPos().y > 500){
            map->koopas.erase(map->koopas.begin() + i);
        }
        
        map->koopas[i]->update(map->objects);
    }
}


std::vector<Object*> World::getObjects(){
    return map->objects;
}




