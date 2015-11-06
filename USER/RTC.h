

#ifndef __RTC_H__ 
#define __RTC_H__

#include "stm32f10x.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include <stdio.h>
#include "USART.h"

void RTC_Init(void);
void RTC_LED_Toggle(void);
void Time_Show(void);

#endif
