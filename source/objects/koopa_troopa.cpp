#include "koopa_troopa.hpp"




void Koopa::update(std::vector<Object*> objs){
    if(!visited){return;}
    

    gravity(objs);
    collision(objs);
    int min_dist_to_platform = checkDistToPlatform(objs);
    
    if (state == KOOPA_ATTACK_STATE){
        updateFigure();
        return;
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE){
        if (dir == LEFT){
            _moveX(-1);
        }
        else if (dir == RIGHT){
            _moveX(+1); 
        }
        if (state == KOOPA_FALL_STATE){
            if (fall_speed_vertical > min_dist_to_platform){
                fall_speed_vertical = min_dist_to_platform;
            }
            falling();
        }
        
        updateFigure();
        return;
    }

    
    // Normal state
    if (dir == LEFT){
        _moveX(-1);
    }
    else if (dir == RIGHT){
        _moveX(+1); 
    }
    
    if (state == KOOPA_FALL_STATE){
        if (fall_speed_vertical > min_dist_to_platform){
            fall_speed_vertical = min_dist_to_platform;
        }
        falling();
    }
    
    updateFigure();

}



void Koopa::updateFigure(){
    if(state == KOOPA_WALK_STATE){
        if (dir == RIGHT){
            if (right_walk_state == 1){
                image = KOOPA_WALK_RIGHT1;
                if (!RW_Timer.isStarted()){
                    RW_Timer.start();
                }
                if (RW_Timer.getTime() > 80){
                    right_walk_state = 2;
                    RW_Timer.reset();
                }
            }
            else if (right_walk_state == 2){
                image = KOOPA_WALK_RIGHT2;
                if (!RW_Timer.isStarted()){
                    RW_Timer.start();
                }
                if (RW_Timer.getTime() > 80){
                    right_walk_state = 1;
                    RW_Timer.reset();
                }
            }
        }
        else if (dir == LEFT){
            if (left_walk_state == 1){
                image = KOOPA_WALK_LEFT1;
                if (!LW_Timer.isStarted()){
                    LW_Timer.start();
                }
                if (LW_Timer.getTime() > 200){
                    left_walk_state = 2;
                    LW_Timer.reset();
                }
            }
            else if (left_walk_state == 2){
                image = KOOPA_WALK_LEFT2;
                if (!LW_Timer.isStarted()){
                    LW_Timer.start();
                }
                if (LW_Timer.getTime() > 200){
                    left_walk_state = 1;
                    LW_Timer.reset();
                }
            }
        }
    }
    else if (state == KOOPA_ATTACK_STATE || state == KOOPA_FAST_AND_FURIOUS_STATE){
        image = KOOPA_DEAD;
    }
}





void Koopa::gravity(std::vector<Object*> objs){
    bool on_the_floor = false;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if (collisionGravity(o1, o2) != -1){
            endFall();
            on_the_floor = true;
        }
        else{
        }
    }

    if (!on_the_floor){
        if (state != KOOPA_FALL_STATE){
            startFall();
        }
        
    }
}


int Koopa::checkDistToPlatform(std::vector<Object*> objs){
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


int Koopa::collisionGravity(Rectangle o1, Rectangle o2){
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



void Koopa::startFall(){
    if (state == KOOPA_ATTACK_STATE){return;}
    state = KOOPA_FALL_STATE;
    fall_speed_vertical = 3;
}

void Koopa::falling(){
    _moveY(fall_speed_vertical);
    fall_speed_vertical += (fall_cycles%2 == 0)?1:0;
    if (fall_speed_vertical >= 10){
        fall_speed_vertical = 10;
    }
    fall_cycles += 1;


    _moveX(0);


    
}


void Koopa::endFall(){
    if (state == KOOPA_ATTACK_STATE){
        return;
    }
    if (state == KOOPA_FALL_STATE){
        state = KOOPA_WALK_STATE;
    }    
}


void Koopa::collision(std::vector<Object*> objs){
    for (auto o:objs){
        if (o->getType() == GOOMBA || o->getType() == KOOPA){continue;}
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
            if (o1_right <= o2_left && abs(o1_right - o2_left) < 6){
                if (state == KOOPA_WALK_STATE){
                    if (dir == RIGHT){
                        dir = LEFT;
                    } 
                }
                else if (state == KOOPA_FAST_AND_FURIOUS_STATE){
                    if (o->getType() == GOOMBA){
                        o->death();
                    }
                }
            }
            else if (o1_left >= o2_right && abs(o1_left - o2_right) < 6){
                if (state == KOOPA_WALK_STATE){
                    if (dir == LEFT){
                        dir = RIGHT;
                    } 
                }
                else if (state == KOOPA_FAST_AND_FURIOUS_STATE){
                    if (o->getType() == GOOMBA){
                        o->death();
                    }
                }
            }
        }
    }
}



void Koopa::death(){
    // Very simple death
    dead = true;
    if (state == KOOPA_WALK_STATE){
        state = KOOPA_ATTACK_STATE;
    }
    else if (state == KOOPA_ATTACK_STATE){
        attack = true;
        dir = RIGHT;
        state = KOOPA_FAST_AND_FURIOUS_STATE;
    }
}