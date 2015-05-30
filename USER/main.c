
/**************************************************************/
//�� �� �ţ� Ver 1.0
//�� �� �ߣ� MingH
//�������ڣ� 2015-5-20
//����˵���� SD Card/RTC/Beep
//**************************************************************/

#include "stm32f10x.h"
#include <stdio.h>
#include "BUZZER.h"
#include "USART.h"
#include "RTC.h"
#include "SDIO_SD.h"



SD_Error Status = SD_OK;

	
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




/**************************************************************/
//�� �� ���� main()
//�� �� �ߣ� MingH
//��ڲ����� ��
//����˵���� ������
//**************************************************************/
int main(void)
{
	RCC_Config();		// ʱ�ӳ�ʼ������
	Beep_Init();		// ��������ʼ������
	USB2Serial_Init(); 	// ���ڳ�ʼ������
	
		
	// �����ȴ�����ո�������
	if (getchar() == 0x20) {
		printf("\r\n�����Ĳ�����Ŀ: \r\n1. RTC \r\n2. SDIO \r\n3. BEEP\r\n");
		printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
		printf("������ RTC ʱ����! \r\n");
		RTC_Init(); 		// RTC ��ʼ������
	}
	
	
	/*------------------------------ SD Init ---------------------------------- */
		
	printf("\r\n��⵽�Ѿ����� SD ��......\r\n");
	printf("���ڳ�ʼ�� SD ��......\r\n");
	printf("���ڶ�ȡ SD ����Ϣ: \r\n\r\n");

	if((Status = SD_Init()) == SD_OK) {
	
		printf ("\r\nSD ����ʼ���ɹ�!\r\n");
	}
	else {
		printf("SD ����ʼ��ʧ��! \r\n");
	}

	
	printf("\r\n******************\r\n\r\n");

	while (1)
	{
		// ����ֻ������ѯ
		if (SD_Detect() != SD_NOT_PRESENT) {
			// ��ɫ LED ��ʾ SD ������
			GPIO_ResetBits(GPIOA, GPIO_Pin_0);   
		}
		else {
			
			GPIO_SetBits(GPIOA, GPIO_Pin_0);
		}
	
		Time_Show();		
	}
}

