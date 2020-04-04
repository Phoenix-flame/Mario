#include "mushroom.hpp"



void Mushroom::update(std::vector<Object*> objs){
    if (pos.x >= x_gravity_en){
        gravity_en = true;
    }
    if (gravity_en){
        gravity(objs);
        collision(objs);
    }
    
    
    min_dist_to_platform = checkDistToPlatform(objs);


    if (state == M_FALL){
        if (fall_speed_vertical > min_dist_to_platform.x){
            fall_speed_vertical = min_dist_to_platform.x;
        }
        funcToRun = &Mushroom::falling;
    }
    else if (state == M_RUN){
        funcToRun = &Mushroom::run;
    }


    (this->*funcToRun)();
}



void Mushroom::riseLikeThePhoenix(){
    if (pos.y != y_target){
        _moveY(-1);
    }
    else {
        funcToRun = &Mushroom::run;
    }
}


void Mushroom::run(){
    

    _moveX(speed);
}



void Mushroom::startFall(){
    state = M_FALL;
    fall_speed_vertical = 3;
    fall_cycles = 0;
}

void Mushroom::falling(){
     _moveY(fall_speed_vertical);
    _moveX(speed);
    fall_speed_vertical += (fall_cycles%2 == 0)?1:0;
    if (fall_speed_vertical >= terminal_speed){
        fall_speed_vertical = terminal_speed;
    }
    fall_cycles += 1;
}

void Mushroom::endFall(){
    state = M_RUN;
}

int Mushroom::collisionGravity(Rectangle o1, Rectangle o2){
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


void Mushroom::gravity(std::vector<Object*> objs){
    bool on_the_floor = false;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if (collisionGravity(o1, o2) != -1){
            endFall();
            on_the_floor = true;
        }
    }

    if (!on_the_floor){
        if (state != M_FALL){
            startFall();
        }
        
    }
}


Point Mushroom::checkDistToPlatform(std::vector<Object*> objs){
    int min_dist_b = DBL_MAX;
    int min_dist_t = DBL_MAX;
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
        int dist_t = -1;
        if (o1_right >= o2_left && o1_left <= o2_right){
            if (o1_bottom <= o2_top){
                dist_b = abs(o1_bottom - o2_top);
            }
            if (o1_top >= o2_bottom){
                dist_t = abs(o1_top - o2_bottom);
            }
        }
        
        if (dist_b != -1){
            if (dist_b < min_dist_b){
                min_dist_b = dist_b;
            }
        }
        if (dist_t != -1){
            if (dist_t < min_dist_t){
                min_dist_t = dist_t;
            }
        }

    }
    return Point(min_dist_b, min_dist_t);
}



void Mushroom::collision(std::vector<Object*> objs){
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if ((o1.right_center.y >= o2.y && o1.right_center.y <= (o2.y + o2.h)) ||
            (o1.y >= o2.y && o1.y <= (o2.y + o2.h)) ||
            ((o1.y + o1.h - 3) >= o2.y && (o1.y + o1.h - 3) <= (o2.y + o2.h))){
            if (abs(o1.right_center.x - o2.left_center.x) < 4){
                if(o->getType() != GOOMBA && o->getType() != KOOPA){
                    speed *= -1;
                }
                
            }
            else if (abs(o1.left_center.x - o2.right_center.x) < 4){
                if(o->getType() != GOOMBA && o->getType() != KOOPA){
                    speed *= -1;
                }
            }
        } 
    }
}