#include "player.hpp"




void Player::update(int dir){
    if (state == STAND){

    }
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