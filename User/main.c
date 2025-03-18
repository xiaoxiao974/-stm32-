#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "My_System_SE.h"
#include "AD.h"
#include "Motor.h"
#include "Buzzer.h"
#include "MyRTC.h"
#include "Serial.h"
#include <string.h>   

// 模式
typedef enum
{
	MODE_SUPER,	 // 超能模式
	MODE_BALANCE // 平衡模式
} Mode;
Mode pattern = MODE_BALANCE; // 默认设置为平衡模式

uint8_t Addition = 0;

void USA(void); // 串口通讯
void Module_Init(void); // 模块初始化
void IWDG_Init(void); // 看门狗初始化
void GPIO_Configuration(void); // GPIO配置
void Display_Time(void); // 显示时间
void Select_Mode(void); // 模式选择
void Display_Light(void); // 显示光照强度
void Display_Temperature_And_Control_Buzzer(void); // 显示温度并控制蜂鸣器
void Control_Fan_By_Temperature(uint8_t temp); // 根据温度控制风扇
void Handle_Sleep_Mode(void); // 处理睡眠模式

int main(void)
{
	/*模块初始化*/
	Module_Init();
	
	// 看门狗
	IWDG_Init();
	
	// PA0引脚初始化为推挽输出
	GPIO_Configuration();

	while (1)
	{
		if (My_System_SE_Flag == 1)
		{
			// 时间显示
			Display_Time();

			// 模式选择
			Select_Mode();

			GPIO_SetBits(GPIOA, GPIO_Pin_0); // PA0输出高电平

			// 显示光照强度
			Display_Light();

			// 显示温度
			Display_Temperature_And_Control_Buzzer();

			// 串口控制模式
			USA();

			_DelayMs(100);
		}
		else
		{
			Handle_Sleep_Mode();
		}
		IWDG_ReloadCounter(); // 重新加载IWDG计数器的值
	}
}

void USA(void)
{
	if (Serial_RxFlag == 1) // 如果接收到数据包
	{
		/*将收到的数据包与预设的指令对比，以此决定将要执行的操作*/
		if (strcmp(Serial_RxPacket, "MODE_SUPER") == 0) // 如果收到MODE_SUPER指令
		{
			pattern = MODE_SUPER;					// 设置模式为超能模式
			Serial_SendString("MODE_SUPER_OK\r\n"); // 串口回传一个字符串MODE_SUPER_OK
		}
		else if (strcmp(Serial_RxPacket, "MODE_BALANCE") == 0) // 如果收到MODE_BALANCE指令
		{
			pattern = MODE_BALANCE;					  // 设置模式为平衡模式
			Serial_SendString("MODE_BALANCE_OK\r\n"); // 串口回传一个字符串MODE_BALANCE_OK
		}
		else // 上述所有条件均不满足，即收到了未知指令
		{
			Serial_SendString("ERROR_COMMAND\r\n"); // 串口回传一个字符串ERROR_COMMAND
		}

		Serial_RxFlag = 0; // 处理完成后，需要将接收数据包标志位清零，否则将无法接收后续数据包
	}
}

// 模块初始化函数
void Module_Init(void)
{
	OLED_Init();		 // OLED初始化
	My_System_SE_Init(); // 启动按键初始化
	AD_Init();			 // AD初始化
	Motor_Init();		 // 直流电机初始化
	Buzzer_Init();		 // 蜂鸣器初始化
	my_rtc_init();		 // RTC初始化
	Serial_Init();		 // 串口初始化
}

// 看门狗初始化函数
void IWDG_Init(void)
{
	// 使能对IWDG_PR和IWDG_RLR寄存器的写访问
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	// 设置IWDG预分频器值为256
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	// 设置IWDG重装载值为1249（10秒超时）
	IWDG_SetReload(1249);
	// 重新加载IWDG计数器的值
	IWDG_ReloadCounter();
	// 使能IWDG
	IWDG_Enable();
	// 开启停止模式
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // 使能PWR时钟
}

// GPIO配置函数
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA的时钟
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	  // 推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;			  // 选择PA0引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  // 50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  // 初始化GPIOA

	GPIO_ResetBits(GPIOA, GPIO_Pin_0); // PA0输出低电平
}

// 显示时间函数
void Display_Time(void)
{
	my_rtc_get_time();						// 获取时间
	OLED_ShowString(4, 1, "Time:  -  -  "); // 显示时间
	OLED_ShowNum(4, 6, MyRTC_Time[3], 2);	// 显示时
	OLED_ShowNum(4, 9, MyRTC_Time[4], 2);	// 显示分
	OLED_ShowNum(4, 12, MyRTC_Time[5], 2);	// 显示秒
}

