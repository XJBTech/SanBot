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

#define DEBUG_MODULE "MPU"

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#include "config.h"
#include "debug.h"
#include "mpu9150.h"
#include "i2cs.h"
#include "rcc.h"

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
// #include "quaternionFilters.h"

// long quat[4];

uint8_t MPU9150_ADDR = 255;

mpu_data_struct mpu_data;

typedef struct {
    //volatile stm32_i2c_state state;
    /* First slave register. */
    uint8_t slave_reg;
    /* 0 if slave register has not been written yet. */
    uint8_t slave_reg_written;
    uint8_t *data;
    unsigned short length;
    uint8_t enabled;
} stm32_i2c_info;

__IO uint32_t  MPU9150_Timeout = MPU9150_TIMEOUT;

#define PI 3.1415926f

#define PRINT_ACCEL     (0x01)
#define PRINT_GYRO      (0x02)
#define PRINT_QUAT      (0x04)
#define PRINT_COMPASS   (0x08)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)

#define MOTION          (0)
#define NO_MOTION       (1)

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (100)

static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};

struct int_param_s int_param;

struct rx_s {
    unsigned char header[3];
    unsigned char cmd;
};
struct hal_s {
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned short report;
    unsigned short dmp_features;
    unsigned char motion_int_mode;
    struct rx_s rx;
};
static struct hal_s hal = {0};

static inline unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

static inline unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

void mpu9150Init(void)
{
    uint8_t i, addr;
    // unsigned short fsr[1];
    int result;

    i2csInit();

    DEBUG_PRINT("probing devices...\n");

    for(addr = 0; addr < 127; addr ++)
    {
        i = i2csProbing(addr);
        if(!i){
            DEBUG_PRINT("find I2CS [%02X]\n", addr);
            MPU9150_ADDR = addr;
        }
    }

    if(MPU9150_ADDR == 255)
    {
        DEBUG_PRINT("no device be founded\n");
        return;
    }

    int_param.cb = 0;
    int_param.pin = 0;
    int_param.lp_exit = 0;
    int_param.active_low = 1;
    // "int_param" structure is doing nothing here, just statisfying calling convention
    result = mpu_init(&int_param, MPU9150_ADDR);
    DEBUG_PRINT("mpu init => %d\r\n", result);
    // delay_ms(1000);
    // while(1);
    result = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
    DEBUG_PRINT("sensor setup => %d\r\n", result);
    // delay_ms(1000);
    result = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    DEBUG_PRINT("mpu_configure_fifo => %d\r\n", result);
    // delay_ms(1000);

    result = dmp_load_motion_driver_firmware();// load the DMP firmware
    DEBUG_PRINT("firmware loaded => %d\r\n", result);
    // delay_ms(1000);
    result = dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
    // result = dmp_register_tap_cb(tap_cb);
    // result = dmp_register_android_orient_cb(android_orient_cb);
    DEBUG_PRINT("dmp_set_orientation => %d\r\n", result);
    // delay_ms(1000);
    // hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
    //     DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
    //     DMP_FEATURE_GYRO_CAL;
    hal.dmp_features = DMP_FEATURE_6X_LP_QUAT;
    result = dmp_enable_feature(hal.dmp_features);
    DEBUG_PRINT("dmp_enable_feature => %d\r\n", result);
    // delay_ms(1000);
    result = dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    DEBUG_PRINT("dmp_set_fifo_rate => %d\r\n", result);
    // delay_ms(1000);
    result = mpu_set_dmp_state(1);
    DEBUG_PRINT("mpu_set_dmp_state => %d\r\n", result);
    // delay_ms(1000);
    /* Initialize HAL state variables. */
    memset(&hal, 0, sizeof(hal));
    hal.sensors = ACCEL_ON | GYRO_ON;
    hal.report = PRINT_QUAT;
    result = mpu_set_sample_rate(DEFAULT_MPU_HZ);
    DEBUG_PRINT("mpu_set_sample_rate => %d\r\n", result);
    // delay_ms(1000);
    result = mpu_set_compass_sample_rate(100);       // set the compass update rate to match
    DEBUG_PRINT("mpu_set_compass_sample_rate => %d\r\n", result);
    // delay_ms(1000);
    /************** 3. Enable the Interrupt now **************************/
    // MPU9150_Interrupt_Cmd(ENABLE);
    hal.dmp_on = 1;

    // mpu_set_accel_fsr(8);
    // mpu_get_accel_fsr(fsr);
    // DEBUG_PRINT("mpu_accel_fsr => %d\r\n", fsr[0]);

    // mpu_set_gyro_fsr(2000);
    // mpu_get_gyro_fsr(fsr);
    // DEBUG_PRINT("mpu_gyro_fsr => %d\r\n", fsr[0]);

    // mpu_get_compass_fsr(fsr);
    // DEBUG_PRINT("mpu_compass_fsr => %d\r\n", fsr[0]);

}

