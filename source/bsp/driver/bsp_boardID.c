/*******************************************************************************
* Description  : 读设备编号
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <bsp_boardID.h>


/*******************************************************************************
* Description  : 端口初始化
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/
void Init_boardID( void )
{
    GPIO_InitTypeDef gpio_init;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    
    gpio_init.GPIO_Pin  =    GPIO_Pin_11
                            |GPIO_Pin_12;
    
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio_init);
}

/*******************************************************************************
* Description  : 取电路板ID号
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/
uint8   get_boardID(void)
{
	uint8 id = 0;
	Delay_Nus(10);
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11))
	{
		id |= 0x2;
	}
    
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12))
	{
		id |= 0x01;
	}
    
	return id;
}

/*******************************************************************************
* 				end of file
*******************************************************************************/
