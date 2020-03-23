#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
}


void World::loop(){
    
    gravity();
    collision();
}


std::vector<Object*> World::getObjects(){
    return map->objects;
}






bool World::collisionGravity(Object* o1, Object* o2){

    Point o1_xRange = o1->getXRange();
    int o1_left = o1_xRange.x;
    int o1_right = o1_xRange.y;
    Point o1_yRange = o1->getYRange();
    int o1_top = o1_yRange.x;
    int o1_bottom = o1_yRange.y;
    Point o2_xRange = o2->getXRange();
    int o2_left = o2_xRange.x;
    int o2_right = o2_xRange.y;
    Point o2_yRange = o2->getYRange();
    int o2_top = o2_yRange.x;
    int o2_bottom = o2_yRange.y;

    
    if (o1_right >= o2_left && o1_left <= o2_right){
        if (o1_bottom >= (o2_top - 3) && o1_top <= o2_bottom){
            return true;
        }
    }
    if (o1_right == o2_right && o1_left == o2_left){
        if (o1_bottom >= (o2_top - 3) && o1_top <= o2_bottom){
            return true;
        }
    }
    return false;
    
}

bool World::collisionJump(Object* o1, Object* o2){

    Point o1_xRange = o1->getXRange();
    int o1_left = o1_xRange.x;
    int o1_right = o1_xRange.y;
    Point o1_yRange = o1->getYRange();
    int o1_top = o1_yRange.x;
    int o1_bottom = o1_yRange.y;
    Point o2_xRange = o2->getXRange();
    int o2_left = o2_xRange.x;
    int o2_right = o2_xRange.y;
    Point o2_yRange = o2->getYRange();
    int o2_top = o2_yRange.x;
    int o2_bottom = o2_yRange.y;

    
    if (o1_right >= o2_left && o1_left <= o2_right){
        if (o2_bottom >= (o1_top - 3) && o2_top <= o1_bottom){
            return true;
        }
    }
    if (o1_right == o2_right && o1_left == o2_left){
        if (o2_bottom >= (o1_top - 3) && o2_top <= o1_bottom){
            return true;
        }
    }
    return false;
    
}

void World::collision(){
    Player* player = map->player;
    bool below_sth = false;
    for (auto o:map->grounds){
        if (collisionJump(player, o)){
            below_sth = true;
        }
    }
    for (auto o:map->bricks){
        if (collisionJump(player, o)){
            below_sth = true;
        }
    }
    for (auto o:map->blocks){
        if (collisionJump(player, o)){
            below_sth = true;
        }
    }
    for (auto o:map->coins){
        if (collisionJump(player, o)){
            below_sth = true;
        }
    }
    for (auto o:map->fires){
        if (collisionJump(player, o)){
            below_sth = true;
        }
    }
    for (auto o:map->healths){
        if (collisionJump(player, o)){
            below_sth = true;
        }
    }
    if (player->getState() == JUMP && below_sth){
        player->endJump();
    }
}

void World::gravity(){
    Player* player = map->player;
    
    bool on_the_floor = false;
    for (auto o:map->grounds){
        if (collisionGravity(player, o)){
            player->endFall();
            on_the_floor = true;
        }
    }
    for (auto o:map->bricks){
        if (collisionGravity(player, o)){
            player->endFall();
            on_the_floor = true;
        }
    }
    for (auto o:map->blocks){
        if (collisionGravity(player, o)){
            player->endFall();
            on_the_floor = true;
        }
    }
    for (auto o:map->coins){
        if (collisionGravity(player, o)){
            player->endFall();
            on_the_floor = true;
        }
    }
    for (auto o:map->fires){
        if (collisionGravity(player, o)){
            player->endFall();
            on_the_floor = true;
        }
    }
    for (auto o:map->healths){
        if (collisionGravity(player, o)){
            player->endFall();
            on_the_floor = true;
        }
    }
    // std::cout << (on_the_floor?"stand":"falling") << std::endl;
    if (!on_the_floor){
        player->startFall();
    }
    
}