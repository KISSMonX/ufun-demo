#include "stm32f10x.h"
#include "Touch.h"

static void Touch_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //GPIO_Mode_Out_PP;// GPIO_Mode_AF_PP
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* GPIOA clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
}

void Control_Touch_LED(unsigned char value)
{
	if (value) {	
		GPIO_ResetBits(GPIOA, GPIO_Pin_3);
	}
	else {
		GPIO_SetBits(GPIOA, GPIO_Pin_3);
	}
}


//KEY0 PC5 KEY1 PC4 KEY2 PB3 KEY3 PB4
void Touch_Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);       
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);      

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC,&GPIO_InitStructure);
	
}

void Touch_Init(void)
{
	Touch_LED_Init();
	Touch_Key_Init();
}

void Touch_Key_Proc(void)
{
	if ((key0 == 1) || (key1 == 1) || (key2 == 1) || (key3 == 1))
		Control_Touch_LED(1);
	else
		Control_Touch_LED(0);
}

