#include "map.hpp"


Map::Map(std::string filepath, Window* win){
    this->offset.x = 0;
    this->offset.y = 0;
    
    this->win = win;
    std::ifstream map_file;
    map_file.open(filepath);
    std::string line;
    int y = 0;
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
}