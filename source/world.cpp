#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
}


void World::loop(){
    ghostCollector();
    collision(map->player);
    
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

void World::gravity(){
    
}


int World::collisionGravity(Rectangle o1, Rectangle o2){
    int o1_top = o1.y;
    int o1_bottom = o1.y + o1.h;
    int o2_top = o2.y;
    int o2_bottom = o2.y + o2.h;
    
    int o1_left = o1.x;
    int o1_right = o1.x + o1.w;
    int o2_left = o2.x;
    int o2_right = o2.x + o2.w;

    if (o1_right >= o2_left && o1_left <= o2_right){
        if (o1_bottom <= o2_top && abs(o1_bottom - o2_top) < 3){
            return abs(o1_bottom - o2_top);
        }
    }

    return -1;
}


Player* World::getPlayer(){
    return map->player;
}

std::vector<Object*> World::getGhosts(){
    return ghosts;
}



void World::collision(Object* obj){
    bool on_the_floor = false;
    obj->notifyFreeLeft();
    obj->notifyFreeRight();
    for (auto o:getObjects()){
        Rectangle o1(obj->getPos(), obj->getPos() + obj->getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        // Left and right collision
        if ((o1.right_center.y >= o2.y && o1.right_center.y <= (o2.y + o2.h)) ||
            (o1.y >= o2.y && o1.y <= (o2.y + o2.h)) ||
            ((o1.y + o1.h - 3) >= o2.y && (o1.y + o1.h - 3) <= (o2.y + o2.h))){
            if (abs(o1.right_center.x - o2.left_center.x) < 4){
                // TODO notify either objects
                obj->notifyCollisionRight(o);
            }
            else if (abs(o1.left_center.x - o2.right_center.x) < 4){
                // TODO notify either objects
                obj->notifyCollisionLeft(o);
            }
        } 

        // Bottom collision
        if (collisionGravity(o1, o2) != -1){
            on_the_floor = true;
            obj->notifyCollisionBottom(o);
        }
        else{
            // obj->notifyFreeBottom();
        }
    }
    if (!on_the_floor){
        obj->notifyFreeBottom();
    }
}