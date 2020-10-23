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
        started = false;
        clock = high_resolution_clock::now();
    }
    int getTime(){
        return SDL_GetTicks() - time;
    }

    int getSecs(){
        if (!started){ return 0;}
        auto end = high_resolution_clock::now();
        return (duration_cast<seconds>(end - clock).count());
    }

    int getMilliseconds(){
        if (!started){ return 0;}
        // std::cout << "here" << std::endl;
        auto end = high_resolution_clock::now();
        return (duration_cast<milliseconds>(end - clock).count());
    }

    void start(){
        time = SDL_GetTicks();
        started = true;
        clock = high_resolution_clock::now();
    }
    bool isStarted(){
        return started;
    }

private:    
    int time;
    system_clock::time_point clock;
    bool started = false;
};


#endif // !_TIMER_HPP_