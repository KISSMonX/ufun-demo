/**
  ******************************************************************************
  * @file    RTC/Calendar/main.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  */
#include "RTC.h"


#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */

/* Private variables ---------------------------------------------------------*/
__IO uint32_t TimeDisplay = 0;


/**
 * @brief  Configures the nested vectored interrupt controller.
 * @param  None
 * @retval None
 */
void RTC_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/**
 * @brief  Configures the RTC.
 * @param  None
 * @retval None
 */
void RTC_Configuration(void)
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

/**************************************************************/
//程 序 名： RTC_LED_Init
//开 发 者： MingH
//入口参数： 无
//功能说明： RTC 秒指示 LED(红色和绿色交替), IO 口配置
//**************************************************************/
void RTC_LED_Init(void) 
{
	GPIO_InitTypeDef GPIO_InitStructure;


	/* GPIOA clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**************************************************************/
//程 序 名： RTC_LED_Toggle
//开 发 者： MingH
//入口参数： 无
//功能说明： RTC 秒指示 LED(红色和绿色交替), 交替翻转
//**************************************************************/
void RTC_LED_Toggle(void)
{
	static  char status = 0;
	
	if (status) {
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		status = 0;
	}
	else {
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
		status = 1;
	}
}


/**
 * @brief  Returns the time entered by user, using Hyperterminal.
 * @param  None
 * @retval Current time RTC counter value
 */
uint32_t Time_Regulate(void)
{
	uint32_t Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;

	printf("\r\n==============Time Settings===================");
	printf("\r\n  Please Set Hours");

	while (Tmp_HH == 0xFF)
	{
		Tmp_HH = USART_Scanf(23);
	}
	printf(":  %d", Tmp_HH);
	printf("\r\n  Please Set Minutes");
	while (Tmp_MM == 0xFF)
	{
		Tmp_MM = USART_Scanf(59);
	}
	printf(":  %d", Tmp_MM);
	printf("\r\n  Please Set Seconds");
	while (Tmp_SS == 0xFF)
	{
		Tmp_SS = USART_Scanf(59);
	}
	printf(":  %d\r\n\r\n", Tmp_SS);

	/* Return the value to store in RTC counter register */
	return((Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS));
}

/**
 * @brief  Adjusts time.
 * @param  None
 * @retval None
 */
void Time_Adjust(void)
{
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Change the current time */
	RTC_SetCounter(Time_Regulate());
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

/**
 * @brief  Displays the current time.
 * @param  TimeVar: RTC counter value.
 * @retval None
 */
void Time_Display(uint32_t TimeVar)
{
	uint32_t THH = 0, TMM = 0, TSS = 0;

	/* Reset RTC Counter when Time is 23:59:59 */
	if (RTC_GetCounter() == 0x0001517F)
	{
		RTC_SetCounter(0x0);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}

	/* Compute  hours */
	THH = TimeVar / 3600;
	/* Compute minutes */
	TMM = (TimeVar % 3600) / 60;
	/* Compute seconds */
	TSS = (TimeVar % 3600) % 60;

	printf("Current Time: %0.2d:%0.2d:%0.2d\r", THH, TMM, TSS);
}

/**
 * @brief  Shows the current time (HH:MM:SS) on the Hyperterminal.
 * @param  None
 * @retval None
 */
void Time_Show(void)
{
	/* If 1s has been elapsed */
	if (TimeDisplay == 1)
	{
		/* Display current time */
		Time_Display(RTC_GetCounter());
		TimeDisplay = 0;
	}
}


/**************************************************************/
//程 序 名： RTC_Init
//开 发 者： MingH
//入口参数： 无
//功能说明： RTC 相关寄存器配置初始化, 外部调用
//**************************************************************/
void RTC_Init(void)
{
	RTC_NVIC_Configuration(); // RTC 中断配置
	RTC_LED_Init();
	
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		   the first time the program is executed) */

		printf("\r\n\n RTC not yet configured....");

		/* RTC Configuration */
		RTC_Configuration();

		printf("\r\n RTC configured....");

		/* Adjust time by values entered by the user on the hyperterminal */
		Time_Adjust();

		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		/* Check if the Power On Reset flag is set */
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{
			printf("\r\n\n Power On Reset occurred....");
		}
		/* Check if the Pin Reset flag is set */
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{
			printf("\r\n\n External Reset occurred....");
		}

		printf("\r\n No need to configure RTC....");
		/* Wait for RTC registers synchronization */
		RTC_WaitForSynchro();

		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}

#ifdef RTCClockOutput_Enable
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Disable the Tamper Pin */
	BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
				      functionality must be disabled */

	/* Enable RTC Clock Output on Tamper Pin */
	BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
#endif

	/* Clear reset flags */
	RCC_ClearFlag();
}


