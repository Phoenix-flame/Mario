#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "rsdl.hpp"
#include "block.hpp"
#include "ground.hpp"

class Map{
public:
    Map(std::string filepath, Window* win);
    void drawObjects();
    Point offset;
private:
    std::vector<Block> blocks;
    std::vector<Ground> grounds;

    Window* win;
};