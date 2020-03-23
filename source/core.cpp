#include "core.hpp"

Core::Core(){
    this->win = new Window(640, 480, "Mario");
    this->world = new World();

    FPS = 0;
}


void Core::loop(){
    while (true){
        frameTime = SDL_GetTicks();
        if(!this->events()){
            break;
        }
        
        this->update();
        this->world->loop();
        this->draw();
        if(SDL_GetTicks() - frameTime < MIN_FRAME_RATE) {
            SDL_Delay(MIN_FRAME_RATE - (SDL_GetTicks () - frameTime));
        }
    }
}


void Core::update(){
    int player_x = world->getPlayer()->getPos().x + world->camera->getPos().x;
    if(KEY_RIGHT_PRESSED){
        world->getPlayer()->update(world->getObjects(), 1);
        if (player_x > 400){
            world->camera->move(-5);
        }
        
    }
    else if (KEY_LEFT_PRESSED){
        world->getPlayer()->update(world->getObjects(), -1);
        if (player_x < 100){
            world->camera->move(5);
        }
        
    }
    else{
        world->getPlayer()->update(world->getObjects(), 0);
    }
    if(KEY_UP_PRESSED){
        if(world->getPlayer()->can_jump){
            world->getPlayer()->startJump();
            world->getPlayer()->can_jump = false;
        }
        
    }

}




void Core::draw(){
    win->clear();
    drawObjects();

    this->showDebug();


    win->update_screen();
}

void Core::showDebug(){
    win->show_text("FPS: " + std::to_string(FPS), Point(10, 10), WHITE, "assets/Roboto-Regular.ttf", 10);
    win->show_text("State: " + std::string(ToString(world->getPlayer()->getState())),
     world->getPlayer()->getPos() + Point(15, -15) + world->camera->getPos(),
     WHITE, "assets/Roboto-Regular.ttf", 15);
    Point start_player = world->getPlayer()->getPos() + world->camera->getPos();
    win->draw_rect(Rectangle(start_player, start_player + world->getPlayer()->getSize()), WHITE, 1U);


    for (auto o:world->getObjects()){
        if (o->selected){
            Point start_object = o->getPos() + world->camera->getPos();
            win->draw_rect(Rectangle(start_object, start_object + o->getSize()), BLUE, 1U);
        }
        
    }
}




void Core::drawObjects(){
    Point offset = world->camera->getPos();
    for (auto b:world->getObjects()){
        win->draw_img(b->getImage(),
                 Rectangle(b->getPos() + offset, b->getPos() + b->getSize() + offset), NULL_RECT,
                 0, false);
    }
    Player * player = world->getPlayer();
    win->draw_img(player->getImage(),
                 Rectangle(player->getPos() + offset, player->getPos() + player->getSize() + offset), NULL_RECT,
                 0, false);
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
                world->getPlayer()->can_jump = true;
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