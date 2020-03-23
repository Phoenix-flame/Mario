#include "core.hpp"

Core::Core(){
    this->win = new Window(640, 480, "Mario");
    this->map = new Map("assets/maps/1/1.txt", win);
}


void Core::loop(){
    while (true){
        frameTime = SDL_GetTicks();
        if(!this->events()){
            break;
        }
        this->draw();

        if(SDL_GetTicks() - frameTime < MIN_FRAME_RATE) {
            SDL_Delay(MIN_FRAME_RATE - (SDL_GetTicks () - frameTime));
        }
    }
}

bool Core::events(){
    Event event = win->poll_for_event();
    switch (event.get_type()) {
        case Event::QUIT:
            return false;
        case Event::LCLICK:
            
            break;
        case Event::KEY_PRESS:
            std::cout << event.get_pressed_key() << std::endl;
            if (event.get_pressed_key() == 'q'){
                return false;
            }
            else if (event.get_pressed_key() == 'R'){

            }
            else if (event.get_pressed_key() == 'O'){
                map->offset.x -= 10;
            }
            else if (event.get_pressed_key() == 'P'){
                map->offset.x += 10;
            }
        default:;
    }
    return true;
}


void Core::draw(){
    win->clear();
    map->drawObjects();

    win->update_screen();
}