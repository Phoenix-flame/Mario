#include "world.hpp"

World::World(){
    this->camera = new Camera();
    this->map = new Map("assets/maps/1/1.txt");
    this->gameState = new GameState();
}


void World::loop(){
    ghostCollector();
    collision(map->player);
    for(unsigned int i = 0 ; i < map->objects.size() ; i++){
        Type t = map->objects[i]->getType();
        if(t == PLAYER){continue;}
        else if (t == BRICK){
            ((Brick*)map->objects[i])->update();
            if (((Brick*)map->objects[i])->broken){
                map->objects.erase(map->objects.begin() + i);
            }
        }
        else if (t == COIN_CONTAINER){
            ((CoinContainer*)map->objects[i])->update();
        }
        else if (t == FIRE_CONTAINER){
            ((FireContainer*)map->objects[i])->update();
        }
        else if (t == HEALTH_CONTAINER){
            ((HealthContainer*)map->objects[i])->update();
        }
        else if (t == GOOMBA){
            if ((map->objects[i]->getPos() + camera->getPos()).x < 700){
                ((Goomba*)map->objects[i])->seen();
            } 
            collision(map->objects[i]);


            if (map->objects[i]->getPos().y > 500){
                map->objects.erase(map->objects.begin() + i);
            }
            
            ((Goomba*)map->objects[i])->update();
        }
        else if (t == KOOPA){
            if ((map->objects[i]->getPos() + camera->getPos()).x < 700){
                ((Koopa*)map->objects[i])->seen();
            } 
            collision(map->objects[i]);
            if (map->objects[i]->getPos().y > 500){
                map->objects.erase(map->objects.begin() + i);
            }
            
            ((Koopa*)map->objects[i])->update();
        }
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
            collision(ghosts[i]);
            ((Mushroom*)ghosts[i])->update();
        }
        else if (ghosts[i]->getType() ==G_FLOWER){
            collision(ghosts[i]);
            ((Flower*)ghosts[i])->update();
        }
        
    }
}

void World::ghostCollector(){
    for (auto obj:map->objects){
        if (obj->has_ghost){
            for(auto g:obj->ghost){
                ghosts.push_back(g);
                if (g->getType() == G_TEXT){
                    gameState->score += ((Text*)g)->score;
                }
            }
            obj->has_ghost = false;
            obj->ghost.clear();
        }
    }
}


std::vector<Object*> World::getObjects(){
    return map->objects;
}

GameState* World::getGameState(){
    return gameState;
}



Player* World::getPlayer(){
    return map->player;
}

std::vector<Object*> World::getGhosts(){
    return ghosts;
}



void World::collision(Object* obj){
    if (obj->dead) {return;}
    bool on_the_floor = false;
    bool collision_with_center = false;
    bool collided = false;
    Object* selected;

    obj->notifyFreeLeft();
    obj->notifyFreeRight();
    for (auto o:getObjects()){
        if (o->dead){continue;}
        Rectangle o1(obj->getPos(), obj->getPos() + obj->getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        // Left and right collision
        if ((o1.right_center.y >= o2.y && o1.right_center.y <= (o2.y + o2.h)) ||
            (o1.y >= o2.y && o1.y <= (o2.y + o2.h)) ||
            ((o1.y + o1.h - 1) >= o2.y && (o1.y + o1.h - 1) <= (o2.y + o2.h))){
            if (abs(o1.right_center.x - o2.left_center.x) < 5){
                // TODO notify either objects Done
                obj->notifyCollisionRight(o);
                o->notifyCollisionLeft(obj);
            }
            else if (abs(o1.left_center.x - o2.right_center.x) < 5){
                // TODO notify either objects Done
                obj->notifyCollisionLeft(o);
                o->notifyCollisionRight(obj);
            }
        } 
        // if (){}
        static int counter = 0;
        // Bottom collision
        int left = o1.left_bottom.x + 2;
        int right = o1.right_bottom.x -2;
        if ((o1.bottom_center.x >= o2.left_bottom.x && o1.bottom_center.x <= o2.right_bottom.x)){
            if (o1.bottom_center.y <= o2.top_center.y && abs(o1.bottom_center.y - o2.top_center.y) < 3){
                on_the_floor = true;
                obj->notifyCollisionBottom(o);
            }
            else{
                obj->notifyDistToPlatform(abs(o1.bottom_center.y - o2.top_center.y));
            }
        }
        
        else if (left >= o2.left_bottom.x && left <= o2.right_bottom.x){
            if (o1.bottom_center.y <= o2.top_center.y && abs(o1.bottom_center.y - o2.top_center.y) < 3){
                on_the_floor = true;
                obj->notifyCollisionBottom(o);
            }
            else{
                obj->notifyDistToPlatform(abs(o1.bottom_center.y - o2.top_center.y));
            }
        }
        
        else if (right >= o2.left_bottom.x && right <= o2.right_bottom.x){
            if (o1.bottom_center.y <= o2.top_center.y && abs(o1.bottom_center.y - o2.top_center.y) < 3){
                on_the_floor = true;
                obj->notifyCollisionBottom(o);
            }
            else{
                obj->notifyDistToPlatform(abs(o1.bottom_center.y - o2.top_center.y));
            }
        }


        // Top Collision Just for player
        if (obj->getType() == PLAYER){
            if ((o1.top_center.x >= o2.left_top.x && o1.top_center.x <= o2.right_top.x)){
                if (o1.top_center.y >= o2.bottom_center.y && o1.top_center.y - o2.bottom_center.y < 5){
                    obj->notifyCollisionTop(o);
                    collision_with_center = true;
                    collided = true;
                    return;
                }
                else {
                    obj->notifyDistToCeil(abs(o1.top_center.y - o2.bottom_center.y));
                }
            }
            else if (!collision_with_center){
                if ((o1.left_top.x + 2) >= o2.left_top.x && (o1.left_top.x + 2) <= o2.right_top.x){
                    if (o1.top_center.y >= o2.bottom_center.y && o1.top_center.y - o2.bottom_center.y < 5){
                        if (((Player*)obj)->getState() == JUMP){
                            collided = true;
                            selected = o;
                        }
                    }
                    else {
                        obj->notifyDistToCeil(abs(o1.top_center.y - o2.bottom_center.y));
                    }
                }
                else if ((o1.right_top.x - 2) >= o2.left_top.x && (o1.right_top.x - 2) <= o2.right_top.x){
                    if (o1.top_center.y >= o2.bottom_center.y && o1.top_center.y - o2.bottom_center.y < 5){
                        if (((Player*)obj)->getState()  == JUMP){
                            collided = true;
                            selected = o;
                        }
                    }
                    else {
                        obj->notifyDistToCeil(abs(o1.top_center.y - o2.bottom_center.y));
                    }
                }
            }
        }
        
    }
    if (!on_the_floor){
        obj->notifyFreeBottom();
    }
    if (obj->getType() == PLAYER && !collision_with_center && collided){
        obj->notifyCollisionTop(selected);
    }
}
