#include "stm32f10x.h"
#include "PCIe.h"


//GPIOA GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_11|GPIO_Pin_12;
//GPIOB GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
//GPIOC GPIO_Pin_0|GPIO_Pin_1;
u32 timer3_cnt = 0;

u8	pa4_flag = 0;
u8	pa5_flag = 0;
u8	pa6_flag = 0;
u8	pa7_flag = 0;
u8	pa11_flag = 0;
u8	pa12_flag = 0;

u8	pb0_flag = 0;
u8	pb1_flag = 0;
u8	pb6_flag = 0;
u8	pb7_flag = 0;
u8	pb10_flag = 0;
u8	pb11_flag = 0;
u8	pb12_flag = 0;
u8	pb13_flag = 0;
u8	pb14_flag = 0;
u8	pb15_flag = 0;

u8	pc0_flag = 0;
u8	pc1_flag = 0;


/**************************************************************/
//程 序 名： NVIC_Config()
//开 发 者： Haichao.Xie
//入口参数： 无
//功能说明： 中断嵌套向量配置
//**************************************************************/
void NVIC_Config(void)//嵌套向量控制器
{		
	NVIC_InitTypeDef NVIC_InitStructure;                       //定义结构体变量		
	//  设置优先分级组   																
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);            //0组，全副优先级		 
 
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;             //选择中断通道，库P166页，
	//  选择中断通道。注意：固件库中为XXX_IRQChannel，但该程序预定义为XXX_IRQn，所以要特别注意	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;    //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;           //响应优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;              //启动此通道的中断
	NVIC_Init(&NVIC_InitStructure);                            //嵌套向量初始化		  
}

/**************************************************************/
//程 序 名： TIM3_Config()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 定时器基本时钟配置
//**************************************************************/
void TIM3_Config(void)
{
    //TIM_OCInitTypeDef  TIM_OCInitStructure;//定义结构体
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure ;//定义结构体
    /* 打开定时器2外设时钟 */ 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
                          
		/* 定时器2基本定时器设置 */
		/* 定时器从0计数到0xff为一个定时周期 */
    TIM_TimeBaseStructure.TIM_Period = 0xff;
	
		/* 设置预分频 2000 分频 */
    TIM_TimeBaseStructure.TIM_Prescaler = 5;
	
		/* 设置时钟分频系数，此处未分频 */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	
		/* 向上计数模式 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
		/* 基本定时器初始化 */
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
			
    /* 使能 TIM3 重载寄存器 ARR */
    TIM_ARRPreloadConfig(TIM3, ENABLE);
		
    /* 使能定时器3 */
    TIM_Cmd(TIM3, ENABLE);
		
		/* 使能定时器3的 update中断 */
    TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE);
}


/**************************************************************/
//程 序 名： TIM3_IRQHandler()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 定时器3中断服务程序
//**************************************************************/ 
void TIM3_IRQHandler(void)						   //TIM3中断服务函数
{
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //TIM_IT_Update
		{
				TIM_ClearITPendingBit (TIM3, TIM_IT_Update); //必须要清除中断标志位
				timer3_cnt++;
				/*********************************************************************
				u8	pa4_flag = 0;
				u8	pa5_flag = 0;
				u8	pa6_flag = 0;
				u8	pa7_flag = 0;
				u8	pa11_flag = 0;
				u8	pa12_flag = 0;

				u8	pb0_flag = 0;
				u8	pb1_flag = 0;
				u8	pb6_flag = 0;
				u8	pb7_flag = 0;
				u8	pb10_flag = 0;
				u8	pb11_flag = 0;
				u8	pb12_flag = 0;
				u8	pb13_flag = 0;
				u8	pb14_flag = 0;
				u8	pb15_flag = 0;

				u8	pc0_flag = 0;
				u8	pc1_flag = 0;
				*********************************************************************************/
	
				if(timer3_cnt%12 == 0) //pa4_flag
				{
						pa4_flag = !pa4_flag;
				}
				if(timer3_cnt%15 == 0) //pa5_flag
				{
						pa5_flag = !pa5_flag;
				}
				if(timer3_cnt%18 == 0) //pa6_flag
				{
						pa6_flag = !pa6_flag;
				}
				if(timer3_cnt%21 == 0) //pa7_flag
				{
						pa7_flag = !pa7_flag;
				}
				if(timer3_cnt%24 == 0) //pa11_flag
				{
						pa11_flag = !pa11_flag;
				}
				if(timer3_cnt%27 == 0) //pa12_flag
				{
						pa12_flag = !pa12_flag;
				}
				
				if(timer3_cnt%30 == 0) //pb0_flag
				{
						pb0_flag = !pb0_flag;
				}
				if(timer3_cnt%33 == 0) //pb1_flag
				{
						pb1_flag = !pb1_flag;
				}
				if(timer3_cnt%36 == 0) //pb6_flag
				{
						pb6_flag = !pb6_flag;
				}
				if(timer3_cnt%39 == 0) //pb7_flag
				{
						pb7_flag = !pb7_flag;
				}
				if(timer3_cnt%42 == 0) //pb10_flag
				{
						pb10_flag = !pb10_flag;
				}
				if(timer3_cnt%45 == 0) //pb11_flag
				{
						pb11_flag = !pb11_flag;
				}
				if(timer3_cnt%48 == 0) //pb12_flag
				{
						pb12_flag = !pb12_flag;
				}
				if(timer3_cnt%51 == 0) //pb13_flag
				{
						pb13_flag = !pb13_flag;
				}
				if(timer3_cnt%54 == 0) //pb14_flag
				{
						pb14_flag = !pb14_flag;
				}
				if(timer3_cnt%57 == 0) //pb15_flag
				{
						pb15_flag = !pb15_flag;
				}
		
				if(timer3_cnt%60 == 0) //pc0_flag
				{
						pc0_flag = !pc0_flag;
				}
				if(timer3_cnt%63 == 0) //pc1_flag
				{
						pc1_flag = !pc1_flag;
				}
				
		}
}


