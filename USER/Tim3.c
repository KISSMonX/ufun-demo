#include "stm32f10x.h"
#include "Tim3.h"
#include "PCIe.h"
#include "SDIO_SD.h"

unsigned int timer3_cnt = 0;

extern unsigned char read_lis3dh_flag;
extern unsigned char read_sd_detect_flag;
extern unsigned char sd_detect_change;
extern unsigned int buzzer_delay;
/**************************************************************/
//�� �� ���� Tim3 NVIC_Config()
//�� �� �ߣ� Haichao.Xie
//��ڲ����� ��
//����˵���� �ж�Ƕ����������
//**************************************************************/
void Tim3_NVIC_Config(void)//Ƕ������������
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
    TIM_TimeBaseStructure.TIM_Period =  1000;
	
		/* ����Ԥ��Ƶ 72��Ƶ */
    TIM_TimeBaseStructure.TIM_Prescaler =  72 - 1;
	
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




//TIMER3��ʼ��
void Tim3_Init(void)
{
	Tim3_NVIC_Config();
	TIM3_Config();
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
		
		Change_PcieIo_Flag();
		
		if (timer3_cnt >= 10){
			timer3_cnt = 0;
			read_lis3dh_flag = 1;
			read_sd_detect_flag = 1;
		}

		if(sd_detect_change){
			buzzer_delay++;
			if (buzzer_delay >= 50){
				TIM_Cmd(TIM1, DISABLE);  //�رշ�����
				TIM_CtrlPWMOutputs(TIM1, DISABLE);
				sd_detect_change = 0;
				buzzer_delay = 0;
			}
		}
	}
}


