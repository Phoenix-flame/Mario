#include "goomba.hpp"

void Goomba::update(std::vector<Object*> objs){
    if(!visited){return;}
    if(state == GOOMBA_DEAD_STATE){
        if (!deathAnimation.isStarted()){
            deathAnimation.start();
        }
        if (deathAnimation.getTime() < 100){
            fall_speed_vertical = 3;
            _moveY(-5);
        }
        else if (deathAnimation.getTime() >= 200){
            falling();
        }
        updateFigure();
        return;
    }
    gravity(objs);
    collision(objs);
    int min_dist_to_platform = checkDistToPlatform(objs);
    if (dir == LEFT){
        _moveX(-1);
    }
    else if (dir == RIGHT){
        _moveX(+1); 
    }
    
    if (state == GOOMBA_FALL_STATE){
        if (fall_speed_vertical > min_dist_to_platform){
            fall_speed_vertical = min_dist_to_platform;
        }
        falling();
    }
    updateFigure();
}
void Goomba::updateFigure(){
    if (state == GOOMBA_WALK_STATE){
        if (walk_state == 1){
            image = GOOMBA_WALK1_IMAGE;
            if (!walkTimer.isStarted()){walkTimer.start();}
            if (walkTimer.getTime() > 200){
                walk_state = 2;
                walkTimer.reset();
            }
        }
        else if (walk_state == 2){
            image = GOOMBA_WALK2_IMAGE;
            if (!walkTimer.isStarted()){walkTimer.start();}
            if (walkTimer.getTime() > 200){
                walk_state = 1;
                walkTimer.reset();
            }
        }
    }
    else if (state == GOOMBA_DEAD_STATE){
        image = GOOMBA_DEAD_IMAGE;
    }
}

void Goomba::gravity(std::vector<Object*> objs){
    bool on_the_floor = false;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if (collisionGravity(o1, o2) != -1){
            if (state != GOOMBA_DEAD_STATE) endFall();
            on_the_floor = true;
        }
        else{
        }
    }

    if (!on_the_floor){
        if (state != GOOMBA_FALL_STATE){
            startFall();
        }
        
    }
}


int Goomba::checkDistToPlatform(std::vector<Object*> objs){
    int min_dist_b = DBL_MAX;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        int o1_top = o1.y;
        int o1_bottom = o1.y + o1.h;
        int o2_top = o2.y;
        int o2_bottom = o2.y + o2.h;
        
        int o1_left = o1.x;
        int o1_right = o1.x + o1.w;
        int o2_left = o2.x;
        int o2_right = o2.x + o2.w;


        int dist_b = -1;
        if (o1_right >= o2_left && o1_left <= o2_right){
            if (o1_bottom <= o2_top){
                dist_b = abs(o1_bottom - o2_top);
            }
        }
        
        if (dist_b != -1){
            if (dist_b < min_dist_b){
                min_dist_b = dist_b;
            }
        }

    }
    return min_dist_b;
}


int Goomba::collisionGravity(Rectangle o1, Rectangle o2){
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



void Goomba::startFall(){
    state = GOOMBA_FALL_STATE;
    fall_speed_vertical = 3;
}

void Goomba::falling(){
    _moveY(fall_speed_vertical);
    fall_speed_vertical += (fall_cycles%2 == 0)?1:0;
    if (fall_speed_vertical >= 10){
        fall_speed_vertical = 10;
    }
    fall_cycles += 1;


    _moveX(0);


    
}


void Goomba::endFall(){
    if (state == GOOMBA_FALL_STATE){
        state = GOOMBA_WALK_STATE;
    }    
}


void Goomba::collision(std::vector<Object*> objs){
    for (auto o:objs){
        // if (o->getType() == GOOMBA || o->getType() == KOOPA){continue;}
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        int o1_center = o1.y + o1.h/2.0;
        int o2_top = o2.y;
        int o2_bottom = o2.y + o2.h;
        
        int o1_left = o1.x;
        int o1_right = o1.x + o1.w;
        int o2_left = o2.x;
        int o2_right = o2.x + o2.w;

        if (o1_center >= o2_top && o1_center <= (o2_bottom - 5)){
            if (o1_right <= o2_left && abs(o1_right - o2_left) < 2){
                if (state == GOOMBA_WALK_STATE){
                    if (dir == RIGHT){
                        dir = LEFT;
                    } 
                }
            }
            else if (o1_left >= o2_right && abs(o1_left - o2_right) < 2){
                if (state == GOOMBA_WALK_STATE){
                    if (dir == LEFT){
                        dir = RIGHT;
                    } 
                }
            }
        }
    }
}



void Goomba::death(){
    // Very simple death
    state = GOOMBA_DEAD_STATE;
}
