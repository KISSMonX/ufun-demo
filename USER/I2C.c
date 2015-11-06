
/* 包含头文件 *****************************************************************/
#include "main.h"
#include "I2C.h"


void I2C_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure I2C2 pins: SCL and SDA -------------------*/
	GPIO_InitStructure.GPIO_Pin = SCL | SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	// 配置成开漏输出，GPIO模拟I2C
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}


void I2C_delay(void)
{
	u8 i = I2C_DELAY;	// 在72MHz主频下，经测试最低到2还能写入
	while (i) { i--; }
}

Bool I2C_Start(void)
{
	SDA_H; SCL_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SDA_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	return TRUE;
}

void I2C_Stop(void)
{
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SDA_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SDA_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
}

void I2C_Ack(void)
{
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SDA_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
}

void I2C_NoAck(void)
{
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SDA_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
}

Bool I2C_WaitAck(void)	// 返回为1有ACK，为0无ACK
{
	SCL_L;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SDA_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_H;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	SCL_L;
	if (SDA_read) { return FALSE; }
	else { return TRUE; }
}

u8 I2C_ReceiveByte(void)	// 数据从高位到低位
{
	u8 i = 8, ReceiveByte = 0;

	SDA_H;
	while (i--) {
		ReceiveByte <<= 1;
		SCL_L;
#if ENABLE_I2C_DELAY
		I2C_delay();
#endif
		SCL_H;
#if ENABLE_I2C_DELAY
		I2C_delay();
#endif
		if (SDA_read) { ReceiveByte |= 0x01; }
	}
	SCL_L;

	return ReceiveByte;
}

void I2C_SendByte(u8 SendByte)	// 数据从高位到低位
{
	u8 i = 8;

	while (i--) {
		SCL_L;
#if ENABLE_I2C_DELAY
		I2C_delay();
#endif
		if (SendByte & 0x80) { SDA_H; }
		else { SDA_L; }
		SendByte <<= 1;
#if ENABLE_I2C_DELAY
		I2C_delay();
#endif
		SCL_H;
#if ENABLE_I2C_DELAY
		I2C_delay();
#endif
	}
	SCL_L;
}

u8 ReadI2C(u8 address, u8 WriteAddress)	// 读出1串数据
{
	u8 ch;				// 定义存储读出数据的临时变量;

	I2C_Start();		// 启动总线，开始传输数据;
	I2C_SendByte(WriteAddress);
	I2C_WaitAck();		// 发送从器件硬件地址;
	I2C_SendByte(address);
	I2C_WaitAck();		// 发送从器件内部数据存储器的地址;
	I2C_Start();		// 重新启动总线，开始传输数据;
	I2C_SendByte(WriteAddress + 1);
	I2C_WaitAck();		// 发送从器件内部数据存储器的地址;
	ch = I2C_ReceiveByte();
	I2C_NoAck();		// 将读出的一个字节数据存入临时变量，发送非应答位;
	I2C_Stop();			// 发送停止信号，释放总路线;

	return (ch);
}

void WriteI2C(u8 ch, u8 address, u8 WriteAddress)
{
	I2C_Start();			// 启动总线，开始传输数据;
	I2C_SendByte(WriteAddress);			I2C_WaitAck();
	//while(TestAck());		// 发送从器件硬件地址;
	I2C_SendByte(address);				I2C_WaitAck();
	//while(TestAck());		// 发送从器件存储器字节地址;
	I2C_SendByte(ch);					I2C_WaitAck();
	//while(TestAck());		// 发送数据;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	I2C_Stop();				// 发送停止位，发送数据结束;
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
}

/*******************************************************************************
* Function Name  : I2C_EE_BufferRead
* Description    : Reads a block of data from the EEPROM.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the EEPROM.
*                  - ReadAddr : EEPROM's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the EEPROM.
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_BufferRead(u8 *pBuffer, u8 HARD_ADDRESS, u8 ReadAddr, u16 NumByteToRead)
{
	/* Send START condition */
	I2C_GenerateSTART(I2C2, ENABLE);

	/* Test on EV5 and clear it */
	//while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	/* Send Hardware address for write */
	I2C_Send7bitAddress(I2C2, HARD_ADDRESS, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Clear EV6 by setting again the PE bit */
	I2C_Cmd(I2C2, ENABLE);

	/* Send the Hardware's internal address to write to */
	I2C_SendData(I2C2, ReadAddr);

	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send STRAT condition a second time */
	I2C_GenerateSTART(I2C2, ENABLE);

	/* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send Hardware address for read */
	I2C_Send7bitAddress(I2C2, HARD_ADDRESS, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	/* While there is data to be read */
	while (NumByteToRead) {
		if (NumByteToRead == 1) {
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2C2, DISABLE);

			/* Send STOP Condition */
			I2C_GenerateSTOP(I2C2, ENABLE);
		}

		/* Test on EV7 and clear it */
		if (I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
			/* Read a byte from the Hardware */
			*pBuffer = I2C_ReceiveData(I2C2);

			/* Point to the next location where the byte read will be saved */
			pBuffer++;

			/* Decrement the read bytes counter */
			NumByteToRead--;
		}
	}

	/* Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(I2C2, ENABLE);
}

/*******************************************************************************
* Function Name  : I2C_EE_ByteWrite
* Description    : Writes one byte to the I2C EEPROM.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the EEPROM.
*                  - WriteAddr : EEPROM's internal address to write to.
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_ByteWrite(u8 *pBuffer, u8 HARD_ADDRESS, u8 WriteAddr)
{
	/* Send STRAT condition */
	I2C_GenerateSTART(I2C2, ENABLE);

	/* Test on EV5 and clear it */
	//while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
#if ENABLE_I2C_DELAY
	I2C_delay();
#endif
	/* Send Hardware address for write */
	I2C_Send7bitAddress(I2C2, HARD_ADDRESS, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Send the Hardware's internal address to write to */
	I2C_SendData(I2C2, WriteAddr);

	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send the byte to be written */
	I2C_SendData(I2C2, *pBuffer);

	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send STOP condition */
	I2C_GenerateSTOP(I2C2, ENABLE);
}

/************************************文件结束*********************************/
