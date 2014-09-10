#include "sound.h"

uint8_t playingPin;
unsigned char *playing;
uint16_t sampleRate;
uint16_t playingLength;
uint16_t cycle;
uint16_t sampleCycles;
uint16_t pos;

extern void (*Wiring_TIM3_Interrupt_Handler)(void);

extern "C" void TIM3_Sound_Interrupt_Handler()
{
    if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);

        cycle++;
        if (cycle < sampleCycles) {
            return;
        }

        cycle = 0;
        pos++;

        if(pos > playingLength){
            TIM_ITConfig(TIM3, TIM_IT_CC4, DISABLE);
            TIM_CCxCmd(TIM3, TIM_Channel_4, TIM_CCx_Disable);
            pinMode(playingPin, INPUT);
        }
        else
        {
            TIM_SetCompare4(TIM3, playing[pos]);
        }
    }
}

void playSound(uint8_t pin, unsigned char* sound, uint16_t length){
    TIM_TimeBaseInitTypeDef timerInitStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    NVIC_InitTypeDef nvicStructure;

    if(PIN_MAP[pin].timer_peripheral != TIM3){
        DEBUG("This code only works with TIM3, got %d",
              PIN_MAP[pin].timer_peripheral);
        return;
    }
    if(PIN_MAP[pin].timer_ch != TIM_Channel_4){
        DEBUG("This code only works with TIM_Channel_4, got %d",
              PIN_MAP[pin].timer_ch);
        return;
    }

    playingPin = pin;
    playing = sound;
    cycle = 0;
    pos = WAV_START;
    playingLength = length;
    sampleRate = *((uint32_t*)&playing[24]);
    sampleCycles = (SystemCoreClock / 256) / sampleRate;
    DEBUG("Sample rate: %d\t Cycles per sample: %d", sampleRate, sampleCycles);

    pinMode(pin, AF_OUTPUT_PUSHPULL);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, DISABLE);

    Wiring_TIM3_Interrupt_Handler = TIM3_Sound_Interrupt_Handler;

    //Enable Timer Interrupt
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    nvicStructure.NVIC_IRQChannel = TIM3_IRQn;

    NVIC_Init(&nvicStructure);

    timerInitStructure.TIM_Prescaler = 0;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 256;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM3, &timerInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = playing[pos];

    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_ITConfig(TIM3, TIM_IT_CC4, ENABLE);
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);

    TIM_Cmd(TIM3, ENABLE);
}
