#include "player.hpp"

void Player::update(std::vector<Object*> objs, int _dir){
    Dir trans_dir = (_dir == 1)?RIGHT:LEFT;
    bool stop = (_dir == 0)?true:false;
    
    // Movement profile
    if (state == DEAD){
        speed = 0;
        funcToRun = &Player::death_animation;
    }
    else{
        // Physics
        jumpCollision(objs);
        // collision(objs);
        min_dist_to_platform = checkDistToPlatform(objs);
    }
    if (state != DEAD){
        if (state == STAND && stop){
            funcToRun = &Player::stand;
        }
        else if (state == STAND && !stop) {
            dir = trans_dir;
            if (dir == RIGHT && collidedR){
                state = STAND;
                speed = 0;
                funcToRun = &Player::stand;
            }
            else if (dir == LEFT && collidedL){
                state = STAND;
                speed = 0;
                funcToRun = &Player::stand;
            }
            else{
                startMove();
                funcToRun = &Player::normalMove;
            }
            
        }
        else if (state == WALK && stop){
            endMove();
            funcToRun = &Player::normalMove;
        }
        else if (state == JUMP){
            if (stop){
                speed = 0;
            }
            else if (!collidedR && !collidedL){
                dir = (stop)?STOP:trans_dir;
                if (dir == LEFT){speed = -5;}
                else if (dir == RIGHT){speed = +5;}
                else{speed = 0;}
            }
            if (abs(jump_speed_vertical) > min_dist_to_platform.y){
                jump_speed_vertical = -min_dist_to_platform.y;
            }
            funcToRun = &Player::jump;
        }
        else if (state == FALL){
            if (stop){
                speed = 0;
            }
            else if(!collidedL && !collidedR){
                dir = (stop)?STOP:trans_dir;
                if (dir == LEFT){speed = -5;}
                else if (dir == RIGHT){speed = 5;}
                else{speed = 0;}
            }

            // It's important
            if (fall_speed_vertical > min_dist_to_platform.x){
                fall_speed_vertical = min_dist_to_platform.x;
            }


            funcToRun = &Player::falling;
        }
    }
    
 
    // Update Player State
    (this->*funcToRun)();
    updateFigure();
}

