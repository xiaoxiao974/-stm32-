#include "stm32f10x.h" // Device header
#include "Delay.h"

uint8_t My_System_SE_Flag = 0; // 全局变量，用于标志系统状态
uint8_t sleep_flag = 0;		//睡眠标志位

void _DelayMs(volatile uint16_t xMs)
{
	volatile uint32_t Count = 8000;
	while (xMs--)
	{
		Count = 8000;
		while (Count--)
			;
	}
}

void My_System_SE_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // 开启AFIO的时钟，外部中断必须开启AFIO的时钟

    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); // 将PA3引脚初始化为上拉输入

    /*AFIO选择中断引脚*/
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3); // 将外部中断的3号线映射到GPIOA，即选择PA3为外部中断引脚

    /*EXTI初始化*/
    EXTI_InitTypeDef EXTI_InitStructure;                    // 定义结构体变量
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;              // 选择配置外部中断的3号线
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;               // 指定外部中断线使能
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     // 指定外部中断线为中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 指定外部中断线为下降沿触发
    EXTI_Init(&EXTI_InitStructure);                         // 将结构体变量交给EXTI_Init，配置EXTI外设

    /*NVIC中断分组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 配置NVIC为分组2
                                                    // 即抢占优先级范围：0~3，响应优先级范围：0~3
                                                    // 此分组配置在整个工程中仅需调用一次
                                                    // 若有多个中断，可以把此代码放在main函数内，while循环之前
                                                    // 若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置

    /*NVIC配置*/
    NVIC_InitTypeDef NVIC_InitStructure;                      // 定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;          // 选择配置NVIC的EXTI3线
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 指定NVIC线路使能
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 指定NVIC线路的抢占优先级为3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 指定NVIC线路的响应优先级为3
    NVIC_Init(&NVIC_InitStructure);                           // 将结构体变量交给NVIC_Init，配置NVIC外设
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) == SET) // 判断是否是外部中断3号线触发的中断
    {
		_DelayMs(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0);	//等待按键松手
		_DelayMs(20);											//延时消抖
        

        if(My_System_SE_Flag == 0)
        {
            My_System_SE_Flag = 1; // 系统状态标志位置1
            
        }
        else
        {
            My_System_SE_Flag = 0; // 系统状态标志位置0
            sleep_flag = 1;		//置睡眠标志位
        }

    }
    EXTI_ClearITPendingBit(EXTI_Line3); // 清除外部中断3号线的中断标志位
                                        // 中断标志位必须清除
                                        // 否则中断将连续不断地触发，导致主程序卡死

}


