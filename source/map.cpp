#include "map.hpp"


Map::Map(std::string filepath){
    std::ifstream map_file;
    std::string line;
    int y = 0;
    while (std::getline(map_file, line)){
        for(unsigned int x = 0 ; x < line.size() ; x ++){
            if (line[x] == '@'){
                Block tmp(x, y);
                this->blocks.push_back(tmp);
            }
        }
        y += 1;
    }

    map_file.close();


}