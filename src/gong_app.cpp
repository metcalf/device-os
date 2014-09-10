#include "gong_app.h"

GongApp::GongApp(uint8_t pwrPin, uint8_t ctrlPin)
{
    gong = new Gong(pwrPin, ctrlPin);
}

void GongApp::onButtonPress()
{
    gong->sound();
}

void GongApp::onButtonMessage()
{
    if(rateLimitTimer.expired())
    {
        gong->sound();
        rateLimitTimer.countdown_ms(GONG_RATE_LIMIT);
    }
    else
    {
        DEBUG("Not sounding gong, rate limited");
    }
}
