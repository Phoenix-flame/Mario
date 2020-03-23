#include "rsdl.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include "map.hpp"


#define MIN_FRAME_RATE 60


class Core{
public:
    Core();
    void draw();
    bool events();

    void loop();

private:
    Map* map;
    Window* win;
    int frameTime;

};