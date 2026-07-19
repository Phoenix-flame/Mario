#include "core.hpp"
#include <unistd.h>
#include <sys/stat.h>

namespace
{
    bool directoryExists(const char *path)
    {
        struct stat info;
        return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
    }

    bool fileExists(const char *path)
    {
        struct stat info;
        return stat(path, &info) == 0 && S_ISREG(info.st_mode);
    }

    void configureResourceWorkingDirectory()
    {
        if (directoryExists("assets") && fileExists("assets/maps/1/1.txt"))
        {
            return;
        }

        if (directoryExists("../assets") && fileExists("../assets/maps/1/1.txt"))
        {
            chdir("..");
            return;
        }

        if (directoryExists("../../assets") && fileExists("../../assets/maps/1/1.txt"))
        {
            chdir("../..");
            return;
        }
    }
}

Core::Core(){
    configureResourceWorkingDirectory();
    this->win = new Window(640, 480, "Mario");
    this->world = new World(currentLevel);
    audio = new Audio(this->win);
    // this->win->play_music("./assets/sounds/Super Mario Bros. theme music.ogg");
    gameTimer = new Timer();
    endGameTimer = new Timer();
    shootTimer = new Timer();
    levelCompleteTimer = new Timer();
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

        if(!this->events()){break;}
        this->update();
        this->world->loop();
        this->audio->update(world->getPlayer());
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
            resetGame(currentLevel);
            return;
        }
    }
    if (state == WON){
        if (!levelCompleteTimer->isStarted()){levelCompleteTimer->start();}
        if (levelCompleteTimer->getSecs() > 3){
            if (currentLevel < TOTAL_LEVELS){
                currentLevel++;
                resetGame(currentLevel);
            }
            else{
                currentLevel = 1;
                resetGame(currentLevel);
            }
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
    if (debugEnabled){
        this->showDebug();
    }
    win->update_screen();
}

void Core::drawBackground(){
    const RGB SKY_BLUE(92, 148, 252);
    const RGB CLOUD_TRANSPARENT(255, 0, 255);
    win->fill_rect(Rectangle(Point(0, 0), Point(640, 480)), SKY_BLUE);

    static int cloud_offset = 0;
    static int last_cloud_tick = SDL_GetTicks();
    int current_tick = SDL_GetTicks();
    if (current_tick - last_cloud_tick > 45)
    {
        cloud_offset = (cloud_offset + 1) % 760;
        last_cloud_tick = current_tick;
    }

    auto draw_cloud_tile = [this, CLOUD_TRANSPARENT](std::string filename, Rectangle dst) {
        win->draw_img_with_color_key(filename, CLOUD_TRANSPARENT, dst, NULL_RECT, 0, false);
    };

    auto draw_cloud = [this, draw_cloud_tile](int x, int y, int tile_size) {
        const std::string path = "assets/sprites/objects/cloud/";
        draw_cloud_tile(path + "cloud_left_top.bmp", Rectangle(Point(x, y), Point(x + tile_size, y + tile_size)));
        draw_cloud_tile(path + "cloud_center_top.bmp", Rectangle(Point(x + tile_size, y), Point(x + tile_size * 2, y + tile_size)));
        draw_cloud_tile(path + "cloud_right_top.bmp", Rectangle(Point(x + tile_size * 2, y), Point(x + tile_size * 3, y + tile_size)));
        draw_cloud_tile(path + "cloud_left_bot.bmp", Rectangle(Point(x, y + tile_size), Point(x + tile_size, y + tile_size * 2)));
        draw_cloud_tile(path + "cloud_center_bot.bmp", Rectangle(Point(x + tile_size, y + tile_size), Point(x + tile_size * 2, y + tile_size * 2)));
        draw_cloud_tile(path + "cloud_right_bot.bmp", Rectangle(Point(x + tile_size * 2, y + tile_size), Point(x + tile_size * 3, y + tile_size * 2)));
    };

    const int period = 760;
    int parallax = world->camera->getPosBackground().x / 2;
    int bases[5] = {80, 230, 390, 560, 720};
    int ys[5] = {70, 120, 55, 145, 95};
    int tile_sizes[5] = {20, 16, 22, 18, 16};

    for (int i = 0; i < 5; i++)
    {
        int x = (bases[i] + parallax - cloud_offset) % period;
        while (x < -100){ x += period; }
        while (x > 680){ x -= period; }
        draw_cloud(x, ys[i], tile_sizes[i]);
    }
}

void Core::showDebug(){
    const std::string font = "assets/Roboto-Regular.ttf";
    const Point camera = world->camera->getPos();
    Player *player = world->getPlayer();
    Point player_pos = player->getPos();
    Point player_size = player->getSize();
    Point screen_player_pos = player_pos + camera;

    win->fill_rect(Rectangle(Point(0, 0), Point(270, 122)), BLACK);
    win->show_text("DEBUG: ON (D to toggle)", Point(8, 6), YELLOW, font, 14);
    win->show_text("FPS: " + std::to_string((int)FPS), Point(8, 24), WHITE, font, 13);
    win->show_text("Player: " + std::string(ToString(player->getState())) + " / " + std::string(ToString(player->getDir())), Point(8, 42), WHITE, font, 13);
    win->show_text("Speed: " + std::to_string(player->getSpeed()), Point(8, 60), WHITE, font, 13);
    win->show_text("World: (" + std::to_string(player_pos.x) + ", " + std::to_string(player_pos.y) + ")", Point(8, 78), WHITE, font, 13);
    win->show_text("Camera: (" + std::to_string(camera.x) + ", " + std::to_string(camera.y) + ")", Point(8, 96), WHITE, font, 13);

    win->fill_rect(Rectangle(Point(430, 0), Point(640, 104)), BLACK);
    win->show_text("Objects: " + std::to_string(world->getObjects().size()), Point(438, 8), CYAN, font, 13);
    win->show_text("Bodies: " + std::to_string(world->getCollisionBodies().size()), Point(438, 26), CYAN, font, 13);
    win->show_text("Ghosts: " + std::to_string(world->getGhosts().size()), Point(438, 44), CYAN, font, 13);
    win->show_text("Score: " + std::to_string(world->getGameState()->score), Point(438, 62), CYAN, font, 13);
    win->show_text("Time: " + std::to_string(gameTimer->getSecs()) + "s", Point(438, 80), CYAN, font, 13);

    Rectangle player_rect(screen_player_pos, screen_player_pos + player_size);
    win->draw_rect(player_rect, player->isInvincible() ? CYAN : YELLOW, 2U);
    win->draw_line(Point(player_rect.left_center.x - 40, player_rect.left_center.y), Point(player_rect.right_center.x + 40, player_rect.right_center.y), RED);
    win->draw_line(Point(player_rect.top_center.x, player_rect.top_center.y - 40), Point(player_rect.bottom_center.x, player_rect.bottom_center.y + 40), RED);
    win->draw_line(Point(player_rect.x, player_rect.y - player->min_dist_to_platform.y), Point(player_rect.x + player_rect.w, player_rect.y - player->min_dist_to_platform.y), GREEN);
    win->show_text(player->isInvincible() ? "PLAYER INVINCIBLE" : "PLAYER", Point(player_rect.x, player_rect.y - 18), player->isInvincible() ? CYAN : YELLOW, font, 11);

    win->draw_rect(Rectangle(Point(0, 0), Point(640, 480)), MAGENTA, 1U);
    win->draw_line(Point(0, 240), Point(640, 240), MAGENTA);
    win->draw_line(Point(320, 0), Point(320, 480), MAGENTA);

    int visible_body_count = 0;
    for (auto body : world->getCollisionBodies()){
        Point p(body.bounds.x, body.bounds.y);
        p += camera;
        Point s(body.bounds.w, body.bounds.h);
        if (p.x + s.x < 0 || p.x > 640 || p.y + s.y < 0 || p.y > 480){
            continue;
        }

        Rectangle bounds(p, p + s);

        auto horizontalOverlap = [](const Rectangle &a, const Rectangle &b) {
            return a.x < b.x + b.w && a.x + a.w > b.x;
        };
        auto verticalOverlap = [](const Rectangle &a, const Rectangle &b) {
            return a.y < b.y + b.h && a.y + a.h > b.y;
        };

        for (unsigned int i = 0; i < body.parts.size(); i++){
            Rectangle part = body.parts[i].rect;
            bool has_left_neighbor = false;
            bool has_right_neighbor = false;
            bool has_top_neighbor = false;
            bool has_bottom_neighbor = false;

            for (unsigned int j = 0; j < body.parts.size(); j++){
                if (i == j){ continue; }

                Rectangle other = body.parts[j].rect;
                if (other.x + other.w == part.x && verticalOverlap(part, other)){
                    has_left_neighbor = true;
                }
                if (part.x + part.w == other.x && verticalOverlap(part, other)){
                    has_right_neighbor = true;
                }
                if (other.y + other.h == part.y && horizontalOverlap(part, other)){
                    has_top_neighbor = true;
                }
                if (part.y + part.h == other.y && horizontalOverlap(part, other)){
                    has_bottom_neighbor = true;
                }
            }

            Point top_left(part.x + camera.x, part.y + camera.y);
            Point top_right(part.x + part.w + camera.x, part.y + camera.y);
            Point bottom_left(part.x + camera.x, part.y + part.h + camera.y);
            Point bottom_right(part.x + part.w + camera.x, part.y + part.h + camera.y);

            if (!has_top_neighbor){
                win->draw_line(top_left, top_right, GREEN);
            }
            if (!has_bottom_neighbor){
                win->draw_line(bottom_left, bottom_right, GREEN);
            }
            if (!has_left_neighbor){
                win->draw_line(top_left, bottom_left, GREEN);
            }
            if (!has_right_neighbor){
                win->draw_line(top_right, bottom_right, GREEN);
            }
        }

        // Thin outer rectangle shows the broad-phase bounds. Green lines are the exact non-convex boundary.
        win->draw_rect(bounds, BLUE, 1U);

        std::string label = std::string("BODY ") + ToString(body.type) + " x" + std::to_string(body.parts.size());
        win->show_text(label, Point(bounds.x, bounds.y - 10), GREEN, font, 9);
        visible_body_count++;
    }

    int visible_count = 0;
    for (auto o : world->getObjects()){
        if (o == player){ continue; }
        if (o->getType() == BLOCK || o->getType() == BRICK || o->getType() == GROUND || o->getType() == PIPE ||
            o->getType() == COIN_CONTAINER || o->getType() == FIRE_CONTAINER || o->getType() == HEALTH_CONTAINER){
            continue;
        }

        Point p = o->getPos() + camera;
        Point s = o->getSize();
        if (p.x + s.x < 0 || p.x > 640 || p.y + s.y < 0 || p.y > 480){ continue; }

        Rectangle rect(p, p + s);
        RGB color = (o->getType() == GOOMBA || o->getType() == KOOPA) ? MAGENTA : (o->dead ? RED : BLUE);
        win->draw_rect(rect, color, 1U);
        win->draw_point(rect.top_center, RED);
        win->draw_point(rect.bottom_center, CYAN);
        win->draw_point(rect.left_center, YELLOW);
        win->draw_point(rect.right_center, YELLOW);

        if (visible_count < 35){
            std::string label = std::string(ToString(o->getType()));
            if (o->dead){ label += " DEAD"; }
            win->show_text(label, Point(rect.x, rect.y - 10), color, font, 9);
        }
        visible_count++;
    }

    for (auto g : world->getGhosts()){
        Point p = g->getPos() + camera;
        Point s = g->getSize();
        if (p.x + s.x < 0 || p.x > 640 || p.y + s.y < 0 || p.y > 480){ continue; }

        Rectangle rect(p, p + s);
        win->draw_rect(rect, CYAN, 1U);
        win->show_text(std::string(ToString(g->getType())), Point(rect.x, rect.y - 10), CYAN, font, 9);
    }

    win->show_text("visible bodies: " + std::to_string(visible_body_count), Point(438, 118), YELLOW, font, 12);
    win->show_text("visible dynamic: " + std::to_string(visible_count), Point(438, 134), YELLOW, font, 12);
}


void Core::drawHood(){
    win->show_text("Level " + std::to_string(currentLevel), Point(10, 10), WHITE, "assets/Roboto-Regular.ttf", 20);
    win->show_text("Score: " + std::to_string(world->getGameState()->score), Point(200, 10), WHITE, "assets/Roboto-Regular.ttf", 20);
    win->show_text(std::to_string(gameTimer->getSecs()) + " s", Point(580, 10), WHITE, "assets/Roboto-Regular.ttf", 20);
}

void Core::drawObjects(){
    Point offset = world->camera->getPos();
    
    for (auto b:world->getGhosts()){
        if (b->getType() == G_TEXT){
            win->show_text(((Text*)b)->text, b->getPos() + offset, WHITE, "assets/Roboto-Regular.ttf", 12);
            continue;
        }
        win->draw_img(b->getImage(), Rectangle(b->getPos() + offset, b->getPos() + b->getSize() + offset), NULL_RECT, 0, ((Coin*)b)->flipped);
    }

    for (auto b:world->getObjects()){
        if (b->getType() == PLAYER && !((Player*)b)->shouldDraw()){
            continue;
        }
        win->draw_img(b->getImage(), Rectangle(b->getPos() + offset, b->getPos() + b->getSize() + offset), NULL_RECT, 0, false);
    }

    Player * player = world->getPlayer();
    if (player->shouldDraw()){
        win->draw_img(player->getImage(), Rectangle(player->getPos() + offset, player->getPos() + player->getSize() + offset), NULL_RECT, 0, false);
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
            if (event.get_pressed_key() == 'q'){
                return false;
            }
            else if (event.get_pressed_key() == 'd' || event.get_pressed_key() == 'D'){
                debugEnabled = !debugEnabled;
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

void Core::resetGame(int level){
    world = new World(level);
    currentLevel = level;
    FPS = 0;
    gameTimer->reset();
    gameTimer->start();
    endGameTimer->reset();
    shootTimer->reset();
    levelCompleteTimer->reset();
    audio->reset();
}
