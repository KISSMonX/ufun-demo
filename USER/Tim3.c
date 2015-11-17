#include "stm32f10x.h"
#include "Tim3.h"
#include "PCIe.h"
#include "SDIO.h"

unsigned int timer3_cnt = 0;

extern unsigned char read_lis3dh_flag;
extern unsigned char read_sd_detect_flag;
extern unsigned char sd_detect_change;
extern unsigned int buzzer_delay;
/**************************************************************/
//程 序 名： Tim3 NVIC_Config()
//开 发 者： Haichao.Xie
//入口参数： 无
//功能说明： 中断嵌套向量配置
//**************************************************************/
void Tim3_NVIC_Config(void)//嵌套向量控制器
{
	NVIC_InitTypeDef NVIC_InitStructure;                       //定义结构体变量
	//  设置优先分级组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);            //0组，全副优先级

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           //选择中断通道，库P166页，
	//  选择中断通道。注意：固件库中为XXX_IRQChannel，但该程序预定义为XXX_IRQn，所以要特别注意
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;         //响应优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //启动此通道的中断
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
	TIM_TimeBaseStructure.TIM_Period =  1000;

	/* 设置预分频 72分频 */
	TIM_TimeBaseStructure.TIM_Prescaler =  72 - 1;

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
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}




//TIMER3初始化
void Tim3_Init(void)
{
	Tim3_NVIC_Config();
	TIM3_Config();
}


/**************************************************************/
//程 序 名： TIM3_IRQHandler()
//开 发 者： chenhonglin
//入口参数： 无
//功能说明： 定时器3中断服务程序
//**************************************************************/
void TIM3_IRQHandler(void)						   //TIM3中断服务函数
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) { //TIM_IT_Update
		TIM_ClearITPendingBit (TIM3, TIM_IT_Update); //必须要清除中断标志位
		timer3_cnt++;

		Change_PcieIo_Flag();

		if (timer3_cnt >= 10) {
			timer3_cnt = 0;
			read_lis3dh_flag = 1;
			read_sd_detect_flag = 1;
		}

		if (sd_detect_change) {
			buzzer_delay++;
			if (buzzer_delay >= 50) {
				TIM_Cmd(TIM1, DISABLE);  //关闭蜂鸣器
				TIM_CtrlPWMOutputs(TIM1, DISABLE);
				sd_detect_change = 0;
				buzzer_delay = 0;
			}
		}
	}
}

