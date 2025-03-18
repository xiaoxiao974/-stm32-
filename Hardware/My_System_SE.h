#ifndef __MY_SYSTEM_SE_H
#define __MY_SYSTEM_SE_H

extern uint8_t My_System_SE_Flag;
extern uint8_t sleep_flag;
void _DelayMs(volatile uint16_t xMs);
void My_System_SE_Init(void);
void EXTI3_IRQHandler(void);

#endif
