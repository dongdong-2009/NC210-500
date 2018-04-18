/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                          (c) Copyright 2003-2009; Micrium, Inc.; Weston, FL               
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            uC/Modbus
*
*                                      MODBUS BOARD SUPPORT PACKAGE
*                                         Philips LPC2000 (ARM7)
*
* Filename    : mb_bsp.c
* Version     : V2.12
* Programmers : JJL
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include  <global.h>
#include <includes.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES 
*********************************************************************************************************
*/

#define  BIT0                       0x01
#define  BIT1                       0x02
#define  BIT2                       0x04
#define  BIT3                       0x08
#define  BIT4                       0x10
#define  BIT5                       0x20
#define  BIT6                       0x40
#define  BIT7                       0x80

#define UARAT_CFG_MAX_PORT          5           // ������󴮿�����
/***********************************************
* ������ 
*/
#define  BSP_COM1_TXD               DEF_BIT_09
#define  BSP_COM1_RXD               DEF_BIT_10

#define  BSP_COM2_TXD               DEF_BIT_02
#define  BSP_COM2_RXD               DEF_BIT_03

#define  BSP_COM3_TXD               DEF_BIT_02
#define  BSP_COM3_RXD               DEF_BIT_03

/***********************************************
* ������ 
*/
#define  BSP_COM1_REN               DEF_DISABLED
#define  BSP_COM2_REN               DEF_DISABLED
#define  BSP_COM3_REN               DEF_ENABLED
#define  BSP_COM4_REN               DEF_ENABLED

/***********************************************
* ������ 
*/
#if (BSP_COM1_REN == DEF_ENABLED)
#define MBREN1_GPIO_PIN        GPIO_Pin_8             /* PA.08 */
#define MBREN1_GPIO_PORT       GPIOA
#define MBREN1_GPIO_RCC        RCC_APB2Periph_GPIOA
#endif

#if (BSP_COM2_REN == DEF_ENABLED)
#define MBREN2_GPIO_PIN        GPIO_Pin_1             /* PA.01 */
#define MBREN2_GPIO_PORT       GPIOA
#define MBREN2_GPIO_RCC        RCC_APB2Periph_GPIOA
#endif

#if (BSP_COM3_REN == DEF_ENABLED)
#define MBREN3_GPIO_PIN        GPIO_Pin_15             /* PE.15 */
#define MBREN3_GPIO_PORT       GPIOE
#define MBREN3_GPIO_RCC        RCC_APB2Periph_GPIOE
#endif

#if (BSP_COM4_REN == DEF_ENABLED)
#define MBREN4_GPIO_PIN        GPIO_Pin_15             /* PE.15 */
#define MBREN4_GPIO_PORT       GPIOA
#define MBREN4_GPIO_RCC        RCC_APB2Periph_GPIOA
#endif
/*
*********************************************************************************************************
*                                             LOCAL VARIABLES
*********************************************************************************************************
*/

static  CPU_INT32U  MB_Tmr_ReloadCnts;
static  MODBUS_CH  *MB_ChPortMap[UARAT_CFG_MAX_PORT] = {NULL,NULL,NULL,NULL,NULL};

void            USART1_RxTxISRHandler   (void);
void            USART2_RxTxISRHandler   (void);
void            USART3_RxTxISRHandler   (void);
void            USART4_RxTxISRHandler   (void);
void            USARTx_RxTxISRHandler   (MODBUS_CH *pch);

