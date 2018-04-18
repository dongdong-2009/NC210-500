
#include <includes.h>
#include <bsp_adc.h>
#include <bsp_boardID.h>
#include <app.h>
#include <bsp.h>
#include <app_loco_runstatus.h>



/*******************************************************************************
* Description  :  指示灯任务，手柄在零位或没信号，慢闪，其他快闪，定期调用（100ms）
* Author       : 2018/4/13 星期五, by redmorningcn
*******************************************************************************/
void    led_task(void)
{
    static  uint8   blinkcnt= 0;
    
    blinkcnt++;
    
    if(loco.para.loco.lw )   //闪快慢控制   
    {
        blinkcnt %= 20;                                     //所有通道无信号，慢闪  
    }
    else
    {
        blinkcnt %= 2;                                      //任意通道有信号，快闪
    }
    
    if(blinkcnt == 0)                                       //亮、灭灯
    {
        BSP_LED_On(1);
    }
    else
    {
        BSP_LED_Off(1);
    }
}

/*******************************************************************************
* Description  : 闲置任务，时间不紧迫的工作在此运行
* Author       : 2018/4/16 星期一, by redmorningcn
*******************************************************************************/
void    idle_task(void)      
{
    static  uint32  tick;
    if(sys.time > tick+100 ||  sys.time < tick) //100ms
    {
        tick = sys.time;                        //时间
        
        led_task();                             //指示灯控制

    }
}


void main (void)
{
	BSP_Init();                                                 /* Initialize BSP functions                             */
	CPU_TS_TmrInit();
	/***********************************************
	* 描述： 初始化滴答定时器，即初始化系统节拍时钟。
	*/
	sys.cpu_freq = BSP_CPU_ClkFreq();  //时钟频率               /* Determine SysTick reference freq.                    */
    
    /*******************************************************************************
    * Description  : 信号幅值及工作电源电压检测初始化化
    * Author       : 2018/4/12 星期四, by redmorningcn
    *******************************************************************************/
	Bsp_ADC_Init();
    
    /*******************************************************************************
    * Description  : 设备ID号获取初始化
    * Author       : 2018/4/13 星期五, by redmorningcn
    *******************************************************************************/
    Init_boardID();
    sys.id = get_boardID();
    
    Timer8_Iint();              //启动全局定时器

    while(1)
    {
        /*******************************************************************************
        * Description  : 采集计算工况电压
        * Author       : 2018/4/17 星期二, by redmorningcn
        *******************************************************************************/
        app_calc_locovoltage();
        
        /*******************************************************************************
        * Description  : 空闲任务，时间不敏感的的工作在此运行，每100ms运行一次
        * Author       : 2018/4/16 星期一, by redmorningcn
        *******************************************************************************/
        idle_task();
    }
}

