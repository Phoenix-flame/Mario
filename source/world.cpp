#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
}


void World::loop(){
    ghostCollector();

    for(auto b:map->bricks){
        b->update();
    }
    for(auto c:map->coins){
        c->update();
    }
    for(auto f:map->fires){
        f->update();
    }
    for(unsigned int i = 0 ; i < map->goombas.size() ; i ++){
        if ((map->goombas[i]->getPos() + camera->getPos()).x < 700){
            map->goombas[i]->seen();
        } 

        if (map->goombas[i]->getPos().y > 500){
            map->goombas.erase(map->goombas.begin() + i);
        }
        
        map->goombas[i]->update(map->objects, map->player);
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
    for(unsigned int i = 0 ; i < ghosts.size(); i++){
        if (ghosts[i]->ghost_dead){
            ghosts.erase(ghosts.begin() + i);
            continue;
        }
        if (ghosts[i]->getType() == G_COIN){
            ((Coin*)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() == G_TEXT){
            ((Text*)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() ==G_MUSHROOM){
            ((Mushroom*)ghosts[i])->update(map->objects);
        }
        
    }
}

void World::ghostCollector(){
    for (auto obj:map->objects){
        if (obj->has_ghost){
            std::cout << "wohaha\n";
            for(auto g:obj->ghost){
                ghosts.push_back(g);
            }
            obj->has_ghost = false;
            obj->ghost.clear();
        }
    }
}


std::vector<Object*> World::getObjects(){
    return map->objects;
}




