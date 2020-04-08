#include "mushroom.hpp"



void Mushroom::update(){
    if(pos.x > x_gravity_en){
        gravity_en = true;
    }
    if (state == M_FALL){
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
    fall_cycles += 1;
}

void Mushroom::endFall(){
    state = M_RUN;
}





// Collision Notification
void Mushroom::notifyCollisionLeft(Object* obj){
    Type t = obj->getType();
    if(t== KOOPA || t == GOOMBA){
        return;
    }
    if (t == PLAYER){
    }
    speed *= -1;
    
}
void Mushroom::notifyCollisionRight(Object* obj){
    Type t = obj->getType();
    if(t== KOOPA || t == GOOMBA){
        return;
    }
    if (t == PLAYER){
    }
    speed *= -1;
    
}
void Mushroom::notifyCollisionTop(Object* obj){
  
}
void Mushroom::notifyCollisionBottom(Object* obj){
    endFall();
}

void Mushroom::notifyFreeLeft(){}
void Mushroom::notifyFreeRight(){}
void Mushroom::notifyFreeTop(){}
void Mushroom::notifyFreeBottom(){
    if (!gravity_en){return;}
    if (state != M_FALL){
        startFall();
    }
}

void Mushroom::notifyDistToPlatform(int d){
    if (fall_speed_vertical > d){
        fall_speed_vertical = d;
    }
}

void Mushroom::notifyDistToCeil(int d){

}