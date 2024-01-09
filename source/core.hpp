#include "world.hpp"
#include "audio.hpp"
#include <chrono>
using namespace std::chrono;

#define MIN_FRAME_RATE 30

#define BACKGROUND "assets/background_image.png"

class Core
{
public:
    Core();
    void draw();
    bool events();

    void loop();
    void update();
    void resetGame();

    void showDebug();
    void drawObjects();
    void drawBackground();
    void drawHood();

private:
    World *world;
    Window *win;
    Audio *audio;

    double FPS;
    int frameTime;
    Timer *gameTimer;
    Timer *endGameTimer;

    // Keyboard events
    bool KEY_UP_PRESSED = false;
    bool KEY_DOWN_PRESSED = false;
    bool KEY_LEFT_PRESSED = false;
    bool KEY_RIGHT_PRESSED = false;
    bool KEY_SHIFT_PRESSED = false;
};