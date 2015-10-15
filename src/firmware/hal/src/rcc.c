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

#define DEBUG_MODULE "RCC"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"

#include "uart.h"
#include "rcc.h"
#include "debug.h"

#if defined PLATFORM_DEVICE_SANBOT_A
    #include "mpu9150.h"
#endif

static volatile uint32_t s_uiDelayCount = 0;
static volatile uint8_t s_ucTimeOutFlag = 0;

static SOFT_TMR s_tTmr[TMR_COUNT];

__IO int32_t g_iRunTime = 0;

static void timerSoftDec ( SOFT_TMR * _tmr );
static void timerRunPer10ms ( void );

// void __set_PRIMASK(uint32_t priMask)
// {
//   __ASM volatile ("MSR primask, %0" : : "r" (priMask) );
// }

void rccInit ( void )
{
    ErrorStatus HSEStartUpStatus;

    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig ( RCC_HSE_ON );

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    while ( HSEStartUpStatus != SUCCESS )
    {
        HSEStartUpStatus = RCC_WaitForHSEStartUp();
    }

    if ( HSEStartUpStatus == SUCCESS )
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd ( FLASH_PrefetchBuffer_Enable );

        /* Flash 2 wait state */
        FLASH_SetLatency ( FLASH_Latency_2 );

        if ( HSE_VALUE == 8000000 )
        {
            RCC_HCLKConfig ( RCC_SYSCLK_Div1 );
        }
        else
        {
            RCC_HCLKConfig ( RCC_SYSCLK_Div2 );
        }

        /* PCLK2 = HCLK */
        RCC_PCLK2Config ( RCC_HCLK_Div1 );

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config ( RCC_HCLK_Div2 );

        //RCC_ADCCLKConfig(RCC_PCLK2_Div4);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig ( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

        /* Enable PLL */
        RCC_PLLCmd ( ENABLE );

        while ( RCC_GetFlagStatus ( RCC_FLAG_PLLRDY ) == RESET ) {}

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig ( RCC_SYSCLKSource_PLLCLK );

        /* Wait till PLL is used as system clock source */
        while ( RCC_GetSYSCLKSource() != 0x08 ) {}
    }

    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );

}


void timerInit ( void )
{
    uint8_t i;

    /* 清零所有的软件定时器 */
    for ( i = 0; i < TMR_COUNT; i++ )
    {
        s_tTmr[i].Count = 0;
        s_tTmr[i].PreLoad = 0;
        s_tTmr[i].Flag = 0;
        s_tTmr[i].Mode = TMR_ONCE_MODE; /* 缺省是1次性工作模式 */
    }

    SysTick_Config ( SystemCoreClock / 1000 );
}

void SysTick_ISR ( void )
{
    static uint8_t s_count = 0;
    // uint8_t i;

    /* 每隔1ms进来1次 （仅用于 bsp_DelayMS） */
    if ( s_uiDelayCount > 0 )
    {
        if ( --s_uiDelayCount == 0 )
        {
            s_ucTimeOutFlag = 1;
        }
    }

    // /* 每隔1ms，对软件定时器的计数器进行减一操作 */
    // for (i = 0; i < TMR_COUNT; i++)
    // {
    //   timerSoftDec(&s_tTmr[i]);
    // }

    /* 全局运行时间每1ms增1 */
    g_iRunTime++;
    // if (g_iRunTime == 0x7FFFFFFF)  这个变量是 int32_t 类型，最大数为 0x7FFFFFFF
    // {
    //   g_iRunTime = 0;
    // }

    // // bsp_RunPer1ms();     每隔1ms调用一次此函数，此函数在 bsp.c

    if ( ++s_count >= 10 )
    {
        s_count = 0;
        timerRunPer10ms();
    }
}

/*
*********************************************************************************************************
* 函 数 名: bsp_SoftTimerDec
* 功能说明: 每隔1ms对所有定时器变量减1。必须被SysTick_ISR周期性调用。
* 形    参:  _tmr : 定时器变量指针
* 返 回 值: 无
*********************************************************************************************************
*/
static void timerSoftDec ( SOFT_TMR * _tmr )
{
    if ( _tmr->Count > 0 )
    {
        /* 如果定时器变量减到1则设置定时器到达标志 */
        if ( --_tmr->Count == 0 )
        {
            _tmr->Flag = 1;

            /* 如果是自动模式，则自动重装计数器 */
            if ( _tmr->Mode == TMR_AUTO_MODE )
            {
                _tmr->Count = _tmr->PreLoad;
            }
        }
    }
}

