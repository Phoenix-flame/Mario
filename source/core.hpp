#include "world.hpp"
#include "audio.hpp"
#include "objects/bullet.hpp"
#include <chrono>
using namespace std::chrono;

#define MIN_FRAME_RATE 30

#define BACKGROUND "assets/background_image.png"

class Core
{
public:
    // startLevel picks which map to load first; the game still advances
    // through the remaining levels from there.
    Core(int startLevel = 1);
    ~Core();
    void draw();
    bool events();

    void loop();
    void update();
    void resetGame(int level = 1);

    void showDebug();
    // Screenshot helpers, also used to regenerate the README images.
    bool saveScreenshot(const std::string &path);
    void setDebug(bool enabled) { debugEnabled = enabled; }
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
    Timer *shootTimer;
    Timer *levelCompleteTimer;

    bool debugEnabled = false;

    int currentLevel = 1;
    static const int TOTAL_LEVELS = 3;
    bool levelCompleted = false;

    // Keyboard events
    bool KEY_UP_PRESSED = false;
    bool KEY_DOWN_PRESSED = false;
    bool KEY_LEFT_PRESSED = false;
    bool KEY_RIGHT_PRESSED = false;
    bool KEY_SHIFT_PRESSED = false;
};
