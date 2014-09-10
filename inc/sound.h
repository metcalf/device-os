#ifndef SOUND_H_
#define SOUND_H_

#include "spark_wiring.h"

#include <stdint.h>

#define WAV_START 44

void playSound(uint8_t pin, unsigned char* sound, uint16_t length);

#endif /* SOUND_H_ */
