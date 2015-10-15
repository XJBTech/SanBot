#ifndef __TIM_H__
#define __TIM_H__

#define TIM_MOTOR_QTY		3
#define TIM_USED_QTY		6

#define TIM_STARTANGLE		0.0f

typedef struct {
	// from 0 to 2PI 4byte
	float angle;

	// from 0 to 1023
	uint16_t velocity;

	// from 0 to 1023
	uint16_t spin;

	// ms
	uint16_t duration;

	uint16_t pulse[TIM_USED_QTY];
	uint32_t sysTimer;
} move_specfic;

void timInit(void);
// uint8_t TIM_Lock(uint8_t _RW, uint8_t _bit);
uint8_t timSetPulse(TIM_TypeDef* TIMx, uint8_t channel, uint16_t value);
// RW = 1 => Write
uint8_t timMove(move_specfic _import);
uint8_t timImportQueue ( move_specfic _import );

#endif
