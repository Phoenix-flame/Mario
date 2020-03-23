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
    void update();
    
    void showDebug();
private:
    Map* map;
    Window* win;

    int FPS;
    int frameTime;

    // Keyboard events
    bool KEY_UP_PRESSED = false;
    bool KEY_DOWN_PRESSED = false;
    bool KEY_LEFT_PRESSED = false;
    bool KEY_RIGHT_PRESSED = false;

};