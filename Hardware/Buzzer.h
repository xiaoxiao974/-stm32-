#ifndef __BUZZER_H
#define __BUZZER_H

extern uint8_t Buzzer_Flag;
void Buzzer_Init(void);
void TIM2_IRQHandler(void);

#endif
