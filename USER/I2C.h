
#ifndef I2C_HEADER
#define I2C_HEADER

/* ����ͷ�ļ� *****************************************************************/
#include "main.h"
#include "stm32f10x.h"

/* �������� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/
// I2C Basic Operations
#define ENABLE_I2C_DELAY	1
#define I2C_DELAY			2
// ����ѡ��: Optimization<default>, Optimize for time_OFF
// ENABLE_I2C_DELAY=0ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ4MHz, ��2011/12/22����
// I2C_DELAY=0ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ1333kHz, ��2011/12/22����
// I2C_DELAY=1ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ1263kHz, ��2011/12/22����
// I2C_DELAY=2ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ828kHz, ��2011/12/22����
// I2C_DELAY=3ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ666kHz, ��2011/12/22����
// I2C_DELAY=4ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ558kHz, ��2011/12/22����
// I2C_DELAY=5ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ480kHz, ��2011/12/22����
// I2C_DELAY=6ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ421kHz, ��2011/12/22����
// I2C_DELAY=7ʱ, ��Ƶ72MHz, SCL��Ƶ��ԼΪ375kHz, ��2011/12/22����
// ���Ͽɵû���ʽ SCL = 2880 / (I2C_DELAY + 1)kHz, ��������2011/12/22�ṩ

// ����ѡ��: Optimization<default>, Optimize for time_OFF
// ENABLE_I2C_DELAY=0ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ2MHz, ��2012/05/22����
// I2C_DELAY=0ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ750kHz, ��2012/05/22����
// I2C_DELAY=1ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ558kHz, ��2012/05/22����
// I2C_DELAY=2ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ400kHz, ��2012/05/22����
// I2C_DELAY=3ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ300kHz, ��2012/05/22����
// I2C_DELAY=4ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ250kHz, ��2012/05/22����
// I2C_DELAY=5ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ210kHz, ��2012/05/22����
// I2C_DELAY=6ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ180kHz, ��2012/05/22����
// I2C_DELAY=7ʱ, ��Ƶ36MHz, SCL��Ƶ��ԼΪ158kHz, ��2012/05/22����
// ���Ͽɵû���ʽ SCL = 1200 / (I2C_DELAY + 1)kHz, ��������2012/05/22�ṩ 
#define SCL_H				  GPIOB->BSRR = SCL
#define SCL_L				  GPIOB->BRR  = SCL  
#define SDA_H				  GPIOB->BSRR = SDA
#define SDA_L				  GPIOB->BRR  = SDA
#define SCL_read			GPIOB->IDR  & SCL
#define SDA_read			GPIOB->IDR  & SDA

/***************I2C*******************/
void I2C_delay(void);
Bool I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
Bool I2C_WaitAck(void); 
u8 I2C_ReceiveByte(void);
void I2C_SendByte(u8); 
u8 ReadI2C(u8 address, u8 WriteAddress);
void WriteI2C(u8 ch, u8 address, u8 WriteAddress);
void I2C_BufferRead(u8* pBuffer, u8 HARD_ADDRESS, u8 ReadAddr, u16 NumByteToRead); 
void I2C_ByteWrite(u8* pBuffer, u8 HARD_ADDRESS, u8 WriteAddr);

void I2C_GPIO_Configuration(void);

#endif

/************************************�ļ�����*********************************/