/**************************************************************/
//程 序 名： GPIOA_Config()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 配置GPIOA
//**************************************************************/
void GPIOA_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//定义结构体
    /* GPIOA clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_11|GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* GPIOA clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
}
/**************************************************************/
//程 序 名： GPIOB_Config()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 配置GPIOB
//**************************************************************/
void GPIOB_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//定义结构体
    /* GPIOB clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* GPIOB clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}
/**************************************************************/
//程 序 名： GPIOC_Config()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 配置GPIOC
//**************************************************************/
void GPIOC_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//定义结构体
    /* GPIOC clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    /* GPIOC clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

/**************************************************************/
//程 序 名： Pcie_Gpio_Init()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： pcie io 初始化
//**************************************************************/
void Pcie_Gpio_Init(void)
{
		NVIC_Config();	
		TIM3_Config();		
    GPIOA_Config();
    GPIOB_Config();
    GPIOC_Config();
}


/**************************************************************/
//程 序 名： Test_Pcie_Gpio()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 利用定时器2中断让每一个io都输出不同频率的方波，用示波器
//测试每个io的频率，任意io的频率不重复 pcie接口的电气连接完好.
//**************************************************************/
void Test_Pcie_Gpio(void)
{
				/****************************************************************************
				u8	pa4_flag = 0;
				u8	pa5_flag = 0;
				u8	pa6_flag = 0;
				u8	pa7_flag = 0;
				u8	pa11_flag = 0;
				u8	pa12_flag = 0;
				*********************************************************************************/
				if(pa4_flag)
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
				}
				
				if(pa5_flag)
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
				}
				
				if(pa6_flag)
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);
				}
				
				if(pa7_flag)
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
				}
				
				if(pa11_flag)
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
				}
				
				if(pa12_flag)
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_RESET);
				}
				
				/*************************************
				u8	pb0_flag = 0;
				u8	pb1_flag = 0;
				u8	pb6_flag = 0;
				u8	pb7_flag = 0;
				u8	pb10_flag = 0;
				u8	pb11_flag = 0;
				u8	pb12_flag = 0;
				u8	pb13_flag = 0;
				u8	pb14_flag = 0;
				u8	pb15_flag = 0;
				*************************************/
				
				if(pb0_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
				}
				
				if(pb1_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
				}
				
				if(pb6_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_RESET);
				}
				
				if(pb7_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET);
				}
				
				if(pb10_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET);
				}
				
				if(pb11_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_11, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_11, Bit_RESET);
				}
				
				if(pb12_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
				}
				
				if(pb13_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_RESET);
				}
				
				if(pb14_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_14, Bit_RESET);
				}
				if(pb15_flag)
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_15, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOB, GPIO_Pin_15, Bit_RESET);
				}
				/***********************************************
				u8	pc0_flag = 0;
				u8	pc1_flag = 0;
				************************************************/
				if(pc0_flag)
				{
						GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
				}
				
				if(pc1_flag)
				{
						GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET);
				}
				else
				{
						GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
				}
}



