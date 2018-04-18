/*******************************************************************************
* Description  : 开启全局定时器
* Author       : 2018/4/18 星期三, by redmorningcn
*******************************************************************************/

/*******************************************************************************
* INCLUDES
*/
#include <bsp_timer.h>


/*******************************************************************************
* Description  : 全局时间累积。真实时间 time = strSys.time * 65536 + TIM_GetCounter  
                              再乘以单周期时间。
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void TIM8_OVER_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM8,TIM_IT_Update)!=RESET)  //计数器溢出中断
	{
		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);  //清除中断标志
        sys.time++;                                 //系统是时间累加
	}
}

/*******************************************************************************
* Description  : 全局时钟，为所有信号提供统一时间标准
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void Timer8_Iint(void)
{
	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);         //使能定时器时钟
	//初始化定时器8
	TIM_BaseInitStructure.TIM_Period = 65535;                   //计数器自动重装值
	TIM_BaseInitStructure.TIM_Prescaler = 0;                    //不分频
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟不分割
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数设置
    
	TIM_TimeBaseInit(TIM8,&TIM_BaseInitStructure);              //初始化时钟
    
	TIM_ClearFlag(TIM8, TIM_FLAG_Update);                       //清楚中断标志位
	TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);                    //不允许更新中断
	TIM_Cmd(TIM8,ENABLE);
    
	BSP_IntVectSet(TIM8_UP_IRQn, TIM8_OVER_IRQHandler);
	BSP_IntEn(TIM8_UP_IRQn);
    
    sys.time = 0;                                               //系统时间置0
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
