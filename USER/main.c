
#include "stm32f10x.h"
#include <stdio.h>
#include "BUZZER.h"
#include "USART.h"
#include "RTC.h"
#include "SDIO_SD.h"
#include "adc_test.h"
#include "pwm_test.h"
#include "I2C.h"
#include "main.h"
#include "lis3dh_driver.h"
void RCC_Config(void);

RCC_ClocksTypeDef RCC_ClockFreq;

s16 ACCdata[3]={0,0,0};

unsigned char read_flag = 0;

/*******************************************************************************
* Function Name  : SysTick_Delay_ms
* Description    : Inserts a delay time.
* Input          : nTime: specifies the delay time length, millisecond (ms).
* Output         : None.
* Return         : None.
*******************************************************************************/
void SysTick_Delay_ms(u32 nTime)
{
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	// ����������
	while(nTime > SYSTICK_CMP_MS)
	{
		SysTick->LOAD = SysTick_LOAD_RELOAD_Msk;
		SysTick->VAL = 0;							// ��ռ�����
		nTime -= SYSTICK_CMP_MS;
		while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
	}
	SysTick->LOAD = SYSTICK_BASE_MS * nTime;
	SysTick->VAL = 0;							// ��ռ�����
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	// �رռ�����
}


/**************************************************************/
//�� �� ���� main()
//�� �� �ߣ� MingH
//��ڲ����� ��
//����˵���� ������
//**************************************************************/
int main(void)
{
	unsigned char err_code;
	RCC_Config();		// ʱ�ӳ�ʼ������
	Beep_Init();		// ��������ʼ������
	RCC_GetClocksFreq(&RCC_ClockFreq);		
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	
	USB2Serial_Init(); 	// ���ڳ�ʼ������
	Pwm_Init();
	Adc_Init();
	I2C_GPIO_Configuration();
	err_code = LIS3DH_Init();
	if (NO_ERROR == err_code)
	{
		printf("\r\n LIS3DH Init is succeed! \r\n");
	}
	else
	{
		printf("\r\n LIS3DH Init is failed! \r\n");
	}
	
//	// �����ȴ�����ո�������
//	if (getchar() == 0x20) {
//		printf("\r\n�����Ĳ�����Ŀ: \r\n1. RTC \r\n2. SDIO \r\n3. BEEP\r\n");
//		printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
//		printf("������ RTC ʱ����! \r\n");
		RTC_Init(); 		// RTC ��ʼ������
		TIM_Cmd(TIM1, DISABLE);
		TIM_CtrlPWMOutputs(TIM1, DISABLE);
//	}
	

	if(SD_Init() == SD_OK) {
	
		printf ("\r\n����SD��!\r\n");
	}
	else {
		printf("û�з��� SD ���豸! \r\n");
	}

	
	printf("\r\n******************\r\n\r\n");

	while (1)
	{
		// ����ֻ������ѯ
		if (SD_Detect() != SD_NOT_PRESENT) {
			// G_LED ��ʾ SD ������
			GPIO_ResetBits(GPIOA, GPIO_Pin_2);   
		}
		else {
			GPIO_SetBits(GPIOA, GPIO_Pin_2);
		}
		
		Time_Show();	
		if (read_flag)
		{
			Adc_Proc();
			
			Collect_Data(ACCdata);
			printf("X=%d, Y=%d, Z=%d\r\n\r\n", ACCdata[1], ACCdata[0], ACCdata[2]);
			read_flag = 0;
		}
	}
}


/**************************************************************/
//�� �� ���� RCC_Config()
//�� �� �ߣ� Haichao.Xie
//��ڲ����� ��
//����˵���� ϵͳʱ������
//**************************************************************/
void RCC_Config(void)
{
	ErrorStatus HSEStartUpStatus;	//����ṹ��
	/* RCC system reset(for debug purpose)������ RCC�Ĵ�������Ϊȱʡֵ */
	RCC_DeInit();
	/* Enable HSE �����ⲿ���پ���HSE��*/
	RCC_HSEConfig(RCC_HSE_ON);
	/* Wait till HSE is ready �ȴ� HSE ����*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer Ԥȡָ����ʹ��*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state ���ô�����ʱֵ*/
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK ���� AHB ʱ�ӣ�HCLK��*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK ���ø��� AHB ʱ�ӣ�PCLK2��*/
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* PCLK1 = HCLK/1 ���õ��� AHB ʱ�ӣ�PCLK1��*/
		RCC_PCLK1Config(RCC_HCLK_Div1);

		/* PLLCLK = 12MHz * 6 = 72 MHz ���� PLL ʱ��Դ����Ƶϵ��*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);

		/* Enable PLL ʹ�ܻ���ʧ�� PLL*/
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready �ȴ�ָ���� RCC ��־λ���óɹ� �ȴ�PLL��ʼ���ɹ�*/
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		/* Select PLL as system clock source ����ϵͳʱ�ӣ�SYSCLK�� ����PLLΪϵͳʱ��Դ*/
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source �ȴ�PLL�ɹ�������ϵͳʱ�ӵ�ʱ��Դ*/
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
}
