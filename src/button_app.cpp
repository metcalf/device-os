#include "button_app.h"

extern unsigned char _gong_wav[];
extern uint16_t _gong_wav_size;

void ButtonApp::onOwnButtonMessage(){
    playSound(spkrPin, _gong_wav, _gong_wav_size);
}
