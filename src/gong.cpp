#include "gong.h"

Gong::Gong(uint8_t pwrPin, uint8_t ctrlPin) : pwrPin(pwrPin)
{
    currAction = 0;

    servo.attach(ctrlPin);

    pinMode(this->pwrPin, OUTPUT);
    digitalWrite(this->pwrPin, LOW);
}

bool Gong::sound()
{
    if(currAction > 0)
    {
        DEBUG("Not sounding gong, already running");
        return false;
    }

    DEBUG("Sounding gong");

    currAction = 0;
    this->nextAction();

    digitalWrite(this->pwrPin, HIGH);

    return true;
}

void Gong::nextAction()
{
    // We're done
    if(currAction >= sizeof(GONG_SERVO_PATH) / sizeof(Action)){
        this->stop();
        return;
    }

    Action action = GONG_SERVO_PATH[currAction];
    uint16_t actionTime;

    if(action.type == PAUSE){
        actionTime = action.value;
        DEBUG("PAUSE %dms", actionTime);
    } else if(action.type == GOTO){
        actionTime = (abs(servo.read() - action.value) * GONG_SERVO_RATE) / 1000;
        servo.write(action.value);

        DEBUG("GOTO %d", action.value);
    } else {
        DEBUG("Unexpected action %d", action.type);
        // Uhh this is an error
        return;
    }

    currAction++;

    // Reschedule self for time in milliseconds if possible
    if(Scheduler::schedule(actionTime, Gong::callNextAction, (void*)this)){
        this->stop();
    }
}

void Gong::callNextAction(void* gong){
    ((Gong*)gong)->nextAction();
}

void Gong::stop(void)
{
    DEBUG("Stopping gong servo");
    this->currAction = 0;
    digitalWrite(this->pwrPin, LOW);
}
