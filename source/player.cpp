#include "player.hpp"




void Player::update(int _dir){
    std::cout << ToString(state) << " " << _dir << std::endl;
    Dir trans_dir = (_dir == 1)?RIGHT:LEFT;
   
    
    
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
    else if (state == SLIDE && trans_dir != dir){
        startMove();
    }
    else if (state == FALL){
        if (_dir == 0){ trans_dir = STOP;}
        else {
            dir = trans_dir;
        }
        falling(trans_dir, (_dir == 0)?true:false);
    }
    else if(state == JUMP){
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
                if (walk_left == 1){ image = NORM_WALK_LEFT1; walk_left = 2;}
                else if (walk_left == 2){image = NORM_WALK_LEFT2; walk_left = 3;}
                else if (walk_left == 3){image = NORM_WALK_LEFT3; walk_left = 1;}
            }
            else if (dir == RIGHT){
                walk_left = 1;
                if (walk_right == 1){ image = NORM_WALK_RIGHT1; walk_right = 2;}
                else if (walk_right == 2){image = NORM_WALK_RIGHT2; walk_right = 3;}
                else if (walk_right == 3){image = NORM_WALK_RIGHT3; walk_right = 1;}
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
            // if (dir!= STOP){
                image = (dir == LEFT)?NORM_JUMP_LEFT:NORM_JUMP_RIGHT;
            // }
            
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
    std::cout << slide_enable << std::endl;
    
}
void Player::endMove(){
    static int start_slide = 0;
    if (slide_enable < 10){
        state = STAND;
        return;
    }
    else if (state == WALK){
        start_slide = SDL_GetTicks();
        state = SLIDE;
        
    }
    std::cout << SDL_GetTicks() - start_slide << std::endl;
    if (SDL_GetTicks() - start_slide >240){
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
    jump_timer = SDL_GetTicks();
}
void Player::jump(){
    static int jump_cycles = 0;
    
    if (SDL_GetTicks() - jump_timer < 1000){
        
        _moveY(jump_speed_vertical);
        jump_speed_vertical += (jump_cycles%2 == 0)?1:0;
        jump_cycles += 1;
        std::cout << "jump_speed_vertical: " << jump_speed_vertical << std::endl; 
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
        
    }
}
void Player::endJump(){
    state = FALL;
}

void Player::startFall(){
    if (state != JUMP){
        state = FALL;
    }
    
}

void Player::falling(Dir _dir, bool stop_horizontal_move){
    state = FALL;
    _moveY(+5);
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