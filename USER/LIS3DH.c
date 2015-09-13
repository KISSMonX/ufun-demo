/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : LIS3DH_driver.c
* Author             : MSH Application Team
* Author             : Fabio Tota
* Version            : $Revision:$
* Date               : $Date:$
* Description        : LIS3DH driver file
*                      
* HISTORY:
* Date               |	Modification                    |	Author
* 24/06/2011         |	Initial Revision                |	Fabio Tota

********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lis3dh_driver.h"
#include "I2C.h"


/*******************************************************************************
* Function Name		: ReadReg
* Description		: Generic Reading function. It must be fullfilled with either
*			: I2C or SPI reading functions					
* Input			: Register Address
* Output		: Data REad
* Return		: None
*******************************************************************************/
u8_t ReadReg(u8_t Reg, u8_t* Data) 
{
  //To be completed with either I2c or SPI reading function
  //i.e. *Data = SPI_Mems_Read_Reg( Reg );
  *Data = ReadI2C(Reg , 0x32);
  return 1;
}


/*******************************************************************************
* Function Name		: WriteReg
* Description		: Generic Writing function. It must be fullfilled with either
*			: I2C or SPI writing function
* Input			: Register Address, Data to be written
* Output		: None
* Return		: None
*******************************************************************************/
u8_t WriteReg(u8_t Reg, u8_t Data) 
{
  //To be completed with either I2c or SPI writing function
  //i.e. SPI_Mems_Write_Reg(Reg, Data);
  WriteI2C(Data, Reg , 0x32);
  return 1;
}

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : SetODR
* Description    : Sets LIS3DH Output Data Rate
* Input          : Output Data Rate
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t SetODR(ODR_t ov)
{
  u8_t value;

  if( !ReadReg(CTRL_REG1, &value) )
    return MEMS_ERROR;

  value &= 0x0f;
  value |= ov<<ODR_BIT;

  if( !WriteReg(CTRL_REG1, value) )
    return MEMS_ERROR;

  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : SetMode
* Description    : Sets LIS3DH Operating Mode
* Input          : Modality (NORMAL, LOW_POWER, POWER_DOWN)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t SetMode(Mode_t md) {
  u8_t value;
  u8_t value2;
  static   u8_t ODR_old_value;
 
  if( !ReadReg(CTRL_REG1, &value) )
    return MEMS_ERROR;
  
  if( !ReadReg(CTRL_REG4, &value2) )
    return MEMS_ERROR;
  
  if((value & 0xF0)==0) value = value | (ODR_old_value & 0xF0); //if it comes from POWERDOWN  
    
  switch(md) {
  
  case POWER_DOWN:
    ODR_old_value = value;
    value &= 0x0F;
    break;
          
  case NORMAL:
    value &= 0xF7;
    value |= (MEMS_RESET<<LPEN);
    value2 &= 0xF7;
    value2 |= (MEMS_SET<<HR);   //set HighResolution_BIT
    break;
          
  case LOW_POWER:		
    value &= 0xF7;
    value |=  (MEMS_SET<<LPEN);
    value2 &= 0xF7;
    value2 |= (MEMS_RESET<<HR); //reset HighResolution_BIT
    break;
          
  default:
    return MEMS_ERROR;
  }
  
  if( !WriteReg(CTRL_REG1, value) )
    return MEMS_ERROR;
  
  if( !WriteReg(CTRL_REG4, value2) )
    return MEMS_ERROR;  
   
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : SetAxis
* Description    : Enable/Disable LIS3DH Axis
* Input          : X_ENABLE/X_DISABLE | Y_ENABLE/Y_DISABLE | Z_ENABLE/Z_DISABLE
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t SetAxis(Axis_t axis) {
  u8_t value;
  
  if( !ReadReg(CTRL_REG1, &value) )
    return MEMS_ERROR;
  value &= 0xF8;
  value |= (0x07 & axis);
   
  if( !WriteReg(CTRL_REG1, value) )
    return MEMS_ERROR;   
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : SetFullScale
* Description    : Sets the LIS3DH FullScale
* Input          : FULLSCALE_2/FULLSCALE_4/FULLSCALE_8/FULLSCALE_16
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t SetFullScale(Fullscale_t fs) {
  u8_t value;
  
  if( !ReadReg(CTRL_REG4, &value) )
    return MEMS_ERROR;
                  
  value &= 0xCF;	
  value |= (fs<<FS);
  
  if( !WriteReg(CTRL_REG4, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : SetBDU
* Description    : Enable/Disable Block Data Update Functionality
* Input          : ENABLE/DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t SetBDU(State_t bdu) {
  u8_t value;
  
  if( !ReadReg(CTRL_REG4, &value) )
    return MEMS_ERROR;
 
  value &= 0x7F;
  value |= (bdu<<BDU);

  if( !WriteReg(CTRL_REG4, value) )
    return MEMS_ERROR;

  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : SetBLE
* Description    : Set Endianess (MSB/LSB)
* Input          : BLE_LSB / BLE_MSB
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t SetBLE(Endianess_t ble) {
  u8_t value;
  
  if( !ReadReg(CTRL_REG4, &value) )
    return MEMS_ERROR;
                  
  value &= 0xBF;	
  value |= (ble<<BLE);
  
  if( !WriteReg(CTRL_REG4, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

unsigned char LIS3DH_Init(void)
{   
	u8 response;

	switch(50)//config ODR
	{
		case 50:
			response = SetODR(ODR_50Hz);
			break;
		case 100:
			response = SetODR(ODR_100Hz);
			break;
		case 200:
			response = SetODR(ODR_200Hz);
			break;
		case 400:
			response = SetODR(ODR_400Hz);
			break;
		default: return ACC_ODR_ERROR;
	}
	if(response == MEMS_ERROR) return CONFIG_ERROR_ACC;

  // Set PowerMode 
	response = SetMode(NORMAL);
	if(response == MEMS_ERROR) return CONFIG_ERROR_ACC;

  // Set Fullscale
	// FULLSCALE_2		1mg/digit
	// FULLSCALE_4		2mg/digit
	// FULLSCALE_8		4mg/digit
	// FULLSCALE_16		12mg/digit
	switch(2000)//config RANGE
	{
		case 2000:
			response = SetFullScale(FULLSCALE_2);
			break;
		case 4000:
			response = SetFullScale(FULLSCALE_4);
			break;
		case 8000:
			response = SetFullScale(FULLSCALE_8);
			break;
		case 16000:
			response = SetFullScale(FULLSCALE_16);
			break;
		default: return ACC_RANGE_ERROR;
	}
	if(response == MEMS_ERROR) return CONFIG_ERROR_ACC;

// Using the block data update (BDU) feature
	response = SetBDU(MEMS_ENABLE);
	if(response == MEMS_ERROR) return CONFIG_ERROR_ACC;

// Big-little endian selection
	response = SetBLE(BLE_LSB);
	if(response == MEMS_ERROR) return CONFIG_ERROR_ACC;

// Set axis Enable
	response = SetAxis(X_ENABLE | Y_ENABLE | Z_ENABLE);
	if(response == MEMS_ERROR) return CONFIG_ERROR_ACC;

	return NO_ERROR;
}

void Collect_Data(s16* data)
{
	// 加速度的数据，前6字节
	data[0] = ReadI2C(ACC_YOUT_H, ACCWriteAddress) << 8;
	data[0]|= ReadI2C(ACC_YOUT_L, ACCWriteAddress);	// X、Y轴相互交换
	data[0] >>= 4;// 传感器后四位永远为0，这里要做的是右对齐，使一个bit的变化代表4mg
	data[1] = ReadI2C(ACC_XOUT_H, ACCWriteAddress) << 8;
	data[1]|= ReadI2C(ACC_XOUT_L, ACCWriteAddress);	// X、Y轴相互交换
	data[1] = -data[1];
	data[1] >>= 4;// 传感器后四位永远为0，这里要做的是右对齐，使一个bit的变化代表4mg
	data[2] = ReadI2C(ACC_ZOUT_H, ACCWriteAddress) << 8;
	data[2]|= ReadI2C(ACC_ZOUT_L, ACCWriteAddress);	// Z轴不变
	data[2] >>= 4;// 传感器后四位永远为0，这里要做的是右对齐，使一个bit的变化代表4mg
	data[2] = -data[2];
}



