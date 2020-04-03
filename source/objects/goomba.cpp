#include "goomba.hpp"



Goomba::Goomba(int x, int y):Object(Point(x, y), 
        Point(24, 24), 
        GOOMBA_WALK1_IMAGE, 
        GOOMBA){

    state = GOOMBA_WALK_STATE;
    speed = -1;
    dir = LEFT;
    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(1,2);
    walk_state = distribution(generator);
}


void Goomba::update(std::vector<Object*> objs, Object* player){
    if(!visited){return;}

    if(state == GOOMBA_DEAD_STATE){
        funcToRun = &Goomba::death_animation;
    }
    else{
        // Physics
        gravity(objs, player);
        collision(objs, player);
        min_dist_to_platform = checkDistToPlatform(objs, player);

        funcToRun = &Goomba::normal_behavior;
    }


    // Update Player State
    (this->*funcToRun)();
    updateFigure();
}

void Goomba::normal_behavior(){
    _moveX(speed);

    if (state == GOOMBA_FALL_STATE){
        if (fall_speed_vertical > min_dist_to_platform){
            fall_speed_vertical = min_dist_to_platform;
        }
        falling();
    }
}

void Goomba::updateFigure(){
    if (state == GOOMBA_WALK_STATE){
        if (walk_state == 1){
            image = GOOMBA_WALK1_IMAGE;
            if (!walkTimer.isStarted()){walkTimer.start();}
            if (walkTimer.getTime() > 200){
                walk_state = 2;
                walkTimer.reset();
            }
        }
        else if (walk_state == 2){
            image = GOOMBA_WALK2_IMAGE;
            if (!walkTimer.isStarted()){walkTimer.start();}
            if (walkTimer.getTime() > 200){
                walk_state = 1;
                walkTimer.reset();
            }
        }
    }
    else if (state == GOOMBA_DEAD_STATE){
        image = GOOMBA_DEAD_IMAGE;
    }
}


void Goomba::gravity(std::vector<Object*> objs, Object* player){
    std::vector<Object*> objects = objs;
    objects.push_back(player);
    bool on_the_floor = false;
    for (auto o:objects){
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());
        if (collisionGravity(o1, o2) != -1){
            if (state != GOOMBA_DEAD_STATE) endFall();
            on_the_floor = true;
            if(o->getType() == PLAYER){
                std::cout << "Jump to kill :)\n";
                o->death();
            }
        }
        else{
        }
    }

    if (!on_the_floor){
        if (state != GOOMBA_FALL_STATE){
            startFall();
        }
        
    }
}


int Goomba::checkDistToPlatform(std::vector<Object*> objs, Object* player){
    std::vector<Object*> objects = objs;
    objects.push_back(player);
    int min_dist_b = DBL_MAX;
    for (auto o:objects){
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


int Goomba::collisionGravity(Rectangle o1, Rectangle o2){
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



void Goomba::startFall(){
    state = GOOMBA_FALL_STATE;
    fall_speed_vertical = 3;
}

void Goomba::falling(){
    _moveY(fall_speed_vertical);
    fall_speed_vertical += (fall_cycles%2 == 0)?1:0;
    if (fall_speed_vertical >= 10){
        fall_speed_vertical = 10;
    }
    fall_cycles += 1;
    _moveX(0);
}


void Goomba::endFall(){
    if (state == GOOMBA_FALL_STATE){
        state = GOOMBA_WALK_STATE;
    }    
}


void Goomba::collision(std::vector<Object*> objs, Object* player){
    std::vector<Object*> objects = objs;
    objects.push_back(player);
    for (auto o:objects){
        
        Rectangle o1(getPos(), getPos() + getSize());
        Rectangle o2(o->getPos(), o->getPos() + o->getSize());


        if ((o1.right_center.y >= o2.y && o1.right_center.y <= (o2.y + o2.h)) ||
            (o1.y >= o2.y && o1.y <= (o2.y + o2.h)) ||
            ((o1.y + o1.h - 3) >= o2.y && (o1.y + o1.h - 3) <= (o2.y + o2.h))){
            if (abs(o1.right_center.x - o2.left_center.x) < 4){
                if (state == GOOMBA_WALK_STATE){
                    if (dir == RIGHT){
                        if (o->getType() == PLAYER){
                            o->death();
                        }
                        else{
                            dir = LEFT;
                            speed *= -1;
                        }
                    } 
                }
            }
            else if (abs(o1.left_center.x - o2.right_center.x) < 4){
                if (state == GOOMBA_WALK_STATE){
                    if (dir == LEFT){
                        if (o->getType() == PLAYER){
                            o->death();
                        }
                        else{
                            dir = RIGHT;
                            speed *= -1;
                        }
                    } 
                }
            }
        } 
    }
}



void Goomba::death_animation(){
    if (!deathAnimation.isStarted()){
        deathAnimation.start();
    }
    if (deathAnimation.getTime() < 100){
        fall_speed_vertical = 3;
        _moveY(-5);
    }
    else if (deathAnimation.getTime() >= 200){
        falling();
    }
    updateFigure();
}



void Goomba::death(){
    // Very simple death
    state = GOOMBA_DEAD_STATE;
}
