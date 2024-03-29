#include "core.hpp"

Core::Core(){
    this->win = new Window(640, 480, "Mario");
    this->world = new World();
    audio = new Audio(this->win);
    // this->win->play_music("./assets/sounds/Super Mario Bros. theme music.ogg");
    gameTimer = new Timer();
    endGameTimer = new Timer();
    gameTimer->start();
    FPS = 0;
}


void Core::loop(){
    int start = system_clock::now().time_since_epoch().count();
    
    while (true){
        int now = system_clock::now().time_since_epoch().count();
    
        FPS = 1000000000.0/(now - start);
        start = now;
        frameTime = SDL_GetTicks();

        // Key events
        if(!this->events()){break;}
        // Key handlers and camera movment, Player state updater
        this->update();
        // Update game state, Object state updater
        this->world->loop();
        // Handling sounds
        this->audio->update(world->getPlayer());
        // Draw all objects whitin camera field
        this->draw();


        if(SDL_GetTicks() - frameTime < MIN_FRAME_RATE) {
            SDL_Delay(MIN_FRAME_RATE - (SDL_GetTicks () - frameTime));
        }
    }
}


void Core::update(){
    int player_x = world->getPlayer()->getPos().x + world->camera->getPos().x;
    State state = world->getPlayer()->getState();
    if (state == DEAD){
        if (!endGameTimer->isStarted()){endGameTimer->start();}
        
        if (endGameTimer->getSecs() > 2){
            resetGame();
            return;
        }
        
    }
    Dir dir = world->getPlayer()->getDir();

    if(KEY_RIGHT_PRESSED){
        world->getPlayer()->update(1);
        if (player_x > 400){
            world->camera->move(-5);
            world->camera->moveBackground(-1);
        }
        
    }
    else if (KEY_LEFT_PRESSED){
        world->getPlayer()->update(-1);
        if (player_x < 200){
            world->camera->move(5);
            world->camera->moveBackground(1);
        }
        
    }
    else{
        world->getPlayer()->update(0);
    }

    // Move camera when mario is sliding
    if(state == SLIDE && dir == RIGHT){
        if (player_x > 400){
            world->camera->move(-5);
            world->camera->moveBackground(-1);
        }
    }
    else if(state == SLIDE && dir == LEFT){
        if (player_x < 200){
            world->camera->move(5);
            world->camera->moveBackground(1);
        }
    }
    if(KEY_UP_PRESSED){
        if(world->getPlayer()->can_jump){  
            world->getPlayer()->startJump();
            world->getPlayer()->can_jump = false;
        }
    }
    if (KEY_SHIFT_PRESSED){
        world->getPlayer()->shoot();
    }

}




void Core::draw(){
    win->clear();
    this->drawBackground();
    this->drawObjects();
    this->drawHood();
#if 0
   	this->showDebug();
#endif

    win->update_screen();
}

void Core::drawBackground(){
    win->draw_img(BACKGROUND,
                Rectangle(world->camera->getPosBackground(), Point(1600, 480) + world->camera->getPosBackground()),
                Rectangle(Point(0, 0), Point(2000, 1000)),
                0, false);
}

void Core::showDebug(){
    win->show_text("FPS: " + std::to_string(FPS), Point(10, 10), BLACK, "assets/Roboto-Regular.ttf", 20);
    win->show_text("State: " + std::string(ToString(world->getPlayer()->getState())),
     world->getPlayer()->getPos() + Point(15, -15) + world->camera->getPos(),
     WHITE, "assets/Roboto-Regular.ttf", 15);
     win->show_text("Speed: " + std::to_string(world->getPlayer()->getSpeed()),
     world->getPlayer()->getPos() + Point(15, -30) + world->camera->getPos(),
     WHITE, "assets/Roboto-Regular.ttf", 15);
    
    Point pos_player = world->getPlayer()->getPos() + world->camera->getPos();
    Point size_player = world->getPlayer()->getSize();
    win->draw_rect(Rectangle(pos_player, pos_player + world->getPlayer()->getSize()), WHITE, 1U);
    
    win->draw_line(Point(pos_player.x - 50, pos_player.y + size_player.y/2.0), 
                   Point(pos_player.x + size_player.x + 50, pos_player.y + size_player.y/2.0), RED);

    win->draw_line(Point(pos_player.x + size_player.x/2.0, pos_player.y - 50), 
                   Point(pos_player.x + size_player.x/2.0, pos_player.y + size_player.y + 50), RED);

    win->draw_line(Point(pos_player.x, pos_player.y - world->getPlayer()->min_dist_to_platform.y),
    Point(pos_player.x + size_player.x, pos_player.y - world->getPlayer()->min_dist_to_platform.y));
    for (auto o:world->getObjects()){
        if (o->selected){
            Point start_object = o->getPos() + world->camera->getPos();
            win->draw_rect(Rectangle(start_object, start_object + o->getSize()), BLUE, 4U);
        }
    }
}


void Core::drawHood(){
    win->show_text("Score: " + std::to_string(world->getGameState()->score),
        Point(10, 10), WHITE, "assets/Roboto-Regular.ttf", 20);

    win->show_text(std::to_string(gameTimer->getSecs()) + " s",
        Point(580, 10), WHITE, "assets/Roboto-Regular.ttf", 20);
}




void Core::drawObjects(){
    Point offset = world->camera->getPos();
    
    for (auto b:world->getGhosts()){
        if (b->getType() == G_TEXT){
            win->show_text(((Text*)b)->text, b->getPos() + offset, WHITE, "assets/Roboto-Regular.ttf", 12);
            continue;
        }
        win->draw_img(b->getImage(),
                 Rectangle(b->getPos() + offset, b->getPos() + b->getSize() + offset), NULL_RECT,
                 0, ((Coin*)b)->flipped);
    }

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
            if (event.get_pressed_key() == 'q'){
                return false;
            }
            else if (event.get_pressed_key() == 'R' || event.get_pressed_key() == ' '){
                world->getPlayer()->can_jump = true;
                KEY_UP_PRESSED = true;
            }
            else if (event.get_pressed_key() == 'O'){
                KEY_RIGHT_PRESSED = true;
            }
            else if (event.get_pressed_key() == 'P'){
                KEY_LEFT_PRESSED = true;
            }
            else if (event.get_pressed_key() == 'z' || event.get_pressed_key() == 'Z'){
                KEY_SHIFT_PRESSED = true;
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


void Core::resetGame(){
    world = new World();
    FPS = 0;
    gameTimer->reset();
    gameTimer->start();
    endGameTimer->reset();
    audio->reset();
}