#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "block.hpp"

class Map{
public:
    Map(std::string filepath);


private:
    std::vector<Block> blocks;
};