/*$PAGE*/
/*
*********************************************************************************************************
*                                             MB_CommExit()
*
* Description : This function is called to terminate Modbus communications.  All Modbus channels are close.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  MB_CommExit (void)
{
    CPU_INT08U   ch;
    MODBUS_CH   *pch;


    pch = &MB_ChTbl[0];
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
        MB_CommTxIntDis(pch);
        MB_CommRxIntDis(pch);
        pch++;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MB_CommPortCfg()
*
* Description : This function initializes the serial port to the desired baud rate and the UART will be
*               configured for N, 8, 1 (No parity, 8 bits, 1 stop).
*
* Argument(s) : pch        is a pointer to the Modbus channel
*               port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a sepcific Modbus channel.
*               baud       is the desired baud rate for the serial port.
*               parity     is the desired parity and can be either:
*
*                          MODBUS_PARITY_NONE
*                          MODBUS_PARITY_ODD
*                          MODBUS_PARITY_EVEN
*
*               bits       specifies the number of bit and can be either 7 or 8.
*               stops      specifies the number of stop bits and can either be 1 or 2
*
* Return(s)   : none.
*
* Caller(s)   : MB_CfgCh()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  MB_CommPortCfg (MODBUS_CH  *pch,
                      CPU_INT08U  port_nbr,
                      CPU_INT32U  baud,
                      CPU_INT08U  bits,
                      CPU_INT08U  parity,
                      CPU_INT08U  stops)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
    INT16U            BSP_INT_ID_USARTx;
    USART_TypeDef*    USARTx;
    CPU_FNCT_VOID     USARTx_RxTxISRHandler;
    
    /***********************************************
    * ������ ����3���Žӵ��أ�����ʹ��
    */
    if ( port_nbr >= UARAT_CFG_MAX_PORT )
      return;
    
    /***********************************************
    * ������ 
    */
    switch ( port_nbr ) {
    case 0:
        USARTx                    = USART1;
        BSP_INT_ID_USARTx         = BSP_INT_ID_USART1;
        USARTx_RxTxISRHandler     = USART1_RxTxISRHandler;
        
        /* Enable USART1 clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        
        /* Configure USART1 Rx (PA.10) as input floating */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /* Configure USART1 Tx (PA.09) as alternate function push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /***********************************************
        * ������ ���շ���ʹ���ź�
        */
#if (BSP_COM1_REN == DEF_ENABLED)
        RCC_APB2PeriphClockCmd(MBREN1_GPIO_RCC, ENABLE);
        GPIO_InitStructure.GPIO_Pin   = MBREN1_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init(MBREN1_GPIO_PORT, &GPIO_InitStructure);
        
        GPIO_ResetBits(MBREN1_GPIO_PORT, MBREN1_GPIO_PIN);  // �͵�ƽ����ʹ�� 
#endif
        
        break;
    case 1:
        USARTx                    = USART2;
        BSP_INT_ID_USARTx         = BSP_INT_ID_USART2;
        USARTx_RxTxISRHandler     = USART2_RxTxISRHandler;
        
        /* Enable USART2 clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        
        /* Configure USART1 Rx (PA.3) as input floating */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /* Configure USART1 Tx (PA.2) as alternate function push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        /***********************************************
        * ������ ���շ���ʹ���ź�
        */
#if (BSP_COM2_REN == DEF_ENABLED)
        GPIO_InitTypeDef  GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(MBREN2_GPIO_RCC, ENABLE);
        GPIO_InitStructure.GPIO_Pin   = MBREN2_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init(MBREN2_GPIO_PORT, &GPIO_InitStructure);
        
        GPIO_ResetBits(MBREN2_GPIO_PORT, MBREN2_GPIO_PIN);  // �͵�ƽ����ʹ��  
#endif
        break;
    case 2:
        USARTx                    = USART3;
        BSP_INT_ID_USARTx         = BSP_INT_ID_USART3;
        USARTx_RxTxISRHandler     = USART3_RxTxISRHandler;
        
        /* Enable USART3 clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        
        /* Configure USART1 Rx (PB.11) as input floating */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        /* Configure USART1 Tx (PB.10) as alternate function push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        /***********************************************
        * ������ ���շ���ʹ���ź�
        */
#if (BSP_COM3_REN == DEF_ENABLED)
        RCC_APB2PeriphClockCmd(MBREN3_GPIO_RCC, ENABLE);
        GPIO_InitStructure.GPIO_Pin   = MBREN3_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init(MBREN3_GPIO_PORT, &GPIO_InitStructure);
        
        GPIO_ResetBits(MBREN3_GPIO_PORT, MBREN3_GPIO_PIN);  // �͵�ƽ����ʹ��  
#endif
        break;
    case 3:
        USARTx                    = UART4;
        BSP_INT_ID_USARTx         = BSP_INT_ID_USART4;
        USARTx_RxTxISRHandler     = USART4_RxTxISRHandler;
        
        /* Enable USART4 clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        
        /* Configure USART4 Rx (PC.11) as input floating */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        
        /* Configure USART4 Tx (PC.10) as alternate function push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        
        /***********************************************
        * ������ ���շ���ʹ���ź�
        */
#if (BSP_COM4_REN == DEF_ENABLED)
        RCC_APB2PeriphClockCmd(MBREN4_GPIO_RCC, ENABLE);
        GPIO_InitStructure.GPIO_Pin   = MBREN4_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_Init(MBREN4_GPIO_PORT, &GPIO_InitStructure);
        
        GPIO_ResetBits(MBREN4_GPIO_PORT, MBREN4_GPIO_PIN);  // �͵�ƽ����ʹ��  
#endif
        break;
    default:
        return;
    }
    
    /***********************************************
    * ������ configuration
    */
    USART_InitStructure.USART_BaudRate              = baud;
    USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits              = USART_StopBits_1;
    USART_InitStructure.USART_Parity                = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
    
    /***********************************************
    * ������ 
    */
    USART_DeInit(USARTx);
    USART_Init(USARTx, &USART_InitStructure);
    
    /***********************************************
    * ������ 
    */
    USART_ClearFlag(USARTx,USART_FLAG_TXE);
    USART_ClearFlag(USARTx,USART_FLAG_RXNE);
    //USART_ClearFlag(USARTx,USART_FLAG_IDLE);
      
    /***********************************************
    * ������ 
    */
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
    //USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);
    
    /***********************************************
    * ������ 
    */
    USART_Cmd(USARTx, ENABLE);
    
    /***********************************************
    * ������ 
    */
    BSP_IntVectSet(BSP_INT_ID_USARTx, USARTx_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_USARTx);
    
    if (pch != (MODBUS_CH *)0) {
        pch->PortNbr            = port_nbr;                                 /* Store configuration in channel             */
        pch->BaudRate           = baud;
        pch->Parity             = parity;
        pch->Bits               = bits;
        pch->Stops              = stops;
        pch->USARTx             = USARTx;        
        MB_ChPortMap[port_nbr]  = pch;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                         MB_CommRxIntDis()
*
* Description : This function disables Rx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommExit()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  MB_CommRxIntDis (MODBUS_CH  *pch)
{
    CPU_SR  cpu_sr = 0;

    switch (pch->PortNbr) {
    case 0:
        /***********************************************
        * ������ �շ���ʹ�ܿ����ź�
        */
#if (BSP_COM1_REN == DEF_ENABLED)
        GPIO_SetBits(MBREN1_GPIO_PORT, MBREN1_GPIO_PIN);        // �ߵ�ƽ����ʹ�� 
#endif
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        break;
        
    case 1:
        /***********************************************
        * ������ �շ���ʹ�ܿ����ź�
        */
#if (BSP_COM2_REN == DEF_ENABLED)
        GPIO_SetBits(MBREN2_GPIO_PORT, MBREN2_GPIO_PIN);        // �ߵ�ƽ����ʹ�� 
#endif
        USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
        break;
        
    case 2:
        /***********************************************
        * ������ �շ���ʹ�ܿ����ź�
        */
#if (BSP_COM3_REN == DEF_ENABLED)
        GPIO_SetBits(MBREN3_GPIO_PORT, MBREN3_GPIO_PIN);        // �ߵ�ƽ����ʹ�� 
#endif
        USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
        break;
        
    case 3:
        /***********************************************
        * ������ �շ���ʹ�ܿ����ź�
        */
#if (BSP_COM4_REN == DEF_ENABLED)
        GPIO_SetBits(MBREN4_GPIO_PORT, MBREN4_GPIO_PIN);        // �ߵ�ƽ����ʹ�� 
#endif
        USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
        break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                          MB_CommRxIntEn()
*
* Description : This function enables Rx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  MB_CommRxIntEn (MODBUS_CH  *pch)
{
    CPU_SR  cpu_sr = 0;

//    CPU_CRITICAL_ENTER();
    switch (pch->PortNbr) {
    case 0:
#if (BSP_COM1_REN == DEF_ENABLED)
        GPIO_ResetBits(MBREN1_GPIO_PORT, MBREN1_GPIO_PIN);  // �͵�ƽ����ʹ�� 
#endif
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);        
        break;
        
    case 1:
#if (BSP_COM2_REN == DEF_ENABLED)
        GPIO_ResetBits(MBREN2_GPIO_PORT, MBREN2_GPIO_PIN);  // �͵�ƽ����ʹ�� 
#endif
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        break;
        
    case 2:
#if (BSP_COM3_REN == DEF_ENABLED)
        GPIO_ResetBits(MBREN3_GPIO_PORT, MBREN3_GPIO_PIN);  // �͵�ƽ����ʹ�� 
#endif
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
        break;
        
    case 3:
#if (BSP_COM4_REN == DEF_ENABLED)
        GPIO_ResetBits(MBREN4_GPIO_PORT, MBREN4_GPIO_PIN);  // �͵�ƽ����ʹ�� 
#endif
        USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
        break;
    }
//    CPU_CRITICAL_EXIT();
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                       MB_CommRxTxISR_Handler()
*
* Description : This function is the ISR for either a received or transmitted character.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is a ISR
*
* Note(s)     : (1) The pseudo-code for this function should be:  
*
*               if (Rx Byte has been received) {
*                  c = get byte from serial port;
*                  Clear receive interrupt;
*                  pch->RxCtr++;                      Increment the number of bytes received
*                  MB_RxByte(pch, c);                 Pass character to Modbus to process
*              }
*
*              if (Byte has been transmitted) {
*                  pch->TxCtr++;                      Increment the number of bytes transmitted
*                  MB_TxByte(pch);                    Send next byte in response
*                  Clear transmit interrupt           Clear Transmit Interrupt flag
*              }
*********************************************************************************************************
*/
CPU_INT08U   port_nbr   = 0;

void  USARTx_RxTxISRHandler (MODBUS_CH *pch)
{
    volatile  CPU_INT08U    rx_data;
    USART_TypeDef*          USARTx  = pch->USARTx;
    
    /***********************************************
    * �����������ж�
    */
    if (SET == USART_GetFlagStatus(USARTx, USART_FLAG_RXNE)) {
        rx_data = USART_ReceiveData(USARTx);
        pch->RxCtr++;
        MB_RxByte(pch, rx_data);                                    /* Pass character to Modbus to process                  */
        USART_ClearITPendingBit(USARTx, USART_IT_RXNE);             /* Clear the USART2 receive interrupt.                  */
    }
    /***********************************************
    * �����������ж�
    */
    if (SET == USART_GetFlagStatus(USARTx, USART_FLAG_TC)) {
        USART_ClearITPendingBit(USARTx, USART_IT_TC);               /* Clear the USART2 receive interrupt.                  */
        pch->TxCtr++;
        MB_TxByte(pch);                                             /* Send next byte                                       */
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                UART #0 Rx/Tx Communication handler for Modbus
*********************************************************************************************************
*/

/***********************************************
* ������ UART1�жϷ������
*/
void  USART1_RxTxISRHandler (void)
{
    USARTx_RxTxISRHandler(MB_ChPortMap[0]);
}

/***********************************************
* ������ UART2�жϷ������
*/
void  USART2_RxTxISRHandler (void)
{
    USARTx_RxTxISRHandler(MB_ChPortMap[1]);
}

/***********************************************
* ������ UART3�жϷ������
*/
void  USART3_RxTxISRHandler (void)
{
    USARTx_RxTxISRHandler(MB_ChPortMap[2]);
}

/***********************************************
* ������ UART4�жϷ������
*/
void  USART4_RxTxISRHandler (void)
{
    USARTx_RxTxISRHandler(MB_ChPortMap[3]);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                             MB_CommTx1()
*
* Description : This function is called to obtain the next byte to send from the transmit buffer.  When
*               all bytes in the reply have been sent, transmit interrupts are disabled and the receiver
*               is enabled to accept the next Modbus request.
*
* Argument(s) : c     is the byte to send to the serial port
*
* Return(s)   : none.
*
* Caller(s)   : MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommTx1 (MODBUS_CH  *pch,
                  CPU_INT08U  c)
{
    switch (pch->PortNbr) {
    case 0:
        /* Transmit Data */
        USART1->DR = (c & (u16)0x01FF);
        break;
        
    case 1:
        /* Transmit Data */
        USART2->DR = (c & (u16)0x01FF);
        break;
        
    case 2:
        /* Transmit Data */
        USART3->DR = (c & (u16)0x01FF);
        break;
        
    case 3:
        /* Transmit Data */
        UART4->DR = (c & (u16)0x01FF);
        break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                         MB_CommTxIntDis()
*
* Description : This function disables Tx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommExit()
*               MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommTxIntDis (MODBUS_CH  *pch)
{
    CPU_SR  cpu_sr = 0;

//    CPU_CRITICAL_ENTER();
    switch (pch->PortNbr) {                       /* Just enable the receiver interrupt                */
    case 0:
        USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        break;
        
    case 1:
        USART_ITConfig(USART2, USART_IT_TC, DISABLE);
        USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        break;
        
    case 2:
        USART_ITConfig(USART3, USART_IT_TC, DISABLE);
        USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        break;
        
    case 3:
        USART_ITConfig(UART4, USART_IT_TC, DISABLE);
        USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
        break;
    }
//    CPU_CRITICAL_EXIT();
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                         MB_CommTxIntEn()
*
* Description : This function enables Tx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_Tx()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  MB_CommTxIntEn (MODBUS_CH  *pch)
{
    CPU_SR  cpu_sr = 0;

    switch (pch->PortNbr) {                       /* Just enable the receiver interrupt                */
    case 0:
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        break;
        
    case 1:
        USART_ITConfig(USART2, USART_IT_TC, ENABLE);
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
        break;
        
    case 2:
        USART_ITConfig(USART3, USART_IT_TC, ENABLE);
        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
        break;
        
    case 3:
        USART_ITConfig(UART4, USART_IT_TC, ENABLE);
        USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
        break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MB_RTU_TmrInit()
*
* Description : This function is called to initialize the RTU timeout timer.
*
* Argument(s) : freq          Is the frequency of the modbus RTU timer interrupt.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrInit (void)
{
    CPU_INT32U  clk_freq;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
    
    clk_freq            = BSP_CPU_ClkFreq();
    MB_Tmr_ReloadCnts   = clk_freq / 36000/ MB_RTU_Freq - 1;
    
    //Ԥ��Ƶϵ����TIM_Prescaler��  = 36000-1������������ʱ��Ϊ72MHz/36000 = 2kHz 
    //�жϼ����� ��TIM_Period��    = MB_Tmr_ReloadCnts
    //��ʱ���ж�Ƶ��               = clk_freq / ��TIM_Prescaler + 1�� / ��TIM_Period + 1��
    //��ʱ���ж�Ƶ��               = 72000000 / 36000 / 5 = 400Hz
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);            // ʹ�ܶ�ʱ��ʱ��
    TIM_DeInit(TIM3);                                               // ��λTIM3��ʱ��
    
    /* TIM3 configuration */
    TIM_TimeBaseStructure.TIM_Period        = MB_Tmr_ReloadCnts;    // �ж�Ƶ��     
    TIM_TimeBaseStructure.TIM_Prescaler     = 36000 - 1;            // ��Ƶ36000       
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;         // ʱ�ӷ�Ƶ  
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;   // �����������ϼ���
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);           // ���TIM3����жϱ�־
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);      // TIM3����ж�����
    TIM_Cmd(TIM3, ENABLE);                          // ����tim3����
    
    /***********************************************
    * �����������ж���ں������ж����ȼ�
    */
    BSP_IntVectSet(BSP_INT_ID_TIM3, MB_RTU_TmrISR_Handler);
    BSP_IntEn(BSP_INT_ID_TIM3);
    
    MB_RTU_TmrResetAll();                                     /* Reset all the RTU timers, we changed freq. */
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           MB_RTU_TmrExit()
*
* Description : This function is called to disable the RTU timeout timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrExit (void)
{
    TIM_Cmd(TIM3,DISABLE);              //��ֹ������ 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       MB_RTU_TmrISR_Handler()
*
* Description : This function handles the case when the RTU timeout timer expires.
*
* Arguments   : none.
*
* Returns     : none.
*
* Caller(s)   : This is a ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrISR_Handler (void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update)== SET) {//����Ƿ�����������¼�
      TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);/* Clear timer #1 interrupt                           */
      MB_RTU_TmrCtr++;                              /* Indicate that we had activities on this interrupt. */
      MB_RTU_TmrUpdate();                           /* Check for RTU timers that have expired             */
    }
}
#endif
                      
void    UARTPutString(MODBUS_CH  *pch, const char *printfbuf)              
{
//    USART_ITConfig(pch->USARTx, USART_IT_TC, DISABLE);
//    printf((const char *)printfbuf);
//    USART_ClearITPendingBit(pch->USARTx, USART_IT_TC); 
//    USART_ITConfig(pch->USARTx, USART_IT_TC, ENABLE);
    int nbr_bytes   = strlen(printfbuf);
    
    if ( (nbr_bytes > 0) && (nbr_bytes < MB_DATA_NBR_REGS * 2 ) ) {  
        NMB_Tx((MODBUS_CH   *)pch,
               (CPU_INT08U  *)printfbuf,
               (CPU_INT16U   )nbr_bytes);
    }
}