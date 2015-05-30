
/**************************************************************/
//�� �� �ţ� Ver 1.0
//�� �� �ߣ� MingH
//�������ڣ� 2015-5-20
//����˵���� ��Դ���������Գ���, ��Ӧ�忨 IO ��: GPA8
//           г��Ƶ�� 2.6KHz
//**************************************************************/
#include <stm32f10x.h>
#include "BUZZER.h"


/**************************************************************/
//����˵���� ��������ʼ������(IO��, ��ʱ��1)
//��ڲ����� ��
//����ֵ  :  ��
//**************************************************************/
void  Beep_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	uint16_t CCR1_Val = 1538;
	uint16_t PrescalerValue = 0;

	/* GPIOA clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE);

	/* BEEP -- GPIOPA8 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* -----------------------------------------------------------------------
	   TIM1 Frequency = TIM1 counter clock/(ARR + 1)
	   TIM1 Channel1 duty cycle = (TIM1_CCR1/ TIM1_ARR)* 100 = 10%
	   TIM1 Channel2 duty cycle = (TIM1_CCR2/ TIM1_ARR)* 100 = 37.5%
	   TIM1 Channel3 duty cycle = (TIM1_CCR3/ TIM1_ARR)* 100 = 25%
	   TIM1 Channel4 duty cycle = (TIM1_CCR4/ TIM1_ARR)* 100 = 12.5%
	   ----------------------------------------------------------------------- */
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 8000000) - 1; 
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 3076;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; 

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;  
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;        

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);

	TIM_ARRPreloadConfig(TIM1, ENABLE);

	/* TIM1 enable counter */
	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

}


