#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include "rsdl.hpp"
#include <chrono>
using namespace std::chrono;

class Timer{
public:
    Timer(){time = 0;};
    void reset(){
        time = 0;
        clock = 0;
        started = false;
    }
    int getTime(){
        return SDL_GetTicks() - time;
    }

    double getTicks(){
        int cur = system_clock::now().time_since_epoch().count();
        return (cur - clock)/1000000.0;
    }

    void start(){
        time = SDL_GetTicks();
        started = true;
        clock = system_clock::now().time_since_epoch().count();
    }
    bool isStarted(){
        return started;
    }

private:    
    int time;
    int clock;
    bool started = false;
};


#endif // !_TIMER_HPP_