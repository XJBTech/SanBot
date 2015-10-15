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

#define DEBUG_MODULE "MAIN"

#include "config.h"
#include "system.h"
#include "platform.h"

#include "rcc.h"

#include "debug.h"

#include "led.h"

#include "nRF24L01.h"

#if defined PLATFORM_DEVICE_SANBOT_A

    #include "sbn1.sanbot_a.h"
    #include "mpu9150.h"
    #include "adc.h"
    #include "tim.h"


int main()
{
    systemInit();
    platformInit();

    DEBUG_PRINT ( "init successfully\n" );

    // {
    //     move_specfic temp_params;

    //     temp_params.angle = 3.1415926f;
    //     temp_params.velocity = 300;
    //     temp_params.spin     = 0;
    //     temp_params.duration = 0;

    //     timImportQueue ( temp_params );

    // }

    while ( 1 )
    {

        uint8_t i;

        nrf24l01RxMode();

        // waiting package
        i = 0x00;

        while ( i != RX_DR )
        {
            i = nrf24l01RxData ( nRF_ReceiveBuffer );
        }

        ledSet(0, 1);
        ledSet(1, 1);
        ledSet(2, 1);

        sbn1HandleReceived();

        DEBUG_PRINT ( "Received packet\r\n" );

        ledSet(0, 0);
        ledSet(1, 0);
        ledSet(2, 0);


    }

    while(1){
        uint16_t angle;


        for ( angle = 0; angle < 360; angle++ )
        {

            move_specfic temp_params;

            // DEBUG_PRINT("tp1\n");

            temp_params.angle = 2.0f * 3.1415926f * ( float ) angle / 360.0f;
            temp_params.velocity = 1023;
            temp_params.spin     = 0;
            temp_params.duration = 0;

            // DEBUG_PRINT("tp1\n");

            timImportQueue ( temp_params );

            delay_ms ( 20 );
        }
    }

    while ( 1 )
    {

        uint8_t i;

        nrf24l01RxMode();

        // waiting package
        i = 0x00;

        while ( i != RX_DR )
        {
            i = nrf24l01RxData ( nRF_ReceiveBuffer );
        }

        sbn1HandleReceived();

        DEBUG_PRINT ( "Received packet\r\n" );

    }

    return 0;
}

#endif

#if defined PLATFORM_DEVICE_SANBOT_DONGLE

    #include "sbn1.sanbot_dongle.h"


int main()
{
    systemInit();
    platformInit();

    DEBUG_PRINT ( "init successfully\n" );

    while ( 1 )
    {

        SBN1_USART_Handle_Reveived();
        SBN1_Loop();

    }

    return 0;
}


#endif

#if defined PLATFORM_DEVICE_SANBOT_REMOTE

#include "sbn1.sanbot_a.h"
#include "mpu9150.h"
#include "adc.h"

int main()
{
    systemInit();
    platformInit();

    delay_ms(100);

    DEBUG_PRINT("init successfully\n");

    while(1){

        uint32_t systick = timerGetRun();

        mpu9150Get();

        // result = dmp_read_quat(quat, &more);
        // result = mpu_get_compass_reg(compass,&sensor_timestamp);

        while(timerGetRun() < systick + 49)
        {
            
        }

        sbn1HandleReceived();

    }
}

#endif
