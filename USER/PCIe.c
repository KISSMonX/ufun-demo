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
//�� �� ���� NVIC_Config()
//�� �� �ߣ� Haichao.Xie
//��ڲ����� ��
//����˵���� �ж�Ƕ����������
//**************************************************************/
void NVIC_Config(void)//Ƕ������������
{		
	NVIC_InitTypeDef NVIC_InitStructure;                       //����ṹ�����		
	//  �������ȷּ���   																
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);            //0�飬ȫ�����ȼ�		 
 
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;             //ѡ���ж�ͨ������P166ҳ��
	//  ѡ���ж�ͨ����ע�⣺�̼�����ΪXXX_IRQChannel�����ó���Ԥ����ΪXXX_IRQn������Ҫ�ر�ע��	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;    //��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;           //��Ӧ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;              //������ͨ�����ж�
	NVIC_Init(&NVIC_InitStructure);                            //Ƕ��������ʼ��		  
}

/**************************************************************/
//�� �� ���� TIM3_Config()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� ��ʱ������ʱ������
//**************************************************************/
void TIM3_Config(void)
{
    //TIM_OCInitTypeDef  TIM_OCInitStructure;//����ṹ��
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure ;//����ṹ��
    /* �򿪶�ʱ��2����ʱ�� */ 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
                          
		/* ��ʱ��2������ʱ������ */
		/* ��ʱ����0������0xffΪһ����ʱ���� */
    TIM_TimeBaseStructure.TIM_Period = 0xff;
	
		/* ����Ԥ��Ƶ 2000 ��Ƶ */
    TIM_TimeBaseStructure.TIM_Prescaler = 5;
	
		/* ����ʱ�ӷ�Ƶϵ�����˴�δ��Ƶ */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	
		/* ���ϼ���ģʽ */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
		/* ������ʱ����ʼ�� */
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
			
    /* ʹ�� TIM3 ���ؼĴ��� ARR */
    TIM_ARRPreloadConfig(TIM3, ENABLE);
		
    /* ʹ�ܶ�ʱ��3 */
    TIM_Cmd(TIM3, ENABLE);
		
		/* ʹ�ܶ�ʱ��3�� update�ж� */
    TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE);
}


/**************************************************************/
//�� �� ���� TIM3_IRQHandler()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� ��ʱ��3�жϷ������
//**************************************************************/ 
void TIM3_IRQHandler(void)						   //TIM3�жϷ�����
{
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //TIM_IT_Update
		{
				TIM_ClearITPendingBit (TIM3, TIM_IT_Update); //����Ҫ����жϱ�־λ
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
//�� �� ���� GPIOA_Config()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� ����GPIOA
//**************************************************************/
void GPIOA_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//����ṹ��
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
//�� �� ���� GPIOB_Config()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� ����GPIOB
//**************************************************************/
void GPIOB_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//����ṹ��
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
//�� �� ���� GPIOC_Config()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� ����GPIOC
//**************************************************************/
void GPIOC_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//����ṹ��
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
//�� �� ���� Pcie_Gpio_Init()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� pcie io ��ʼ��
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
//�� �� ���� Test_Pcie_Gpio()
//�� �� �ߣ� chenhonglin
//��ڲ����� ��
//����˵���� ���ö�ʱ��2�ж���ÿһ��io�������ͬƵ�ʵķ�������ʾ����
//����ÿ��io��Ƶ�ʣ�����io��Ƶ�ʲ��ظ� pcie�ӿڵĵ����������.
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



