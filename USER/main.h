
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {FALSE = 0, TRUE = !FALSE} Bool;
typedef enum {IDLE_STATE=0, RUNNING_STATE=1, PAUSE_STATE=2, CHANGE_CONFIG_STATE=3} StateType;
typedef struct _Configuration_type
{
	s16 SAMPLE_RATE;
	s16 ACC_RANGE;
	s16 ACC_ODR;
	s16 GYR_RANGE;
	s16 GYR_ODR;
	s16 MAG_RANGE;
	s16 MAG_ODR;
} Configuration_type;

typedef enum
{
	NO_ERROR			= 	0,
	CONFIG_ERROR_CHK	=	1,
	CONFIG_ERROR_TIM	=	2,
	CONFIG_ERROR_ACC	=	3,
	CONFIG_ERROR_GYR	=	4,
	CONFIG_ERROR_MAG	=	5,
	FLASH_ERROR_READ	=	6,
	FLASH_ERROR_WRITE	=	7,
	SAMPLE_RATE_ERROR	=	10,
	ACC_RANGE_ERROR		=	11,
	ACC_ODR_ERROR		=	12,
	GYR_RANGE_ERROR		=	13,
	GYR_ODR_ERROR		=	14,
	MAG_RANGE_ERROR		=	15,
	MAG_ODR_ERROR		=	16,
} Error_type;

/* Exported constants --------------------------------------------------------*/
#define RCC_PLLMul_Config	RCC_PLLMul_9	// 車?HSI?迄?邦∩?米?米?℅?∩車㊣??米?迄那y那?4MHz㏒?3?辰?∩?㊣??米那y?赤那?MCU?¯那㊣?車
#define SENSOR_DATA_SIZE	20	// ??∩?2谷?‘?迄﹞⊿?赤米?那y?Y～邦∩車D?
#define FLASH_SIZE			64	// ?迄?YMCU米?Flash豕Y芍??y豕﹞谷豕??
#define NODE_ADDRESS		'A'	// ?迆米?D辰o?1足?“?a'A'
//#define ENABLE_FABRICATE	0	// ?㊣?足那y?Y㏒?車?車迆米¯那?
//#define ENABLE_AVR_FILTER	0	// 那1?邦AVR??2“?¯
//#define ENABLE_FIR_FILTER	0	// 那1?邦FIR??2“?¯
#define ENABLE_CALIBRATION	0	// 那1?邦D㏒℅?㏒?D豕???邦辰??¯??DD那米?那2?那y2a芍?㏒?米那?¯?谷辰?車D3ㄓ足芍1??a??﹞t??㏒??那?谷??℅谷?‘米那?¯ 
#define ENABLE_FLASH_WRITE	0	// D∩豕?D㏒℅?2?那y那㊣那1車?辰?∩?㏒???o車1?㊣?
#define ENABLE_FLASH_READ	0	// 車?ENABLE_CALIBRATION??o?那1車?㏒???豕?Flash?D米?D㏒℅?2?那y
#define ENABLE_WIRE_UART	1	// 那1?邦車D??∩??迆
//#define ENABLE_BT_UART		1	// 那1?邦角??角∩??迆
#define ENABLE_UART_INT		1	// Use UART interrupt to receive characters
//#define ENABLE_CONFIG_BT	0	// ????角??角?㏒?谷
#define ENABLE_CONFIG_PARA	0	// ??????豕?2?那y
#define FLASH_ADDR			0x0800FC00	// Page 63
#define CONFIG_MAX_LENGTH	100
#define CONFIG_ITERMS		7
#define SAMPLE_RATE_DEFAULT	50
#define ACC_RANGE_DEFAULT	8000
#define ACC_ODR_DEFAULT		50
#define GYR_RANGE_DEFAULT	2000
#define GYR_ODR_DEFAULT		100
#define MAG_RANGE_DEFAULT	1200
#define MAG_ODR_DEFAULT		75

// PORT A
//#define UART2_TXD			GPIO_Pin_2
//#define UART2_RXD			GPIO_Pin_3
#define UART1_TXD			GPIO_Pin_9
#define UART1_RXD			GPIO_Pin_10

// PORT B
#define SCL					GPIO_Pin_6
#define SDA					GPIO_Pin_7
#define LED1				GPIO_Pin_0

// Peripherals Map
#define USART_Wire			USART1
//#define USART_BT			USART2
#define USART_Wire_IRQn		USART1_IRQn
//#define USART_BT_IRQn		USART2_IRQn

// DMA Channels
#define USART1_DMA_CHANNEL_TX	DMA1_Channel4
#define USART2_DMA_CHANNEL_TX	DMA1_Channel7

/* Exported macro ------------------------------------------------------------*/
// Basic IO
#define LED1_ON				GPIO_SetBits(GPIOA, LED1)
#define LED1_OFF			GPIO_ResetBits(GPIOA, LED1)

// SYSTICK
#define SYSTICK_BASE_MS		(RCC_ClockFreq.HCLK_Frequency / 1000)
#define SYSTICK_BASE_US		(RCC_ClockFreq.HCLK_Frequency / 1000 / 1000)
#define SYSTICK_CMP_MS		(1<<24) / SYSTICK_BASE_MS
#define SYSTICK_CMP_US		(1<<24) / SYSTICK_BASE_US

/* Exported functions --------------------------------------------------------*/
void SysTick_Delay_ms(u32 nTime);

#endif
/***********************************???t?芍那?***********************************/
