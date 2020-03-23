#include "rsdl.hpp"


class Timer{
public:
    Timer(){time = 0;};
    void reset(){
        time = 0;
        started = false;
    }
    int getTime(){
        return SDL_GetTicks() - time;
    }
    void start(){
        time = SDL_GetTicks();
        started = true;
    }
    bool isStarted(){
        return started;
    }

private:    
    int time;
    bool started = false;
};