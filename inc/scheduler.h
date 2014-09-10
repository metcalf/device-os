#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "FP.h"
#include "spark_wiring.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void (*Wiring_TIM2_Interrupt_Handler)(void);
extern void Wiring_TIM2_Interrupt_Handler_override(void);

class Scheduler {
public:
    enum Status {TIMER_OFF, TIMER_ON};
    struct Task {
        FP<void, void*> fp;
        void* arg;
    };

    typedef void (*taskHandler)(void*);

    static int schedule(uint16_t delayTime, taskHandler handler, void* arg);

    // Should only be called from ISR
    static void onInterrupt(void);

private:
    static Task task;
    static volatile Status status;

    static void start(uint16_t delay);
};

#ifdef __cplusplus
}
#endif

#endif  /* __SCHEDULER_H */
