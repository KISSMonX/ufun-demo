
#ifndef I2C_HEADER
#define I2C_HEADER

/* 包含头文件 *****************************************************************/
#include "main.h"
#include "stm32f10x.h"

/* 类型声明 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
// I2C Basic Operations
#define ENABLE_I2C_DELAY	1
#define I2C_DELAY			2
// 编译选项: Optimization<default>, Optimize for time_OFF
// ENABLE_I2C_DELAY=0时, 主频72MHz, SCL的频率约为4MHz, 于2011/12/22测试
// I2C_DELAY=0时, 主频72MHz, SCL的频率约为1333kHz, 于2011/12/22测试
// I2C_DELAY=1时, 主频72MHz, SCL的频率约为1263kHz, 于2011/12/22测试
// I2C_DELAY=2时, 主频72MHz, SCL的频率约为828kHz, 于2011/12/22测试
// I2C_DELAY=3时, 主频72MHz, SCL的频率约为666kHz, 于2011/12/22测试
// I2C_DELAY=4时, 主频72MHz, SCL的频率约为558kHz, 于2011/12/22测试
// I2C_DELAY=5时, 主频72MHz, SCL的频率约为480kHz, 于2011/12/22测试
// I2C_DELAY=6时, 主频72MHz, SCL的频率约为421kHz, 于2011/12/22测试
// I2C_DELAY=7时, 主频72MHz, SCL的频率约为375kHz, 于2011/12/22测试
// 综上可得换算式 SCL = 2880 / (I2C_DELAY + 1)kHz, 许永鹏于2011/12/22提供

// 编译选项: Optimization<default>, Optimize for time_OFF
// ENABLE_I2C_DELAY=0时, 主频36MHz, SCL的频率约为2MHz, 于2012/05/22测试
// I2C_DELAY=0时, 主频36MHz, SCL的频率约为750kHz, 于2012/05/22测试
// I2C_DELAY=1时, 主频36MHz, SCL的频率约为558kHz, 于2012/05/22测试
// I2C_DELAY=2时, 主频36MHz, SCL的频率约为400kHz, 于2012/05/22测试
// I2C_DELAY=3时, 主频36MHz, SCL的频率约为300kHz, 于2012/05/22测试
// I2C_DELAY=4时, 主频36MHz, SCL的频率约为250kHz, 于2012/05/22测试
// I2C_DELAY=5时, 主频36MHz, SCL的频率约为210kHz, 于2012/05/22测试
// I2C_DELAY=6时, 主频36MHz, SCL的频率约为180kHz, 于2012/05/22测试
// I2C_DELAY=7时, 主频36MHz, SCL的频率约为158kHz, 于2012/05/22测试
// 综上可得换算式 SCL = 1200 / (I2C_DELAY + 1)kHz, 许永鹏于2012/05/22提供 
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

/************************************文件结束*********************************/