void Player::updateFigure(){
    static int walk_left = 1;
    static int walk_right = 1;
    switch (level)
    {
    case NORMAL:
        if (state == STAND){
            image = (dir == LEFT)?NORM_STAND_LEFT:NORM_STAND_RIGHT;
        }
        else if (state == WALK){
            if (dir == LEFT){
                walk_right = 1;
                if (walk_left == 1){
                    image = NORM_WALK_LEFT1;
                    if (!LW_Timer.isStarted()) LW_Timer.start();
                    if (LW_Timer.getTime() > 30){
                        walk_left = 2;
                        LW_Timer.reset();
                    }
                    
                }
                else if (walk_left == 2){
                    image = NORM_WALK_LEFT2; 
                    if (!LW_Timer.isStarted()) LW_Timer.start();
                    if (LW_Timer.getTime() > 30){
                        walk_left = 3;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 3){
                    image = NORM_WALK_LEFT3; 
                    if (!LW_Timer.isStarted()) LW_Timer.start();
                    if (LW_Timer.getTime() > 30){
                        walk_left = 1;
                        LW_Timer.reset();
                    }
                }
            }
            else if (dir == RIGHT){
                walk_left = 1;
                if (walk_right == 1){ 
                    image = NORM_WALK_RIGHT1; 
                    if (!RW_Timer.isStarted()){
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30){
                        walk_right = 2;
                        RW_Timer.reset();
                    }
                    
                }
                else if (walk_right == 2){
                    image = NORM_WALK_RIGHT2; 
                    if (!RW_Timer.isStarted()){
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30){
                        walk_right = 3;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 3){
                    image = NORM_WALK_RIGHT3; 
                    if (!RW_Timer.isStarted()){
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30){
                        walk_right = 1;
                        RW_Timer.reset();
                    }
                }
            }
        }
        else if (state == DEAD){
            image = NORM_DEAD;
        }
        else if (state == JUMP){
            image = (dir == LEFT)?NORM_JUMP_LEFT:NORM_JUMP_RIGHT;
        }
        else if (state == SLIDE){
            image = (dir == LEFT)?NORM_SLIDE_LEFT:NORM_SLIDE_RIGHT;
        }
        else if (state == FALL){
            image = (dir == LEFT)?NORM_JUMP_LEFT:NORM_JUMP_RIGHT;
        }
        break;
    case BIG:
        if (state == STAND){
            image = (dir == LEFT)?BIG_STAND_LEFT:BIG_STAND_RIGHT;
        }
        else if (state == WALK){
            if (dir == LEFT){
                walk_right = 1;
                if (walk_left == 1){
                    image = BIG_WALK_LEFT1;
                    if (!LW_Timer.isStarted()) LW_Timer.start();
                    if (LW_Timer.getTime() > 30){
                        walk_left = 2;
                        LW_Timer.reset();
                    }
                    
                }
                else if (walk_left == 2){
                    image = BIG_WALK_LEFT2; 
                    if (!LW_Timer.isStarted()) LW_Timer.start();
                    if (LW_Timer.getTime() > 30){
                        walk_left = 3;
                        LW_Timer.reset();
                    }
                }
                else if (walk_left == 3){
                    image = BIG_WALK_LEFT3; 
                    if (!LW_Timer.isStarted()) LW_Timer.start();
                    if (LW_Timer.getTime() > 30){
                        walk_left = 1;
                        LW_Timer.reset();
                    }
                }
            }
            else if (dir == RIGHT){
                walk_left = 1;
                if (walk_right == 1){ 
                    image = BIG_WALK_RIGHT1; 
                    if (!RW_Timer.isStarted()){
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30){
                        walk_right = 2;
                        RW_Timer.reset();
                    }
                    
                }
                else if (walk_right == 2){
                    image = BIG_WALK_RIGHT2; 
                    if (!RW_Timer.isStarted()){
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30){
                        walk_right = 3;
                        RW_Timer.reset();
                    }
                }
                else if (walk_right == 3){
                    image = BIG_WALK_RIGHT3; 
                    if (!RW_Timer.isStarted()){
                        RW_Timer.start();
                    }
                    if (RW_Timer.getTime() > 30){
                        walk_right = 1;
                        RW_Timer.reset();
                    }
                }
            }
        }
        else if (state == DEAD){
            
        }
        else if (state == JUMP){
            image = (dir == LEFT)?BIG_JUMP_LEFT:BIG_JUMP_RIGHT;
        }
        else if (state == SLIDE){
            image = (dir == LEFT)?BIG_SLIDE_LEFT:BIG_SLIDE_RIGHT;
        }
        else if (state == FALL){
            image = (dir == LEFT)?BIG_JUMP_LEFT:BIG_JUMP_RIGHT;
        }
        break;
    case POWER:
        if (state == STAND){

        }
        else if (state == WALK){

        }
        else if (state == DEAD){
            
        }
        else if (state == JUMP){
            
        }
        else if (state == SLIDE){
            
        }
        break;
    default:
        break;
    }
}




void Player::startMove(){
    state = WALK;
    slide_enable = 0;
    
}

void Player::normalMove(){
    if (dir == LEFT){
        speed = -5;
        if (speed < -5){
            speed = -5;
        }
    }
    else if (dir == RIGHT){
        speed = 5;
        if (speed > 5){
            speed = 5;
        }
    }
    if(state == SLIDE){
        if (slideTimer.getTime() > 200){
            state = STAND;
            speed = 0;
            slide_enable = 0;
        }
        else{
            _moveX(speed);  
        }
    }
    else{
        _moveX(speed);
        slide_enable += 1;
    }
}

void Player::stand(){
    
}

void Player::endMove(){
    if (slide_enable < 20){
        state = STAND;
        speed = 0;
        return;
    }
    else if (state == WALK){
        slideTimer.reset();
        slideTimer.start();
        state = SLIDE;
        if (dir == LEFT){speed = -5;}
        else if (dir == RIGHT) {speed = 5;}
        
    } 
}



void Player::startJump(){
    if (state == JUMP || state == FALL || state == DEAD){return;} // You cannot initiate a jump during a jump
    if (state == STAND){ 
        move_during_jump = false;
    }
    else {
        move_during_jump = true;
    }
    state = JUMP;
    jump_speed_vertical = -10;
    jumpTimer.reset();
    jumpTimer.start();
}


void Player::jump(){
    static int jump_cycles = 0;
    
    if (jumpTimer.getTime() < 600){
        _moveY(jump_speed_vertical);
        jump_speed_vertical += (jump_cycles%2 == 0)?1:0;
        jump_cycles += 1;
        if (jump_speed_vertical > 0){
            jump_speed_vertical = 0;
        }
    }
    else {
        state = FALL;
        fall_speed_vertical = -jump_speed_vertical;
    }
    _moveX(speed);
}

void Player::endJump(){
    state = FALL;
}

void Player::startFall(){
    if (state != JUMP){  // when mario is jumping, gravity has no effect on it
        state = FALL;
        fall_speed_vertical = 3;
    }   
}

void Player::falling(){
    _moveY(fall_speed_vertical);
    _moveX(speed);
    fall_speed_vertical += (fall_cycles%2 == 0)?1:0;
    if (fall_speed_vertical >= terminal_speed){
        fall_speed_vertical = terminal_speed;
    }
    fall_cycles += 1;
}


void Player::endFall(){
    if (state == FALL){
        state = STAND;
        speed = 0;
    }
    
}




Point Player::checkDistToPlatform(std::vector<Object*> objs){
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


void Player::jumpCollision(std::vector<Object*> objs){
    Object* selected;
    bool collision_with_center = false;
    bool collided = false;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());

        if ((o1.top_center.x >= o2.left_top.x && o1.top_center.x <= o2.right_top.x)){
            if (o1.top_center.y >= o2.bottom_center.y && abs(o1.top_center.y - o2.bottom_center.y) < 5){
                if (state == JUMP){
                    endJump();
                    collision_with_center = true;
                    collided = true;

                    o->mark();
                    return;
                }
            }
        }
        else if (!collision_with_center){
            if (o1.left_top.x >= o2.left_top.x && o1.left_top.x <= o2.right_top.x){
                if (o1.top_center.y >= o2.bottom_center.y && abs(o1.top_center.y - o2.bottom_center.y) < 5){
                    if (state == JUMP){
                        collided = true;
                        selected = o;
                    }
                }
            }
            else if (o1.right_top.x >= o2.left_top.x && o1.right_top.x <= o2.right_top.x){
                if (o1.top_center.y >= o2.bottom_center.y && abs(o1.top_center.y - o2.bottom_center.y) < 5){
                    if (state == JUMP){
                        collided = true;
                        selected = o;
                    }
                }
            }
        }
    }
    if (!collision_with_center && collided){
        endJump();
        selected->mark();
    }
    
}





void Player::kill(Object* obj){
    obj->death();
}

void Player::dead(){

}

void Player::death(){
    state = DEAD;
    speed = 0;
    terminal_speed = 100;
}


void Player::death_animation(){
    if (!deathAnimation.isStarted()){
        deathAnimation.start();
    }
    if (deathAnimation.getTime() < 300){
        fall_speed_vertical = 3;
        _moveY(-5);
    }
    else if (deathAnimation.getTime() >= 350){
        falling();
    }
}


// Collision Notification
void Player::notifyCollisionLeft(Object* obj){
    if (state == DEAD){
        return;
    }
    if(dir == LEFT){
        collidedL = true;
        speed = 0;
        if (state == WALK || state == SLIDE){
            state = STAND;
        } 
        else if (state == JUMP){
            speed = 0;
        } 
        if (obj->getType() == GOOMBA){
            death();
        }
    }
}
void Player::notifyCollisionRight(Object* obj){
    if (state == DEAD){
        return;
    }
    if (dir == RIGHT){
        collidedR = true;
        speed = 0;  
        if (state == WALK || state == SLIDE){
            state = STAND;
        }
        else if (state == JUMP){
            speed = 0;
        } 
        if (obj->getType() == GOOMBA){
            death();
        }
    }
}
void Player::notifyCollisionTop(Object* obj){

}
void Player::notifyCollisionBottom(Object* obj){
    if (state == DEAD){
        return;
    }
    endFall();

    // Kill enemies by jumping on their head :)
    if (obj->getType() == GOOMBA){  // TODO FIX THIS
        kill(obj);
    }
    else if (obj->getType() == KOOPA){
        // if (!koopa_hit){
        //     kill(obj);
        //     koopa_hit = true;
        // }
    }
}

void Player::notifyFreeLeft(){ collidedL = false; }
void Player::notifyFreeRight(){ collidedR = false;}
void Player::notifyFreeTop(){}
void Player::notifyFreeBottom(){
    if (state == DEAD){
        return;
    }
    // if (obj->getType() == KOOPA){
    //     koopa_hit = false;
    // }
    if (state != FALL){
        startFall();
    } 
}