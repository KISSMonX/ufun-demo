#include "stm32f10x.h"
#include "RGB.h"



void RGB_Init(void)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  	TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure; 			

	//TIM2 clock enable 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);          
	
	
	TIM_TimeBaseStructure.TIM_Period 		        = 1000;	//1ms
	TIM_TimeBaseStructure.TIM_Prescaler         = 72 - 1;	
	TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up; 
	TIM_TimeBaseStructure.TIM_ClockDivision 	  = 0;  
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);


	TIM_OCInitStructure.TIM_OCMode 		    = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState 	= TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity 	  = TIM_OCPolarity_High;  //TIM_OCPolarity_High
	TIM_OCInitStructure.TIM_OCNPolarity 	= TIM_OCPolarity_Low;  
	TIM_OCInitStructure.TIM_OCIdleState 	= TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState 	= TIM_OCIdleState_Reset;

	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  
	
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);  
	
	
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  
	
	
	TIM_ARRPreloadConfig(TIM2, ENABLE);

	TIM_Cmd(TIM2, ENABLE);
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
	
}
	


void RGB_Control(unsigned int r_pwm, unsigned int g_pwm, unsigned int b_pwm)
{
	 if (r_pwm >= 1000)
		 r_pwm = 1000;
	 if (g_pwm >= 1000)
		 g_pwm = 1000;
	 if (b_pwm >= 1000)
		 b_pwm = 1000;
	 
	 TIM2->CCR1 = g_pwm;  //b_pwm

	 TIM2->CCR2 = r_pwm;
	
	 TIM2->CCR3 = b_pwm;  //g_pwm
}
