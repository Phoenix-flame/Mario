#include "player.hpp"

void Player::update(std::vector<Object*> objs, int _dir){
    Dir trans_dir = (_dir == 1)?RIGHT:LEFT;
    bool stop = (_dir == 0)?true:false;
    
    // Physics
    gravity(objs);
    jumpCollision(objs);
    collision(objs);
    min_dist_to_platform = checkDistToPlatform(objs);


    // Movement profile
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
        if (fall_speed_vertical > min_dist_to_platform.x){
            fall_speed_vertical = min_dist_to_platform.x;
        }
        funcToRun = &Player::falling;
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
    case BIG:
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
    if (state == JUMP || state == FALL){return;} // You cannot initiate a jump during a jump
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
    if (fall_speed_vertical >= 10){
        fall_speed_vertical = 10;
    }
    fall_cycles += 1;
}


void Player::endFall(){
    if (state == FALL){
        state = STAND;
        speed = 0;
    }
    
}




void Player::_moveX(int dx){
    pos.x += dx;
    xMin += dx;
    xMax += dx;
}
void Player::_moveY(int dy){
    pos.y += dy;
    yMin += dy;
    yMax += dy;
}



int Player::collisionGravity(Rectangle o1, Rectangle o2){
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


void Player::gravity(std::vector<Object*> objs){
    bool on_the_floor = false;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if (collisionGravity(o1, o2) != -1){
            endFall();
            on_the_floor = true;

            // Kill enemies by jumping on their head :)
            if (o->getType() == GOOMBA){  // TODO FIX THIS
                kill(o);
            }
            else if (o->getType() == KOOPA){
                if (!koopa_hit){
                    kill(o);
                    koopa_hit = true;
                }
                
            }

            // Just for Debug
            // o->selected = true;
        }
        else{
            // Just for Debug
            if (o->getType() == KOOPA){
                koopa_hit = false;
            }
            o->selected = false;
        }
    }

    if (!on_the_floor){
        if (getState() != FALL){
            startFall();
        }
        
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
        int o1_top = o1.y;
        int o1_bottom = o1.y + o1.h;
        int o2_top = o2.y;
        int o2_bottom = o2.y + o2.h;
        
        int o1_center = o1.x + o1.w/2.0;
        int o1_left = o1.x;
        int o1_right = o1.x + o1.w;
        int o2_left = o2.x;
        int o2_right = o2.x + o2.w;

        if ((o1_center >= o2_left && o1_center <= o2_right)){
            
            if (o1_top >= o2_bottom && abs(o1_top - o2_bottom) < 5){
                if (state == JUMP){
                    endJump();
                    collision_with_center = true;
                    collided = true;
                    // o->selected = true;
                    o->mark();
                    return;
                }
            }
        }
        else if (!collision_with_center){
            if (o1_left >= o2_left && o1_left <= o2_right){
                
                if (o1_top >= o2_bottom && abs(o1_top - o2_bottom) < 5){
                    std::cout << "left edge\n";
                    if (state == JUMP){
                        collided = true;
                        selected = o;
                    }
                }
            }
            else if (o1_right >= o2_left && o1_right <= o2_right){
                if (o1_top >= o2_bottom && abs(o1_top - o2_bottom) < 5){
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
        // selected->selected = true;
        selected->mark();
    }
    
}



void Player::collision(std::vector<Object*> objs){
    collidedL = false;
    collidedR = false;

    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if ((o1.right_center.y >= o2.y && o1.right_center.y <= (o2.y + o2.h)) ||
            (o1.y >= o2.y && o1.y <= (o2.y + o2.h)) ||
            ((o1.y + o1.h - 3) >= o2.y && (o1.y + o1.h - 3) <= (o2.y + o2.h))){
            if (abs(o1.right_center.x - o2.left_center.x) < 4){
                if (dir == RIGHT){
                    collidedR = true;
                    speed = 0;  
                    if (state == WALK || state == SLIDE){
                        state = STAND;
                    }
                    else if (state == JUMP){
                        speed = 0;
                    } 
                    
                }
            }
            else if (abs(o1.left_center.x - o2.right_center.x) < 4){
                if(dir == LEFT){
                    collidedL = true;
                    speed = 0;
                    if (state == WALK || state == SLIDE){
                        state = STAND;
                    } 
                    else if (state == JUMP){
                        speed = 0;
                    } 
                }
            }
        }
        
        
    }
}





Point Player::checkDistToLR(std::vector<Object*> objs){
    int min_dist_l = DBL_MAX;
    int min_dist_r = DBL_MAX;
    for (auto o:objs){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        double o1_center = o1.y + o1.h/2.0;
        int o2_top = o2.y;
        int o2_bottom = o2.y + o2.h;
        
        int o1_left = o1.x;
        int o1_right = o1.x + o1.w;
        int o2_left = o2.x;
        int o2_right = o2.x + o2.w;


        int dist_l = -1;
        int dist_r = -1;
        if (o1_center >= o2_top && o1_center <= o2_bottom){
            
            if (o1_right <= o2_left){
                dist_l = abs(o1_right - o2_left);
            }
            if (o1_left >= o2_right){
                dist_r = abs(o1_left - o2_right);
            }
        }
        
        if (dist_l != -1){
            if (dist_l < 100){
                o->selected = true;
            }
            else{
                o->selected = false; 
            }
            if (dist_l < min_dist_l){
                min_dist_l = dist_l;
            }
        }
        if (dist_r != -1){
            if (dist_r < 100){
                o->selected = true;
            }
            else{
                o->selected = false; 
            }
            if (dist_r < min_dist_r){
                min_dist_r = dist_r;
            }
        }

    }

    return Point(min_dist_l, min_dist_r);
}



void Player::kill(Object* obj){
    obj->death();
}

void Player::dead(){

}