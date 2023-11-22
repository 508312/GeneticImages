#ifndef TIMER_H
#define TIMER_H
#include <chrono>

class Timer
{
    public:
        Timer();
        virtual ~Timer();
        void start();
        int get();

    protected:

    private:
        std::chrono::time_point<std::chrono::steady_clock> startT;
};

#endif // TIMER_H