// 模式选择函数
void Select_Mode(void)
{
	if (pattern == MODE_BALANCE)
	{
		Addition = 0;
		OLED_ShowString(1, 16, "B"); // 屏幕显示"B"
	}
	else if (pattern == MODE_SUPER)
	{
		Addition = 10;
		OLED_ShowString(1, 16, "S"); // 屏幕显示"S"
	}
}

// 显示光照强度函数
void Display_Light(void)
{
	OLED_ShowString(1, 1, "Light:");
	OLED_ShowNum(1, 8, 100 - AD_Value[0] * 100 / 4096, 3);
	OLED_ShowString(1, 12, "%");
}

// 显示温度并控制蜂鸣器函数
void Display_Temperature_And_Control_Buzzer(void)
{
	uint8_t temp = 95 - 11 * AD_Value[1] / 315;
	OLED_ShowString(2, 1, "Temp:");
	OLED_ShowNum(2, 7, temp, 2); // 1700~2015到36~25
	OLED_ShowString(2, 10, "C");

	// 根据定时中短和温度控制蜂鸣器
	if (temp > 34 && Buzzer_Flag == 1)
	{
		Serial_SendString("Warning!\r\n");		 // 串口发送警告信息

		GPIO_ResetBits(GPIOB, GPIO_Pin_12); // 将PB12引脚设置为低电平，蜂鸣器鸣叫
		Delay_ms(100);						// 延时100ms
		GPIO_SetBits(GPIOB, GPIO_Pin_12);	// 将PB12引脚设置为高电平，蜂鸣器停止
		Delay_ms(100);						// 延时100ms
		GPIO_ResetBits(GPIOB, GPIO_Pin_12); // 将PB12引脚设置为低电平，蜂鸣器鸣叫
		Delay_ms(100);						// 延时100ms
		GPIO_SetBits(GPIOB, GPIO_Pin_12);	// 将PB12引脚设置为高电平，蜂鸣器停止
		Delay_ms(500);						// 延时500ms
		Buzzer_Flag = 0;
	}

	// 根据温度调节风扇转速
	Control_Fan_By_Temperature(temp);
}

// 根据温度控制风扇函数
void Control_Fan_By_Temperature(uint8_t temp)
{
	if (temp > 34)
	{
		Motor_SetSpeed(60 + Addition); // 直流电机
		// 显示转速百分比
		OLED_ShowString(3, 1, "Speed:");
		OLED_ShowNum(3, 8, 60 + Addition, 2);
		OLED_ShowString(3, 10, "%");
	}
	else if (temp > 32 && temp < 34)
	{
		Motor_SetSpeed(50 + Addition); // 直流电机转速
		OLED_ShowString(3, 1, "Speed:");
		OLED_ShowNum(3, 8, 50 + Addition, 2);
		OLED_ShowString(3, 10, "%");
	}
	else if (temp > 28 && temp < 32)
	{
		Motor_SetSpeed(30 + Addition); // 直流电机转速
		OLED_ShowString(3, 1, "Speed:");
		OLED_ShowNum(3, 8, 30 + Addition, 2);
		OLED_ShowString(3, 10, "%");
	}
	else if (temp < 28)
	{
		Motor_SetSpeed(20 + Addition); // 直流电机转速
		OLED_ShowString(3, 1, "Speed:");
		OLED_ShowNum(3, 8, 20 + Addition, 2);
		OLED_ShowString(3, 10, "%");
	}
}

// 处理睡眠模式函数
void Handle_Sleep_Mode(void)
{
	if (sleep_flag == 1)
	{
		OLED_Clear();						  // 清屏
		OLED_ShowString(1, 1, "Sleeping..."); // 显示"Sleeping..."
		Delay_ms(300);						  // 延时
		OLED_Clear();						  // 清屏
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);	  // PA0输出低电平
		Motor_SetSpeed(0);					  // 直流电机停止

		sleep_flag = 0;												  // 清除睡眠标志位
		PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // 进入停止模式    ,系统主频变为8MHz
		SystemInit();												  // 恢复系统时钟为72MHz
		OLED_ShowString(1, 1, "Wake up!");							  // 唤醒后显示"Wake up!"
		Delay_ms(500);												  // 延时1s
		OLED_Clear();												  // 清屏
	}
}
