#include "Timer.h"

Timer::Timer()
{
    //ctor
}

Timer::~Timer()
{
    //dtor
}

void Timer::start() {
    startT = std::chrono::steady_clock::now();
}

int Timer::get() {
    std::chrono::time_point<std::chrono::steady_clock> endT = std::chrono::steady_clock::now();

    std::chrono::microseconds diff = std::chrono::duration_cast<std::chrono::microseconds>(endT - startT);

    return diff.count();
}
