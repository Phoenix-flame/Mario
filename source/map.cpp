#include "map.hpp"


Map::Map(std::string filepath, Window* win){
    this->offset.x = 0;
    this->offset.y = 0;
    
    this->win = win;
    std::ifstream map_file;
    map_file.open(filepath);
    std::string line;
    int y = 0;
    bool flag_pipes = false;
    std::vector<int> pipes_head;

    while (std::getline(map_file, line)){
        for(unsigned int x = 0 ; x < line.size() ; x ++){
            if (line[x] == '@'){
                Block tmp(x, y);
                this->blocks.push_back(tmp);
            }
            else if(line[x] == '#'){
                Ground tmp(x, y);
                this->grounds.push_back(tmp);
            }
            else if(line[x] == 'b'){
                Brick tmp(x, y);
                this->bricks.push_back(tmp);
            }
            else if(line[x] == '?'){
                CoinContainer tmp(x, y);
                this->coins.push_back(tmp);
            }
            else if(line[x] == 'm'){
                FireContainer tmp(x, y);
                this->fires.push_back(tmp);
            }
            else if(line[x] == 'h'){
                HealthContainer tmp(x, y);
                this->healths.push_back(tmp);
            }
            else if(line[x] == 'l'){
                Goomba tmp(x, y);
                this->goombas.push_back(tmp);
            }
            else if(line[x] == 'k'){
                Koopa tmp(x, y);
                this->koopas.push_back(tmp);
            }
            else if(line[x] == 'M'){
                player = new Player(x, y);
            }
            else if(line[x] == '|'){
                bool exist = false;
                for (auto p:pipes_head){
                    if(p == x){
                        exist = true;
                        if (flag_pipes){ // Right
                            Pipe tmp(x, y, "r");
                            pipes.push_back(tmp);
                            flag_pipes = false;
                        }
                        else{ // Left
                            Pipe tmp(x, y, "l");
                            pipes.push_back(tmp);
                            flag_pipes = true;
                        }
                    }
                    continue;
                }
                if(!exist){
                    if (flag_pipes){ // Right
                        Pipe tmp(x, y, "hr");
                        pipes.push_back(tmp);
                        flag_pipes = false;
                    }
                    else{ // Left
                        Pipe tmp(x, y, "hl");
                        pipes.push_back(tmp);
                        flag_pipes = true;
                    }
                    pipes_head.push_back(x);
                }
            }
        }
        y += 1;
    }

    map_file.close();
}

void Map::drawObjects(){
    for (auto b:blocks){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto g:grounds){
        win->draw_img(g.getImage(),
                 Rectangle(g.getPos() + offset, g.getPos() + g.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto b:bricks){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }

    for (auto b:coins){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto b:fires){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto b:healths){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto b:goombas){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto b:koopas){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    for (auto b:pipes){
        win->draw_img(b.getImage(),
                 Rectangle(b.getPos() + offset, b.getPos() + b.getSize() + offset), NULL_RECT,
                 0, false);
    }
    

    win->draw_img(player->getImage(),
                 Rectangle(player->getPos() + offset, player->getPos() + player->getSize() + offset), NULL_RECT,
                 0, false);
}