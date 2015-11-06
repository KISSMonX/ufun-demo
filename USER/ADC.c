#include "stm32f10x.h"
#include <stdio.h>
#include "USART.h"
#include "ADC.h"


////////////////////////////////////////////////////////////////
#define ARRAYSIZE 2*4   //2通道*4字节
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
volatile uint16_t ADC_values[ARRAYSIZE];

/**************************************************************/
//程 序 名： ADC_Config()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： ADC配置
//**************************************************************/
void ADC_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 ;   // PC2 -> AD_SIG1  PC3-> AD_SIG2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	//ADC1 configuration
	//select continuous conversion mode
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	//We will convert multiple channels
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	//we will convert one time
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//
	//select no external triggering
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	//right 12-bit data alignment in ADC data register
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	//2 channels conversion
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	//load structure values to control and status registers
	ADC_Init(ADC1, &ADC_InitStructure);

	//configure each channel
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_41Cycles5);
	//Enable ADC1
	ADC_Cmd(ADC1, ENABLE);
	//enable DMA for ADC
	ADC_DMACmd(ADC1, ENABLE);
	//Enable ADC1 reset calibration register
	ADC_ResetCalibration(ADC1);
	//Check the end of ADC1 reset calibration register
	while (ADC_GetResetCalibrationStatus(ADC1));
	//Start ADC1 calibration
	ADC_StartCalibration(ADC1);
	//Check the end of ADC1 calibration
	while (ADC_GetCalibrationStatus(ADC1));
}

/**************************************************************/
//程 序 名： DMA_Config()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： DMA配置
//**************************************************************/
void DMA_Config(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	//enable DMA1 clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//reset DMA1 channe1 to default values;
	DMA_DeInit(DMA1_Channel1);
	//channel will be used for memory to memory transfer
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	//setting normal mode (non circular)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	//medium priority
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	//source and destination data size word=32bit
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	//automatic memory destination increment enable.
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	//source address increment disable
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//Location assigned to peripheral register will be source
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	//chunk of data to be transfered
	DMA_InitStructure.DMA_BufferSize = ARRAYSIZE;
	//source and destination start addresses
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_values;
	//send values to DMA registers
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	//Enable DMA1 Channel transfer
	DMA_Cmd(DMA1_Channel1, ENABLE);
	//Start ADC1 Software Conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	// Enable DMA1 Channel Transfer Complete interrupt
	/*
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	*/

	//Enable DMA1 channel IRQ Channel
	/*
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	*/
}
/**************************************************************/
//程 序 名： Adc_Init()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： ADC所有外设初始化
//**************************************************************/
void  Adc_Init(void)
{
	ADC_Config();
	DMA_Config();
}

/**************************************************************/
//程 序 名： Adc_Proc()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： Adc处理函数 ad采样转换为pwm值调节RGB_B RGB_R的亮度
//**************************************************************/
void  Adc_Proc(void)
{
	uint8_t pwmwave = 0; //ad采样转换为pwm值调节RGB_B RGB_R的亮度
	uint8_t index;
	uint16_t value[2];
	for (index = 0; index < 2; index++) { //2 channel
		value[index] = (uint16_t)((ADC_values[index] + ADC_values[index + 2] + ADC_values[index + 4] + ADC_values[index + 6]) / 4);
	}
	pwmwave = value[0] >> 8;
	printf("The AD_SIG1 value is: %d\r\n", pwmwave);

	pwmwave = value[1] >> 8;
	printf("The AD_SIG2 value is: %d\r\n", pwmwave);

}
