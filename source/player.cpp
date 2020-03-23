#include "player.hpp"




void Player::update(std::vector<Object*> objs, int _dir){
    // std::cout << ToString(state) << " " << _dir << std::endl;
    Dir trans_dir = (_dir == 1)?RIGHT:LEFT;
    
    gravity(objs);
    jumpCollision(objs);
    collision(objs);
    
    Point min_dist_to_platform = checkDistToPlatform(objs);
    // std::cout << "Min Dist to Platform: " << min_dist_to_platform.x << std::endl;
    // std::cout << "Min Dist to TopBlock: " << min_dist_to_platform.y << std::endl;

    if (state == STAND && (_dir != 0) ){
        this->dir = (trans_dir == -1)?LEFT:RIGHT;
        startMove();
    }
    else if (state == WALK && _dir != 0){
        this->dir = (_dir == -1)?LEFT:RIGHT;
        move();
    }
    else if ((state == WALK || state == SLIDE) && _dir == 0){
        endMove();
    }
    else if (state == SLIDE){
        startMove();
    }
    else if (state == FALL){
        if (_dir == 0){ trans_dir = STOP;}
        else {
            dir = trans_dir;
        }
        if (fall_speed_vertical > min_dist_to_platform.x){
            fall_speed_vertical = min_dist_to_platform.x;
        }
        falling(trans_dir, (_dir == 0)?true:false);

    }
    else if(state == JUMP){
        if (jump_speed_vertical > min_dist_to_platform.y){
            jump_speed_vertical = min_dist_to_platform.y;
        }
        jump();
        
    }



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
                std::cout << SDL_GetTicks() << std::endl;
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
void Player::move(){
    if (dir == LEFT){
        _moveX(-5);
    }
    else if (dir == RIGHT){
        _moveX(+5);
    }
    slide_enable += 1;
    // std::cout << slide_enable << std::endl;
    
}
void Player::endMove(){

    if (slide_enable < 10){
        state = STAND;
        return;
    }
    else if (state == WALK){
        slideTimer.reset();
        slideTimer.start();
        state = SLIDE;
        
    }

    if (slideTimer.getTime() > 300){
        state = STAND;
        slide_enable = 0;
    }
    else{
        if (dir == LEFT){
            _moveX(-5);
        }
        else if (dir == RIGHT){
            _moveX(+5);
        }   
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
        if (move_during_jump){
            if (dir == LEFT){
                _moveX(-5);
            }
            else if (dir == RIGHT){
                _moveX(+5);
            } 
        }
    }
    else {
        state = FALL;
        fall_speed_vertical = -jump_speed_vertical;
    }
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

void Player::falling(Dir _dir, bool stop_horizontal_move){
    static int fall_cycles = 0;
    _moveY(fall_speed_vertical);
    fall_speed_vertical += (fall_cycles%2 == 0)?1:0;
    if (fall_speed_vertical >= 10){
        fall_speed_vertical = 10;
    }
    fall_cycles += 1;

    if (stop_horizontal_move){return;}
    if (_dir == LEFT){
        _moveX(-5);
    }
    else if (_dir == RIGHT){
        _moveX(+5);
    }  
    
}


void Player::endFall(){
    if (state == FALL){
        state = STAND;
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
            o->selected = true;
        }
        else{
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
    bool collided = false;
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

        if ((o1_right >= o2_left && o1_left <= o2_right) ||
            (o1_left >= o2_left && o1_left <= o2_right)){
            if (o1_top >= o2_bottom && abs(o1_top - o2_bottom) < 10){
                if (state == JUMP){
                    endJump();
                    collided = true;
                    selected = o;
                }
            }
        }
    }
    selected->selected = true;
    selected->mark();
}



void Player::collision(std::vector<Object*> objs){
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

        if (o1_bottom > o2_top && o1_bottom < o2_bottom){
            if (o1_right <= o2_left && abs(o1_right - o2_left) < 10){
                if (state == WALK || state == SLIDE){
                    std::cout << "LEFT HORIZONTAL COLLISION\n";
                    endMove();
                    o->selected = true;
                }
            }
        }
    }
}