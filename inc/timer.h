#ifndef __TIMER_H
#define __TIMER_H

#include "spark_wiring.h"

class Timer {
public:
    Timer(system_tick_t timeout_ms = 0);

    void countdown(system_tick_t timeout_s);
    void countdown_ms(system_tick_t timeout_ms);
    bool expired(void);
    unsigned int left_ms(void);

private:
    system_tick_t start_ms;
    system_tick_t timeout_ms;
};

#endif  /* __TIMER_H */
