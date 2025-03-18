#ifndef MYRTC_H_
#define MYRTC_H_

extern uint16_t MyRTC_Time[]; //设置RTC时间
void my_rtc_set_time(void);
void my_rtc_init(void);
void my_rtc_get_time(void);

#endif /* MYRTC_H_ */
