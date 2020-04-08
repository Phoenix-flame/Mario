#include "koopa_troopa.hpp"




void Koopa::update(){
    if(!visited){return;}

    
    if (state == KOOPA_ATTACK_STATE){
        updateFigure();
        return;
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE){
        if (dir == LEFT){
            _moveX(-4);
        }
        else if (dir == RIGHT){
            _moveX(+4); 
        }
        if (state == KOOPA_FALL_STATE){
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




// Collision Notification
void Koopa::notifyCollisionLeft(Object* obj){
    if (state == KOOPA_WALK_STATE){
        if (obj->getType() == PLAYER){
            obj->death();
        }
        else if (dir == LEFT){
            dir = RIGHT;
        } 
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE){
        if (obj->getType() == GOOMBA || obj->getType() == PLAYER){
            obj->death();
        }
        else{
            if (dir == LEFT){
                dir = RIGHT;
            }
        }
    }
}
void Koopa::notifyCollisionRight(Object* obj){
    if (state == KOOPA_WALK_STATE){
        if (obj->getType() == PLAYER){
            obj->death();
        }
        else if (dir == RIGHT){
            dir = LEFT;
        } 
    }
    else if (state == KOOPA_FAST_AND_FURIOUS_STATE){
        if (obj->getType() == GOOMBA || obj->getType() == PLAYER){
            obj->death();
        }
        else{
            if (dir == RIGHT){
                dir = LEFT;
            } 
        }
    }
}
void Koopa::notifyCollisionTop(Object* obj){
  
}
void Koopa::notifyCollisionBottom(Object* obj){
    endFall();
}

void Koopa::notifyFreeLeft(){}
void Koopa::notifyFreeRight(){}
void Koopa::notifyFreeTop(){}
void Koopa::notifyFreeBottom(){
    if (state != KOOPA_FALL_STATE){
        startFall();
    }
}

void Koopa::notifyDistToPlatform(int d){
    if (fall_speed_vertical > d){
        fall_speed_vertical = d;
    }
}

void Koopa::notifyDistToCeil(int d){

}