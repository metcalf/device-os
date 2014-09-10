#ifndef BUTTONAPP_H_
#define BUTTONAPP_H_

#include "base_app.h"
#include "spark_wiring_tone.h"
#include "sound.h"

#include <stdint.h>

class ButtonApp: public BaseApp
{
public:
    ButtonApp(uint8_t spkrPin): spkrPin(spkrPin){};

    virtual void onOwnButtonMessage();

private:
    uint8_t spkrPin;
};

#endif /* BUTTONAPP_H_*/
