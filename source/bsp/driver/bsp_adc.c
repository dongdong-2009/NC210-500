/*******************************************************************************
* Description  : ADC初始化
                1、提供1个初始化函数，定义初始化指端口及工作模式
                2、提供1个ADC电压读取函数，根据指定的AD通道，读取指定值，单位mV。
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/



/*******************************************************************************
* INCLUDES
*/
#include "bsp_adc.h"


/*******************************************************************************
* Description  : ADC工作模式设置
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/
void ADC_Mode_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);//使能ADC1时钟通道
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);                   //设置分频因子6 72M/6=12M，最大不能超过14M
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;       //模拟输入
	GPIO_Init(GPIOC,&GPIO_InitStructure);               //初始化
	
	ADC_DeInit(ADC1);                                   //复位ADC1，将外设ADC1的全部寄存机设置为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //ADC独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;       //单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//转换由软件启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;             //顺序进行转换的通道数
	
	ADC_Init(ADC1,&ADC_InitStructure);                  //根据指定的参数初始化ADC
	ADC_Cmd(ADC1,ENABLE);                               //使能ADC1
	ADC_ResetCalibration(ADC1);                         //开启复位校准
	while(ADC_GetResetCalibrationStatus(ADC1));         //等待复位结束
	ADC_StartCalibration(ADC1);                         //开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));              //等待校准结束
}


/*******************************************************************************
* Description  : 读取ADC值，数据转换过程采用硬等待。（需调整）
* Author       : 2018/1/24 星期三, by redmorningcn
*******************************************************************************/
uint16_t Get_ADC(uint8_t ch)//ch为通道号
{
    //设置指定ADC的规则组通道，设置它们的转换顺序和采样时间
    uint32  tmp32;
    
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_7Cycles5);//通道1，采样周期为7.5周期+12.5
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);//使能指定的ADC1的软件转换功能
    /*******************************************************************************
    * Description  : 等待ADC转换
    * Author       : 2018/3/29 星期四, by redmorningcn
    *******************************************************************************/
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));   //等待转换结束
    
    tmp32 = ADC_GetConversionValue(ADC1);           //取转换值
    
    tmp32 = (tmp32 * 3300)/4096;                    //返回测量电压值，单位mV
    
	return  tmp32;
}

/*******************************************************************************
* Description  : 信息电平及电源电平检测初始化
  *     @arg ADC_Channel_10: 通道1信号幅值检测
  *     @arg ADC_Channel_11: 通道2信号幅值检测 
  *     @arg ADC_Channel_12: 通道1电源幅值检测
  *     @arg ADC_Channel_12: 通道2电源幅值检测
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/
void Bsp_ADC_Init(void)
{
	ADC_Mode_Config();
}

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
