#include "core.hpp"

Core::Core(){
    this->win = new Window(640, 480, "Mario");
    this->world = new World();
    audio = new Audio(this->win);
    // this->win->play_music("./assets/sounds/Super Mario Bros. theme music.ogg");
    gameTimer = new Timer();
    endGameTimer = new Timer();
    shootTimer = new Timer();
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
        Player *player = world->getPlayer();
        if (player->getLevel() == POWER && (!shootTimer->isStarted() || shootTimer->getTime() > 250)){
            Point pos = player->getPos();
            Point size = player->getSize();
            Dir fireDir = (player->getDir() == LEFT) ? LEFT : RIGHT;
            int bullet_x = (fireDir == RIGHT) ? pos.x + size.x : pos.x - 16;
            int bullet_y = pos.y + (size.y / 2) - 8;
            world->addGhost(new Bullet(bullet_x, bullet_y, fireDir));
            player->shoot();
            shootTimer->reset();
            shootTimer->start();
        }
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
    const RGB SKY_BLUE(92, 148, 252);
    win->fill_rect(Rectangle(Point(0, 0), Point(640, 480)), SKY_BLUE);

    static int cloud_offset = 0;
    static int last_cloud_tick = SDL_GetTicks();
    int current_tick = SDL_GetTicks();
    if (current_tick - last_cloud_tick > 45)
    {
        cloud_offset = (cloud_offset + 1) % 760;
        last_cloud_tick = current_tick;
    }

    auto draw_cloud = [this](int x, int y, int tile_size) {
        const std::string path = "assets/sprites/objects/cloud/";
        win->draw_img(path + "cloud_left_top.bmp", Rectangle(Point(x, y), Point(x + tile_size, y + tile_size)), NULL_RECT, 0, false);
        win->draw_img(path + "cloud_center_top.bmp", Rectangle(Point(x + tile_size, y), Point(x + tile_size * 2, y + tile_size)), NULL_RECT, 0, false);
        win->draw_img(path + "cloud_right_top.bmp", Rectangle(Point(x + tile_size * 2, y), Point(x + tile_size * 3, y + tile_size)), NULL_RECT, 0, false);
        win->draw_img(path + "cloud_left_bot.bmp", Rectangle(Point(x, y + tile_size), Point(x + tile_size, y + tile_size * 2)), NULL_RECT, 0, false);
        win->draw_img(path + "cloud_center_bot.bmp", Rectangle(Point(x + tile_size, y + tile_size), Point(x + tile_size * 2, y + tile_size * 2)), NULL_RECT, 0, false);
        win->draw_img(path + "cloud_right_bot.bmp", Rectangle(Point(x + tile_size * 2, y + tile_size), Point(x + tile_size * 3, y + tile_size * 2)), NULL_RECT, 0, false);
    };

    const int period = 760;
    int parallax = world->camera->getPosBackground().x / 2;
    int bases[5] = {80, 230, 390, 560, 720};
    int ys[5] = {70, 120, 55, 145, 95};
    int tile_sizes[5] = {20, 16, 22, 18, 16};

    for (int i = 0; i < 5; i++)
    {
        int x = (bases[i] + parallax - cloud_offset) % period;
        while (x < -100)
        {
            x += period;
        }
        while (x > 680)
        {
            x -= period;
        }
        draw_cloud(x, ys[i], tile_sizes[i]);
    }
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

            if (event.get_pressed_key() == 'R' || event.get_pressed_key() == ' '){
                KEY_UP_PRESSED = false;
            }
            else if (event.get_pressed_key() == 'O'){
                KEY_RIGHT_PRESSED = false;
            }
            else if (event.get_pressed_key() == 'P'){
                KEY_LEFT_PRESSED = false;
            }
            else if (event.get_pressed_key() == 'z' || event.get_pressed_key() == 'Z'){
                KEY_SHIFT_PRESSED = false;
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
    shootTimer->reset();
    audio->reset();
}