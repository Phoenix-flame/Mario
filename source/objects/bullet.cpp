#include "bullet.hpp"




// Collision Notification
void Bullet::notifyCollisionLeft(Object* obj){
    // Type t = obj->getType();
    // if(t== KOOPA || t == GOOMBA){
    //     return;
    // }
    // if (t == PLAYER){
    //     ((Player*)obj)->powerup();
    //     ghost_dead = true;
    //     return;
    // }
    // speed *= -1;
    
}
void Bullet::notifyCollisionRight(Object* obj){
    // Type t = obj->getType();
    // if(t== KOOPA || t == GOOMBA){
    //     return;
    // }
    // if (t == PLAYER){
    //     ((Player*)obj)->powerup();
    //     ghost_dead = true;
    //     return;
    // }
    // speed *= -1;
    
}
void Bullet::notifyCollisionTop(Object* obj){
    // Type t = obj->getType();
    // if (t == PLAYER){
    //     ((Player*)obj)->powerup();
    //     ghost_dead = true;
    //     return;
    // }
}
void Bullet::notifyCollisionBottom(Object* obj){
    // Type t = obj->getType();
    // if (t == PLAYER){
    //     ((Player*)obj)->powerup();
    //     ghost_dead = true;
    //     return;
    // }
    // endFall();
}

void Bullet::notifyFreeLeft(){}
void Bullet::notifyFreeRight(){}
void Bullet::notifyFreeTop(){}
void Bullet::notifyFreeBottom(){
    // if (!gravity_en){return;}
    // if (state != M_FALL){
    //     startFall();
    // }
}

void Bullet::notifyDistToPlatform(int d){
    // if (fall_speed_vertical > d){
    //     fall_speed_vertical = d;
    // }
}

void Bullet::notifyDistToCeil(int d){

}