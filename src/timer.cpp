#include "timer.h"

Timer::Timer(system_tick_t timeout_ms)
{
    this->countdown_ms(timeout_ms);
}

void Timer::countdown(system_tick_t timeout_s)
{
    this->countdown_ms(timeout_s * 1000);
}

void Timer::countdown_ms(system_tick_t timeout_ms)
{
    start_ms = millis();
    this->timeout_ms = timeout_ms;
}

bool Timer::expired()
{
    return this->left_ms() == 0;
}

unsigned int Timer::left_ms()
{
    system_tick_t elapsed = millis() - this->start_ms;

    if(elapsed >= this->timeout_ms)
    {
        return 0;
    } else {
        return this->timeout_ms - elapsed;
    }
}
