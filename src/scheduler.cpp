#include "scheduler.h"

Scheduler::Task Scheduler::task;
volatile Scheduler::Status Scheduler::status;


extern "C" void Wiring_TIM2_Interrupt_Handler_override()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        Scheduler::onInterrupt();
    }
}

int Scheduler::schedule(uint16_t delayTime, taskHandler handler, void* arg)
{
    // Check if a task is already scheduled
    if(status == TIMER_ON){
        return -1;
    }

    task.fp.attach(handler);
    task.arg = arg;

    if(status != TIMER_ON){
        Scheduler::start(delayTime);
    }

    return 0;
}

void Scheduler::start(uint16_t delayTime)
{
    status = TIMER_ON;

    TIM_TimeBaseInitTypeDef timerInitStructure;
    NVIC_InitTypeDef nvicStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2, DISABLE);

    Wiring_TIM2_Interrupt_Handler = Wiring_TIM2_Interrupt_Handler_override;

    //Enable Timer Interrupt
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    nvicStructure.NVIC_IRQChannel = TIM2_IRQn;

    NVIC_Init(&nvicStructure);

    timerInitStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / 2000) - 1;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = delayTime * 2;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    TIM_Cmd(TIM2, ENABLE);
}

void Scheduler::onInterrupt()
{
    Status oldStatus = status;
    status = TIMER_OFF;

    if(oldStatus == TIMER_ON && Scheduler::task.fp.attached())
    {
        Scheduler::task.fp(Scheduler::task.arg);
    }
}
