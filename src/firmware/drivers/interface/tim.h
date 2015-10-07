#ifndef __TIM_H__
#define __TIM_H__

void timInit(void);
// uint8_t TIM_Lock(uint8_t _RW, uint8_t _bit);
uint8_t timSetPulse(TIM_TypeDef* TIMx, uint8_t channel, uint16_t value);
// RW = 1 => Write

#endif
