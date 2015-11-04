#ifndef __TOUCH_H__
#define __TOUCH_H__


#define key0 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)   
#define key1 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)   
#define key2 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)   
#define key3 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)   


void Touch_Init(void);

void Touch_Key_Proc(void);


#endif
