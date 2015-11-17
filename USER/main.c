
#include "stm32f10x.h"
#include <stdio.h>
#include "BUZZER.h"
#include "USART.h"
#include "RTC.h"
#include "SDIO.h"
#include "ADC.h"
#include "PWM.h"
#include "PCIe.h"
#include "I2C.h"
#include "main.h"
#include "LIS3DH.h"
#include "RGB.h"
#include "Touch.h"
#include "Tim3.h"

void RCC_Config(void);

RCC_ClocksTypeDef RCC_ClockFreq;

unsigned char read_lis3dh_flag = 0;
s16 ACCdata[3] = {0,0,0};
s16 oldACCdata[3] = {0,0,0};
s16 ACCdiff[3] = {0,0,0};

unsigned char i = 0;

unsigned char one_second_flag = 0;

unsigned int buzzer_delay = 0;

unsigned char save_sd_detect = 0;
unsigned char read_sd_detect_flag = 0;
unsigned char sd_detect_change = 0;
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
	Touch_Init();
	Pcie_Gpio_Init();
	Tim3_Init();
	
	RGB_Init();     //RGB ��ʼ��
	RCC_GetClocksFreq(&RCC_ClockFreq);		
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	
	USB2Serial_Init(); 	// ���ڳ�ʼ������
	Pwm_Init();
	Adc_Init();
	I2C_GPIO_Configuration();
	err_code = LIS3DH_Init();
	
	if (NO_ERROR == err_code)
	{
		printf("\r\nLIS3DH Init is succeed! \r\n");
	}
	else
	{
		printf("\r\nLIS3DH Init is failed! \r\n");
	}
	
	RTC_Init(); 		// RTC ��ʼ������


	if(SD_Init() == SD_OK) {
	
		printf ("\r\n����SD��!\r\n");
	}
	else {
		printf("\r\nû�з��� SD ���豸! \r\n");
	}
	printf("\r\n\r\n");
	save_sd_detect = SD_Detect(); //��ʼ��SD������״̬
	
	SysTick_Delay_ms(500);
	TIM_Cmd(TIM1, DISABLE);
	TIM_CtrlPWMOutputs(TIM1, DISABLE);
	while (1)
	{
		if(read_sd_detect_flag){
			
			if (save_sd_detect != SD_Detect()){
				/* �������� */
				TIM_Cmd(TIM1, ENABLE);
				TIM_CtrlPWMOutputs(TIM1, ENABLE);
				sd_detect_change = 1; //SD������״̬�б�
				buzzer_delay = 0;
				if (SD_Detect() != SD_NOT_PRESENT){
						if(SD_Init() == SD_OK) {
							printf ("\r\n����SD��!\r\n");
						}
						else {
							printf("\r\nû�з��� SD ���豸! \r\n");
						}
						printf("\r\n\r\n");
				}
			}
			save_sd_detect = SD_Detect();
			read_sd_detect_flag = 0;
		}
		
		
		Time_Show();	
		Test_Pcie_Gpio();
		Touch_Key_Proc();
		
		if (read_lis3dh_flag){
			Collect_Data(ACCdata);
			for (i=0; i<3; i++){
				if (oldACCdata[i] < ACCdata[i]){
					ACCdiff[i] = ACCdata[i] - oldACCdata[i];
				}
				else{
					ACCdiff[i] = oldACCdata[i] - ACCdata[i];
				}
			}
			RGB_Control(ACCdiff[0]<<2, ACCdiff[1]<<2, ACCdiff[2]<<2);
			for (i=0; i<3; i++){
				oldACCdata[i] = ACCdata[i];
			}
			read_lis3dh_flag = 0;
		}
		if (one_second_flag){
			printf("X=%d, Y=%d, Z=%d\r\n\r\n", ACCdata[1], ACCdata[0], ACCdata[2]);
			Adc_Proc();
			one_second_flag = 0;
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