uint8_t mpu9150Get(void)
{
    // unsigned long sensor_timestamp;
    // short gyro[3], accel[3];
    // short compass[3];
    unsigned char more;

    // int result;
    // long quat[4];
    // delay_ms(50);
    // mpu_get_gyro_reg(mpu_data.gyro, &mpu_data.sensor_timestamp);
    // DEBUG_PRINT("mpu_get_gyro_reg => %d\r\n", result);
    // printf("gyro=>[%d,%d,%d]\r\n", gyro[0], gyro[1], gyro[2]);
    // mpu_get_accel_reg(mpu_data.accel, &mpu_data.sensor_timestamp);
    // DEBUG_PRINT("mpu_get_gyro_reg => %d\r\n", result);
    // Set_LED(0,1);
    // result = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
    // result = dmp_read_quat(quat, &more);
    // mpu_get_compass_reg(mpu_data.compass, &mpu_data.sensor_timestamp);
    // DEBUG_PRINT("mpu_get_compass_reg => %d\r\n", result);
    // printf("quat=>[%d,%d,%d,%d] compass=>[%d,%d,%d]\r\n", quat[0], quat[1], quat[2], quat[3], compass[0], compass[1], compass[2]);
    // MadgwickQuaternionUpdate(accel[0]/32768.0f*8.0f, accel[1]/32768.0f*8.0f, accel[2]/32768.0f*8.0f, gyro[0]*PI/180.0f/32768.0f*2000.0f, gyro[1]*PI/180.0f/32768.0f*2000.0f, gyro[2]*PI/180.0f/32768.0f*2000.0f, compass[0]*10.0f*4912.0f/8190.0f, compass[1]*10.0f*4912.0f/8190.0f, compass[2]*10.0f*4912.0f/8190.0f, q);
    // DEBUG_PRINT("q=>[%10.5f,%10.5f,%10.5f,%10.5f]\n", q[0], q[1], q[2], q[3]);
    // float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float * q
    // DEBUG_PRINT("accel=>[%6d,%6d,%6d] gyro=>[%6d,%6d,%6d] compass=>[%5d,%5d,%5d] \n",
        // mpu_data.accel[0], mpu_data.accel[1], mpu_data.accel[2],
        // mpu_data.gyro[0], mpu_data.gyro[1], mpu_data.gyro[2],
        // mpu_data.compass[0], mpu_data.compass[1], mpu_data.compass[2]);

    // DEBUG_PRINT("ST -> %d\n", timerGetRun());
    dmp_read_quat(mpu_data.quat, &more);
    // DEBUG_PRINT("read quat from dmp st -> %d\n", timerGetRun());
    // DEBUG_PRINT("quat=>[%8d,%8d,%8d,%8d]\r\n",
        // mpu_data.quat[0], mpu_data.quat[1], mpu_data.quat[2], mpu_data.quat[3]);
    return 0;
}

uint8_t mpu9150Status (void)
{
    uint8_t i;
    i = i2csProbing(MPU9150_ADDR);
    if(!i){
        // flag ++;
        DEBUG_PRINT("connection success\n");
        // Set_LED(1, 0);
    } else {
        DEBUG_PRINT("connection failed\n");
    }
    return i;
}

/**
  * @brief  Checks the MPU9150 status.
  * @param  None
  * @retval ErrorStatus: MPU9150 Status (ERROR or SUCCESS).
  */
ErrorStatus mpu9150GetStatus(void)
{
  /* Test if MPU9150 is ready */
  while ((mpu9150Status() == 1) && MPU9150_Timeout)
  {
    MPU9150_Timeout--;
  }
  /* If MPU9150 is not responding return ERROR */
  if (MPU9150_Timeout == 0)
  {
    return ERROR;
  }
  /* In other case return SUCCESS */
  return SUCCESS;
}

void mpu9150InterruptInit(MPU9150intMode_TypeDef MPU9150int_Mode)
{

  // EXTI_InitTypeDef EXTI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable MPU9150int GPIO clocks */
  RCC_APB2PeriphClockCmd(MPU9150_INT_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);

  /* Configure MPU9150int pin as input floating */
  GPIO_InitStructure.GPIO_Pin = MPU9150_INT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(MPU9150_INT_GPIO_PORT, &GPIO_InitStructure);

  // if (MPU9150int_Mode == MPU9150_INT_MODE_EXTI)
  // {
  //   /* Connect MPU9150int EXTI Line to MPU9150int GPIO Pin */
  //   GPIO_EXTILineConfig(MPU9150_INT_EXTI_PORT_SOURCE, MPU9150_INT_EXTI_PIN_SOURCE);

  //   /* Configure MPU9150int EXTI line */
  //   EXTI_InitStructure.EXTI_Line = MPU9150_INT_EXTI_LINE;
  //   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //   EXTI_InitStructure.EXTI_Trigger = MPU9150_INT_EDGE;
  //   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  //   EXTI_Init(&EXTI_InitStructure);
  // }
}

void mpu9150InterruptCmd(uint8_t NewState)
{
  // NVIC_InitTypeDef NVIC_InitStructure;

  // /* Enable and set MPU9150int EXTI Interrupt priority */
  // NVIC_InitStructure.NVIC_IRQChannel = MPU9150_INT_EXTI_IRQn;
  // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MPU9150_INT_EXTI_PREEMPTION_PRIORITY;
  // NVIC_InitStructure.NVIC_IRQChannelSubPriority = MPU9150_INT_EXTI_SUB_PRIORITY;
  // NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
  // NVIC_Init(&NVIC_InitStructure);
}

uint32_t mpu9150GetINTPinState(void)
{
  return GPIO_ReadInputDataBit(MPU9150_INT_GPIO_PORT, MPU9150_INT_PIN);
}
