#include "stm32f10x.h"
#include <time.h>

uint16_t MyRTC_Time[] =    {2025,3,13,13,24,55}  ; //设置RTC时间

void my_rtc_set_time(void)
{
    time_t time_cnt;
    struct tm time_data;

    time_data.tm_year = MyRTC_Time[0]-1900;
    time_data.tm_mon = MyRTC_Time[1]-1;
    time_data.tm_mday = MyRTC_Time[2];
    time_data.tm_hour = MyRTC_Time[3];  
    time_data.tm_min = MyRTC_Time[4];
    time_data.tm_sec = MyRTC_Time[5];

    time_cnt = mktime(&time_data)-8*60*60; //将时间数据转换为时间戳，并减去8小时，因为RTC时间是UTC时间，需要转换为本地时间

    RTC_SetCounter(time_cnt); //设置RTC时间
    RTC_WaitForLastTask(); //等待上次任务完成
}

void my_rtc_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//使能PWR和BKP时钟
    PWR_BackupAccessCmd(ENABLE);//使能RTC备份寄存器访问

    //判断备用电源是否存在
    if(BKP_ReadBackupRegister(BKP_DR1) != 0x1234)
    {    
        RCC_LSEConfig(RCC_LSE_ON); //使能外部晶振
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET); //等待外部晶振稳定
    
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //选择LSE作为RTC时钟源
        RCC_RTCCLKCmd(ENABLE); //使能RTC时钟  
    
        RTC_WaitForSynchro(); //等待RTC时钟稳定
        RTC_WaitForLastTask(); //等待上次任务完成
    
        RTC_SetPrescaler(32767); //设置RTC分频因子为32767，即1秒钟计数一次
        RTC_WaitForLastTask(); //等待上次任务完成
    
        my_rtc_set_time(); //设置RTC时间

        BKP_WriteBackupRegister(BKP_DR1, 0x1234); //向指定的后备寄存器中写入数据
    
    }

    else{
        RTC_WaitForSynchro(); //等待RTC时钟稳定
        RTC_WaitForLastTask(); //等待上次任务完成
    }

    
}

void my_rtc_get_time(void)
{
    time_t time_cnt;
    struct tm *time_data;

    time_cnt = RTC_GetCounter()+8*60*60; //获取RTC时间，并加上8小时，因为RTC时间是UTC时间，需要转换为本地时间
    time_data = localtime(&time_cnt); //将时间戳转换为时间数据

    MyRTC_Time[0] = time_data->tm_year+1900;
    MyRTC_Time[1] = time_data->tm_mon+1;
    MyRTC_Time[2] = time_data->tm_mday;
    MyRTC_Time[3] = time_data->tm_hour;
    MyRTC_Time[4] = time_data->tm_min;
    MyRTC_Time[5] = time_data->tm_sec;
}
