#include "world.hpp"
#define MIN_FRAME_RATE 60   


class Core{
public:
    Core();
    void draw();
    bool events();

    void loop();
    void update();
    
    void showDebug();
    void drawObjects();
private:
    World* world;
    Window* win;

    int FPS;
    int frameTime;

    // Keyboard events
    bool KEY_UP_PRESSED = false;
    bool KEY_DOWN_PRESSED = false;
    bool KEY_LEFT_PRESSED = false;
    bool KEY_RIGHT_PRESSED = false;

};