/*
 *  ____    ____    __   __   ______  ______
 * /\  _`\ /\  _`\ /\ \ /\ \ /\__  _\/\  _  \
 * \ \ \/\ \ \ \L\_\ `\`\/'/'\/_/\ \/\ \ \L\ \
 *  \ \ \ \ \ \  _\L`\/ > <     \ \ \ \ \  __ \
 *   \ \ \_\ \ \ \L\ \ \/'/\`\   \ \ \ \ \ \/\ \
 *    \ \____/\ \____/ /\_\\ \_\  \ \_\ \ \_\ \_\
 *     \/___/  \/___/  \/_/ \/_/   \/_/  \/_/\/_/
 *
 * Originally created by Dexta Robotics.
 * Copyright <C> Dexta Robotics, 2015.
 * All rights reserved.
 */

#ifndef __MPU9150_H
#define __MPU9150_H

#include "stm32f10x.h"

typedef enum {
	MPU9150_STATE_IDLE			= 0x01,
    MPU9150_STATE_WRITING		= 0x02,
    MPU9150_STATE_READING		= 0x03,
    MPU9150_STATE_ERROR 		= 0x04,
} MPU9150_StateTypeDef;


#define MPU9150_TIMEOUT                  ((uint32_t)0x3FFFF)

extern uint8_t MPU9150_ADDR;

typedef struct {
    unsigned long sensor_timestamp;
    short gyro[3];
    short accel[3];
    short compass[3];
    long quat[4];
    float eular[3];
} mpu_data_struct;

extern mpu_data_struct mpu_data;

void mpu9150Init(void);	 
void mpu9150DeInit(void);
uint8_t mpu9150Status (void);
ErrorStatus mpu9150GetStatus(void);
uint8_t mpu9150Get(void);

typedef enum 
{  
  MPU9150_INT_MODE_GPIO = 0,
  MPU9150_INT_MODE_EXTI = 1
} MPU9150intMode_TypeDef;

/** Define the Interrupt pin */  
#define MPU9150_INT_PIN                          GPIO_Pin_7
#define MPU9150_INT_GPIO_PORT                    GPIOC                 
#define MPU9150_INT_GPIO_CLK                     RCC_APB2Periph_GPIOC  
#define MPU9150_INT_EXTI_LINE                    EXTI_Line7
#define MPU9150_INT_EXTI_PORT_SOURCE             GPIO_PortSourceGPIOC  
#define MPU9150_INT_EXTI_PIN_SOURCE              GPIO_PinSource7
#define MPU9150_INT_EDGE                         EXTI_Trigger_Rising 
#define MPU9150_INT_EXTI_IRQn                    EXTI9_5_IRQn

#define MPU9150_INT_EXTI_PREEMPTION_PRIORITY     14
#define MPU9150_INT_EXTI_SUB_PRIORITY            0

void mpu9150InterruptInit(MPU9150intMode_TypeDef Button_Mode);
void mpu9150InterruptCmd(uint8_t NewState);
uint32_t mpu9150GetINTPinState(void);


#endif
