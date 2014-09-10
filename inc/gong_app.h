#ifndef GONGAPP_H_
#define GONGAPP_H_

#include "base_app.h"
#include "gong.h"
#include "timer.h"

#include <stdint.h>

const uint16_t GONG_RATE_LIMIT = 20000; // milliseconds between soundings from remote command

class GongApp: public BaseApp
{
public:
    GongApp(uint8_t pwrPing, uint8_t ctrlPin);

    virtual void onButtonPress();
    virtual void onButtonMessage();

private:
    Timer rateLimitTimer;
    Gong *gong;
};

#endif /* GONGAPP_H_*/
