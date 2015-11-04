#include "stm32f10x.h"
#include "PWM.h"

/**************************************************************/
//程 序 名： PWM_init()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： pwm初始化  PWMCH1-- GPIOPB8 PWMCH2-- GPIOPB9
//**************************************************************/
void  Pwm_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	uint16_t CCR1_Val = 1000;
	uint16_t PrescalerValue = 0;

	/* GPIOB clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM4 | RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);

	/************************
  PWMCH1-- GPIOPB8 
  PWMCH2-- GPIOPB9
  *************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure);


	/* -----------------------------------------------------------------------
	   TIM4 Frequency = TIM4 counter clock/(ARR + 1)
	   TIM4 Channel1 duty cycle = (TIM4_CCR1/ TIM4_ARR)* 100 = 10%
	   TIM4 Channel2 duty cycle = (TIM4_CCR2/ TIM4_ARR)* 100 = 37.5%
	   TIM4 Channel3 duty cycle = (TIM4_CCR3/ TIM4_ARR)* 100 = 25%
	   TIM4 Channel4 duty cycle = (TIM4_CCR4/ TIM4_ARR)* 100 = 12.5%
	   ----------------------------------------------------------------------- */
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 8000000) - 1; 
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 2000;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; 

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;  
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;        

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);

	TIM_ARRPreloadConfig(TIM4, ENABLE);

	/* TIM1 enable counter */
	TIM_Cmd(TIM4, ENABLE);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
}


