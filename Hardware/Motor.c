#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    //改为GPIOB的时钟
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;  //改为PB10和PB11
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);                    //改为GPIOB
    
    PWM_Init();                                                 //初始化直流电机的底层PWM
}

/**
  * 函    数：直流电机设置速度
  * 参    数：Speed 要设置的速度，范围：-100~100
  * 返 回 值：无
  */
void Motor_SetSpeed(int8_t Speed)
{
    if (Speed >= 0)                            //如果设置正转的速度值
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_10);    //改为PB10
        GPIO_ResetBits(GPIOB, GPIO_Pin_11);    //改为PB11
        PWM_SetCompare1(Speed);                //PWM设置为速度值
    }
    else                                    //否则，即设置反转的速度值
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_10);    //改为PB10
        GPIO_SetBits(GPIOB, GPIO_Pin_11);    //改为PB11
        PWM_SetCompare1(-Speed);            //PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
    }
}
