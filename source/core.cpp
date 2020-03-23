#include "core.hpp"

Core::Core(){
    this->win = new Window(640, 480, "Mario");
    this->map = new Map("assets/maps/1/1.txt", win);

    FPS = 0;
}


void Core::loop(){
    while (true){
        frameTime = SDL_GetTicks();
        if(!this->events()){
            break;
        }
        this->update();
        this->draw();
        if(SDL_GetTicks() - frameTime < MIN_FRAME_RATE) {
            SDL_Delay(MIN_FRAME_RATE - (SDL_GetTicks () - frameTime));
        }
    }
}


void Core::update(){
    if(KEY_RIGHT_PRESSED){
        map->offset.x -= 10;
    }
    else if (KEY_LEFT_PRESSED){
        map->offset.x += 10;
    }
}




void Core::draw(){
    win->clear();
    map->drawObjects();

    this->showDebug();


    win->update_screen();
}

void Core::showDebug(){
    win->show_text("FPS: " + std::to_string(FPS), Point(10, 10), WHITE, "assets/Roboto-Regular.ttf", 10);

}









bool Core::events(){
    Event event = win->poll_for_event();
    std::cout << event.get_type() << std::endl;
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
                KEY_UP_PRESSED = true;
            }
            else if (event.get_pressed_key() == 'O'){
                KEY_RIGHT_PRESSED = true;
            }
            else if (event.get_pressed_key() == 'P'){
                KEY_LEFT_PRESSED = true;
            }
            break;
        case Event::KEY_RELEASE:

            if (event.get_pressed_key() == 'R'){
                KEY_UP_PRESSED = false;
            }
            else if (event.get_pressed_key() == 'O'){
                KEY_RIGHT_PRESSED = false;
            }
            else if (event.get_pressed_key() == 'P'){
                KEY_LEFT_PRESSED = false;
            }

        default:;
    }
    return true;
}