/*******************************************************************************
* Description  : ADC转换头文件
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/
#ifndef __BSP_ADC_H__
#define	__BSP_ADC_H__


#include "stm32f10x.h"
#include "global.h"





/*******************************************************************************
 * 描述： 外部函数调用
 */

void Bsp_ADC_Init(void);    

uint16_t Get_ADC(uint8_t ch);


#endif /* __ADC_H */