/*
*********************************************************************************************************
* 函 数 名: bsp_DelayMS
* 功能说明: ms级延迟，延迟精度为正负1ms
* 形    参:  n : 延迟长度，单位1 ms。 n 应大于2
* 返 回 值: 无
*********************************************************************************************************
*/
void timerDelayMs ( uint32_t n )
{
    if ( n == 0 )
    {
        return;
    }
    else if ( n == 1 )
    {
        n = 2;
    }

    DISABLE_INT();        /* 关中断 */

    s_uiDelayCount = n;
    s_ucTimeOutFlag = 0;

    ENABLE_INT();         /* 开中断 */

    while ( 1 )
    {
        // bsp_Idle();       /* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

        /*
          等待延迟时间到
          注意：编译器认为 s_ucTimeOutFlag = 0，所以可能优化错误，因此 s_ucTimeOutFlag 变量必须申明为 volatile
        */
        if ( s_ucTimeOutFlag == 1 )
        {
            break;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_DelayUS
*    功能说明: us级延迟。 必须在systick定时器启动后才能调用此函数。
*    形    参:  n : 延迟长度，单位1 us
*    返 回 值: 无
*********************************************************************************************************
*/
void timerDelayUs ( uint32_t n )
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;

    reload = SysTick->LOAD;
    ticks = n * ( SystemCoreClock / 1000000 ); /* 需要的节拍数 */

    tcnt = 0;
    told = SysTick->VAL;             /* 刚进入时的计数器值 */

    while ( 1 )
    {
        tnow = SysTick->VAL;

        if ( tnow != told )
        {
            /* SYSTICK是一个递减的计数器 */
            if ( tnow < told )
            {
                tcnt += told - tnow;
            }
            /* 重新装载递减 */
            else
            {
                tcnt += reload - tnow + told;
            }

            told = tnow;

            /* 时间超过/等于要延迟的时间,则退出 */
            if ( tcnt >= ticks )
            {
                break;
            }
        }
    }
}


/*
*********************************************************************************************************
* 函 数 名: bsp_StartTimer
* 功能说明: 启动一个定时器，并设置定时周期。
* 形    参:   _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*       _period : 定时周期，单位1ms
* 返 回 值: 无
*********************************************************************************************************
*/
void timerStart ( uint8_t _id, uint32_t _period )
{
    if ( _id >= TMR_COUNT )
    {
        /* 打印出错的源代码文件名、函数名称 */
        uartPrintf ( "Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__ );

        while ( 1 ); /* 参数异常，死机等待看门狗复位 */
    }

    DISABLE_INT();        /* 关中断 */

    s_tTmr[_id].Count = _period;    /* 实时计数器初值 */
    s_tTmr[_id].PreLoad =
        _period;    /* 计数器自动重装值，仅自动模式起作用 */
    s_tTmr[_id].Flag = 0;       /* 定时时间到标志 */
    s_tTmr[_id].Mode = TMR_ONCE_MODE; /* 1次性工作模式 */

    ENABLE_INT();         /* 开中断 */
}

/*
*********************************************************************************************************
* 函 数 名: bsp_StartAutoTimer
* 功能说明: 启动一个自动定时器，并设置定时周期。
* 形    参:   _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*       _period : 定时周期，单位10ms
* 返 回 值: 无
*********************************************************************************************************
*/
void timerStartAuto ( uint8_t _id, uint32_t _period )
{
    if ( _id >= TMR_COUNT )
    {
        /* 打印出错的源代码文件名、函数名称 */
        uartPrintf ( "Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__ );

        while ( 1 ); /* 参数异常，死机等待看门狗复位 */
    }

    DISABLE_INT();      /* 关中断 */

    s_tTmr[_id].Count = _period;      /* 实时计数器初值 */
    s_tTmr[_id].PreLoad =
        _period;    /* 计数器自动重装值，仅自动模式起作用 */
    s_tTmr[_id].Flag = 0;       /* 定时时间到标志 */
    s_tTmr[_id].Mode = TMR_AUTO_MODE; /* 自动工作模式 */

    ENABLE_INT();       /* 开中断 */
}

/*
*********************************************************************************************************
* 函 数 名: bsp_StopTimer
* 功能说明: 停止一个定时器
* 形    参:   _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
* 返 回 值: 无
*********************************************************************************************************
*/
void timerStop ( uint8_t _id )
{
    if ( _id >= TMR_COUNT )
    {
        /* 打印出错的源代码文件名、函数名称 */
        DEBUG_PRINT ( "Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__ );

        while ( 1 ); /* 参数异常，死机等待看门狗复位 */
    }

    DISABLE_INT();    /* 关中断 */

    s_tTmr[_id].Count = 0;        /* 实时计数器初值 */
    s_tTmr[_id].Flag = 0;       /* 定时时间到标志 */
    s_tTmr[_id].Mode = TMR_ONCE_MODE; /* 自动工作模式 */

    ENABLE_INT();     /* 开中断 */
}

/*
*********************************************************************************************************
* 函 数 名: bsp_CheckTimer
* 功能说明: 检测定时器是否超时
* 形    参:   _id     : 定时器ID，值域【0,TMR_COUNT-1】。用户必须自行维护定时器ID，以避免定时器ID冲突。
*       _period : 定时周期，单位1ms
* 返 回 值: 返回 0 表示定时未到， 1表示定时到
*********************************************************************************************************
*/
uint8_t timerCheck ( uint8_t _id )
{
    // DEBUG_PRINT("check %d\n", _id);
    if ( _id >= TMR_COUNT )
    {
        return 0;
    }

    if ( s_tTmr[_id].Flag == 1 )
    {
        // s_tTmr[_id].Flag = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
*********************************************************************************************************
* 函 数 名: bsp_GetRunTime
* 功能说明: 获取CPU运行时间，单位1ms。最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
* 形    参:  无
* 返 回 值: CPU运行时间，单位1ms
*********************************************************************************************************
*/
int32_t timerGetRun ( void )
{
    int32_t runtime;

    DISABLE_INT();    /* 关中断 */

    runtime = g_iRunTime; /* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

    ENABLE_INT();     /* 开中断 */

    return runtime;
}

/*
*********************************************************************************************************
* 函 数 名: SysTick_Handler
* 功能说明: 系统嘀嗒定时器中断服务程序。启动文件中引用了该函数。
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void SysTick_Handler ( void )
{
    SysTick_ISR();
}

void timerRunPer10ms ( void )
{
    #if defined PLATFORM_DEVICE_DEXMO_A
    // mpu9150Get();
    #endif
}
