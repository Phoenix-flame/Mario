#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include "rsdl.hpp"
#include <chrono>
#include <cstdint>
using namespace std::chrono;

class Timer
{
public:
    Timer() { reset(); };

    // RL training runs the engine without sleeping. In that mode all gameplay
    // timers use this deterministic clock instead of wall time.
    static void useSimulatedTime(std::uint64_t startMilliseconds = 0)
    {
        simulatedTimeEnabled() = true;
        simulatedMilliseconds() = startMilliseconds;
    }

    static void advanceSimulatedTime(std::uint64_t milliseconds)
    {
        if (simulatedTimeEnabled())
        {
            simulatedMilliseconds() += milliseconds;
        }
    }

    static void useRealtime()
    {
        simulatedTimeEnabled() = false;
    }

    void reset()
    {
        time = nowMilliseconds();
        started = false;
    }
    int getTime()
    {
        return static_cast<int>(nowMilliseconds() - time);
    }

    int getSecs()
    {
        if (!started)
        {
            return 0;
        }
        return static_cast<int>((nowMilliseconds() - time) / 1000);
    }

    int getMilliseconds()
    {
        if (!started)
        {
            return 0;
        }
        return static_cast<int>(nowMilliseconds() - time);
    }

    void start()
    {
        time = nowMilliseconds();
        started = true;
    }
    bool isStarted()
    {
        return started;
    }

private:
    static bool &simulatedTimeEnabled()
    {
        static bool enabled = false;
        return enabled;
    }

    static std::uint64_t &simulatedMilliseconds()
    {
        static std::uint64_t current = 0;
        return current;
    }

    static std::uint64_t nowMilliseconds()
    {
        if (simulatedTimeEnabled())
        {
            return simulatedMilliseconds();
        }
        return static_cast<std::uint64_t>(SDL_GetTicks());
    }

    std::uint64_t time;
    bool started = false;
};

#endif // !_TIMER_HPP_
