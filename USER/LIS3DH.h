/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : lis3dh_driver.h
* Author             : MSH Application Team
* Author             : Fabio Tota
* Version            : $Revision:$
* Date               : $Date:$
* Description        : Descriptor Header for lis3dh_driver.c driver file（3轴加速度传感器）
*
* HISTORY:
* Date        | Modification                                | Author
* 24/06/2011  | Initial Revision                            | Fabio Tota

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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIS3DH_DRIVER__H
#define __LIS3DH_DRIVER__H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/

//these could change accordingly with the architecture
typedef unsigned char u8_t;
typedef unsigned short int u16_t;
typedef signed char i8_t;
typedef short int i16_t;
typedef u8_t IntPinConf_t;
typedef u8_t Axis_t;
typedef u8_t Int1Conf_t;


typedef enum {
  MEMS_SUCCESS                  =		0x01,
  MEMS_ERROR			=		0x00	
} status_t;

typedef enum {
  MEMS_ENABLE			=		0x01,
  MEMS_DISABLE			=		0x00	
} State_t;

typedef enum {  
  ODR_1Hz		        	=		0x01,		
  ODR_10Hz            =		0x02,
  ODR_25Hz		        =		0x03,
  ODR_50Hz		        =		0x04,
  ODR_100Hz		        =		0x05,	
  ODR_200Hz		        =		0x06,
  ODR_400Hz		        =		0x07,
  ODR_1620Hz_LP		        =		0x08,
  ODR_1344Hz_NP_5367HZ_LP       =		0x09	
} ODR_t;

typedef enum {
  POWER_DOWN                    =		0x00,
  LOW_POWER 			=		0x01,
  NORMAL			=		0x02
} Mode_t;

typedef enum {
  FULLSCALE_2                   =               0x00,
  FULLSCALE_4                   =               0x01,
  FULLSCALE_8                   =               0x02,
  FULLSCALE_16                  =               0x03
} Fullscale_t;

typedef enum {
  BLE_LSB			=		0x00,
  BLE_MSB			=		0x01
} Endianess_t;

typedef enum {
  SELF_TEST_DISABLE             =               0x00,
  SELF_TEST_0                   =               0x01,
  SELF_TEST_1                   =               0x02
} SelfTest_t;

typedef enum {
  X_ENABLE                      =               0x01,
  X_DISABLE                     =               0x00,
  Y_ENABLE                      =               0x02,
  Y_DISABLE                     =               0x00,
  Z_ENABLE                      =               0x04,
  Z_DISABLE                     =               0x00    
} AXISenable_t;

/* Exported constants --------------------------------------------------------*/
// ACC LIS3DH传感器宏定义
#define ACCWriteAddress			0x32	// 在I2C总线中的地址
#define ACCReadAddress			0x33

#define MEMS_SET                0x01
#define MEMS_RESET              0x00


//Register Definition
#define WHO_AM_I				0x0F  // device identification register

// CONTROL REGISTER 1
#define CTRL_REG1				0x20
#define ODR_BIT				        BIT(4)
#define LPEN					BIT(3)
#define ZEN					BIT(2)
#define YEN					BIT(1)
#define XEN					BIT(0)

//CONTROL REGISTER 4
#define CTRL_REG4				0x23
#define BDU					BIT(7)
#define BLE					BIT(6)
#define FS					BIT(4)
#define HR					BIT(3)
#define ST       				BIT(1)
#define SIM					BIT(0)

//OUTPUT REGISTER
#define OUT_X_L					0x28
#define OUT_X_H					0x29
#define OUT_Y_L					0x2A
#define OUT_Y_H					0x2B
#define OUT_Z_L					0x2C
#define OUT_Z_H					0x2D
#define ACC_XOUT_L				OUT_X_L
#define ACC_XOUT_H				OUT_X_H
#define ACC_YOUT_L				OUT_Y_L
#define ACC_YOUT_H				OUT_Y_H
#define ACC_ZOUT_L				OUT_Z_L
#define ACC_ZOUT_H				OUT_Z_H

#define I_AM_LIS3DH			        0x33

//FIFO REGISTERS
#define FIFO_CTRL_REG			        0x2E
#define FIFO_SRC_REG			        0x2F


/* Exported macro ------------------------------------------------------------*/
//#define ValBit(VAR,Place)         (VAR & (1<<Place))
#define BIT(x) ( (x) )

/* Exported functions --------------------------------------------------------*/
//Sensor Configuration Functions
status_t SetODR(ODR_t ov);
status_t SetMode(Mode_t md);
status_t SetAxis(Axis_t axis);
status_t SetFullScale(Fullscale_t fs);
status_t SetBDU(State_t bdu);
status_t SetBLE(Endianess_t ble);

unsigned char LIS3DH_Init(void);

void Collect_Data(s16* data);
//Generic
// i.e. u8_t ReadReg(u8_t Reg, u8_t* Data);
// i.e. u8_t WriteReg(u8_t Reg, u8_t Data);
extern void WriteI2C(u8,u8,u8); 
extern u8 ReadI2C(u8,u8);

#endif /* __LIS3DH_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
