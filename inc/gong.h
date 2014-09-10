#ifndef __GONG_H
#define __GONG_H

#include <stdint.h>

#include "spark_wiring_servo.h"
#include "timer.h"
#include "scheduler.h"

enum actionType { PAUSE, GOTO };

struct Action
{
    actionType type;
    uint16_t value;
};

#define WINDUP_POS 40
#define NEUTRAL_POS 50
#define HIT_POS 66

const Action GONG_SERVO_PATH[] = {
    Action { GOTO, WINDUP_POS },
    Action { PAUSE, 200 }, // Stabilize before hit
    Action { GOTO, HIT_POS },
    // Action { PAUSE, 50 },
    Action { GOTO, NEUTRAL_POS },
    Action { PAUSE, 3000 }, // Delay 2nd hit a bit
    Action { GOTO, WINDUP_POS },
    Action { PAUSE, 200 }, // Stabilize before hit
    Action { GOTO, HIT_POS },
    // Action { PAUSE, 50 },
    Action { GOTO, NEUTRAL_POS },
    Action { PAUSE, 200 } // Ensure we stabilize in a neutral position
};

// microseconds per degree
const uint16_t GONG_SERVO_RATE = 5000;

class Gong
{

public:
    Gong(uint8_t pwrPin, uint8_t ctrlPin);

    bool sound(void);

    // Generally should only be called from ISRs
    void nextAction(void);
    static void callNextAction(void* gong);

private:
    Servo servo;
    uint8_t pwrPin;

    volatile uint8_t currAction;

    void stop(void);
};

#endif  /* __GONG_H */
