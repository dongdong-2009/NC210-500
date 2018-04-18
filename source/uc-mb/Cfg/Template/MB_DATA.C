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
*                         uC/MODBUS TARGET SPECIFIC DATA ACCESS FUNCTIONS (Template)
*
* Filename      : mb_data.c
* Version       : V2.12
* Programmer(s) : JJL
*********************************************************************************************************
* Note(s)       :
*********************************************************************************************************
*/


#include <mb.h>
#include <os.h>
#include <app.h>
#include <bsp_time.h>
#include <bsp_eeprom.h>
#include <includes.h>
#include <global.h>

/*******************************************************************************
* ��    �ƣ� exchangeBytes
* ��    �ܣ� ģ���htons ���� ntohs�����ϵͳ֧�ֽ�����Ŀ�ֱ���滻��ϵͳ����
* ��ڲ����� value
* ���ڲ����� ���Ĺ��ֽ����short��ֵ
* ���� ���ߣ� ������.
* �������ڣ� 2015-06-25
* ��    �ģ�
* �޸����ڣ�
* ��    ע��
*******************************************************************************/
//int16_t	exchangeBytes(int16_t	value)
//{
//	int16_t		tmp_value;
//	uint8_t		*index_1, *index_2;
//
//	index_1 = (uint8_t *)&tmp_value;
//	index_2 = (uint8_t *)&value;
//
//	*index_1 = *(index_2+1);
//	*(index_1+1) = *index_2;
//
//	return tmp_value;
//}

#if MODBUS_CFG_SLAVE_EN == DEF_ENABLED
StrMbData   mbData  = {MB_DATA_NBR_REGS, MB_DATA_NBR_COILS, 0,0,0};
#endif
#define BSP_I2CSetPort(a)		{}
/*$PAGE*/
/*
*********************************************************************************************************
*                                     GET THE VALUE OF A SINGLE COIL
*
* Description: This function returns the value of a single coil.
*              It is called by 'MBS_FC01_CoilRd()'.
*              You must 'map' the 'coil' to the actual application's coil.
*
* Arguments  : coil     is the coil number that is being requested.
*
*              perr     is a pointer to an error code variable.  You must either return:
*
*                       MODBUS_ERR_NONE     the specified coil is valid and you are returning its value.
*                       MODBUS_ERR_RANGE    the specified coil is an invalid coil number in your
*                                           application (i.e. product).  YOUR product defines what the
*                                           valid range of values is for the 'coil' argument.
*
* Note(s)    : 1) You can perform the mapping of coil number to application coils directly in this
*                 function or via a table lookup.  A table lookup would make sense if you had a lot of
*                 coils in your product.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC01_EN == DEF_ENABLED)
CPU_BOOLEAN  MB_CoilRd (CPU_INT16U   coil,
                        CPU_INT16U  *perr)
{
    /***********************************************
    * ������ ����ȷ�ϣ�ͨѶǰ�Ƚ�MB_DATA_ACC_PASSWORDд��reg0
    */
    if ( Ctrl.Para.dat.Password != MB_DATA_ACC_PASSWORD ) {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
        return 0;
    }
    
    CPU_BOOLEAN     coil_val;
    CPU_INT16U      reg         = coil / 16;        // ��ȡ��ǰ�Ĵ���
    CPU_INT08U      bit         = coil % 16;        // ��ȡ��ǰ�Ĵ�����λ
    CPU_INT16U      reg_val;
    //CPU_INT16U      *preg       = (CPU_INT16U *) Ctrl.Para.buf2;
    CPU_INT16U *preg       = (CPU_INT16U *)& Ctrl.Para.buf2[0];
    /***********************************************
    * ������ ��ȡֵ
    */
    if ( reg < sizeof(Ctrl) / 2 ) {
        reg_val = preg[reg];
        *perr = MODBUS_ERR_NONE;
    } else {
        reg_val = 0;
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
    }
        
    /***********************************************
    * ������ ��ȡ��ǰλ��ֵ
    */
    coil_val    = (CPU_BOOLEAN)(reg_val & ( 1 << bit ) );

    return (coil_val);
}
#endif

/*
*********************************************************************************************************
*                                     SET THE VALUE OF A SINGLE COIL
*
* Description: This function changes the value of a single coil.
*              It is called by 'MBS_FC05_CoilWr()' and 'MBS_FC15_CoilWrMultiple()'.
*              You must 'map' the 'coil' to the actual application's coil.
*
* Arguments  : coil      is the coil number that needs to be changed.
*
*              coil_val  is the desired value of the coil.  This value can be either DEF_TRUE or DEF_FALSE with
*                        DEF_TRUE indicating an energized coil.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified coil is valid and your code changed the value
*                                            of the coil.
*                        MODBUS_ERR_RANGE    the specified coil is an invalid coil number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'coil' argument.
*                        MODBUS_ERR_WR       if the device is not able to write or accept the value
*
* Note(s)    : 1) You can perform the mapping of coil number to application coils directly in this
*                 function or via a table lookup.  A table lookup would make sense if you had a lot of
*                 coils in your product.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC05_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC15_EN == DEF_ENABLED)
void  MB_CoilWr (CPU_INT16U    coil,
                 CPU_BOOLEAN   coil_val,
                 CPU_INT16U   *perr)
{
    /***********************************************
    * ������ ����ȷ�ϣ�ͨѶǰ�Ƚ�MB_DATA_ACC_PASSWORDд��reg0
    */
    if ( Ctrl.Para.dat.Password != MB_DATA_ACC_PASSWORD ) {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
        return;
    }
    
    (void)coil;
    (void)coil_val;
    CPU_SR          cpu_sr;
    CPU_INT16U      reg         = coil / 16;
    CPU_INT08U      bit         = coil % 16;
    CPU_INT16U      reg_val     = 0;
    CPU_INT16U      temp;

    /***********************************************
    * ������ ��ȡ��ǰλ
    */
    reg_val         |= coil_val << bit;
    
    //CPU_INT16U      *preg       = (CPU_INT16U *) Ctrl.Para.buf2;
    CPU_INT16U *preg       = (CPU_INT16U *)& Ctrl.Para.buf2[0];
    /***********************************************
    * ������ ��ȡֵ
    */
    if ( reg < sizeof(Ctrl) / 2 ) {
        /***********************************************
        * ������
        */
        if ( reg < 127 ) {
            int idx = reg - 0;
            
            temp          = preg[idx];
            ( reg_val )   ? ( reg_val = reg_val | temp )
                : ( reg_val =~reg_val & temp );
                preg[idx]       = reg_val;
        }
        *perr = MODBUS_ERR_NONE;
    } else {
        reg_val = 0;
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                GET THE VALUE OF A SINGLE DISCRETE INPUT
*
* Description: This function reads the value of a single DI (DI means Discrete Input).
*              It is called by 'MBS_FC02_DIRd()'.
*              You must 'map' the 'di'  to the actual application's DI.
*
* Arguments  : di        is the Discrete Input number that needs to be read.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified DI is valid and your code is returning its
*                                            current value.
*                        MODBUS_ERR_RANGE    the specified DI is an invalid Discrete Input number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'di' argument.
*
* Note(s)    : 1) You can perform the mapping of DI number to the application DIs directly in this function
*                 or via a table lookup.  A table lookup would make sense if you had a lot of Discrete
*                 Inputs in your product.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC02_EN == DEF_ENABLED)
CPU_BOOLEAN  MB_DIRd (CPU_INT16U   di,
                      CPU_INT16U  *perr)
{
    (void)di;
    *perr = MODBUS_ERR_NONE;
    return (DEF_FALSE);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                               GET THE VALUE OF A SINGLE INPUT REGISTER
*
* Description: This function reads the value of a single Input Register.
*              It is called by 'MBS_FC04_InRegRd()' when the argument 'reg' is BELOW the value set by
*              the configuration constant MODBUS_CFG_FP_START_IX (see MB_CFG.H).
*              You must 'map' the Input Register to the actual application's corresponding integer register.
*
* Arguments  : reg       is the Input Register number that needs to be read.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified input register is valid and your code is
*                                            returning its current value.
*                        MODBUS_ERR_RANGE    the specified input register is an invalid number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'reg' argument.
*
* Note(s)    : 1) You can perform the mapping of input register number to the application's input registers
*                 directly in this function or via a table lookup.  A table lookup would make sense if you
*                 had a lot of Input Registers in your product.
*              2) If your product doesn't have input registers, you could simply set '*err' to
*                 MODBUS_ERR_NONE and return 0.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
CPU_INT16U  MB_InRegRd (CPU_INT16U   reg,
                        CPU_INT16U  *perr)
{
    CPU_INT16U  val;
    CPU_SR      cpu_sr;


    switch (reg) {
        case 10:
             val = (CPU_INT16U)OSCPUUsage;
             break;

        case 11:
             val = (CPU_INT16U)OSCtxSwCtr;
             break;

        case 12:
             val = (CPU_INT16U)(OSTime >> 16);
             break;

        case 13:
             val = (CPU_INT16U)(OSTime & 0x0000FFFF);
             break;

        case 14:
             val = (CPU_INT16U)MB_ChSize;
             break;

        case 15:
             val = (CPU_INT16U)(MB_TotalRAMSize & 0x0000FFFF);
             break;

        default:
             val = 0;
             break;
    }
    *perr = MODBUS_ERR_NONE;
    return (val);
}
#endif

/*
*********************************************************************************************************
*                     GET THE VALUE OF A SINGLE 'FLOATING-POINT' INPUT REGISTER
*
* Description: This function reads the value of a single Input Register.
*              It is called by 'MBS_FC04_InRegRd()' when the argument 'reg' is ABOVE or equal to the
*              value set the configuration constant MODBUS_CFG_FP_START_IX (see MB_CFG.H).
*              You must 'map' the Input Register to the actual application's corresponding floating-point
*              register.
*
* Arguments  : reg       is the Input Register number that needs to be read.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified input register is valid and your code is
*                                            returning its current value.
*                        MODBUS_ERR_RANGE    the specified input register is an invalid number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'reg' argument.
*
* Note(s)    : 1) You can perform the mapping of input register number to the application's input registers
*                 directly in this function or via a table lookup.  A table lookup would make sense if you
*                 had a lot of Input Registers in your product.
*              2) If your product doesn't have input registers, you could simply set '*err' to
*                 MODBUS_ERR_NONE and return (CPU_FP32)0.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FP_EN   == DEF_ENABLED)
#if (MODBUS_CFG_FC04_EN == DEF_ENABLED)
CPU_FP32  MB_InRegRdFP (CPU_INT16U   reg,
                        CPU_INT16U  *perr)
{
    (void)reg;
    *perr = MODBUS_ERR_NONE;
    return ((CPU_FP32)0);
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                             GET THE VALUE OF A SINGLE HOLDING REGISTER
*
* Description: This function reads the value of a single Holding Register.
*              It is called by 'MBS_FC03_HoldingRegRd()' when the argument 'reg' is BELOW the value set
*              by the configuration constant MODBUS_CFG_FP_START_IX (see MB_CFG.H).
*              You must 'map' the Holding Register to the actual application's corresponding integer register.
*
* Arguments  : reg       is the Holding Register number that needs to be read.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified holding register is valid and your code is
*                                            returning its current value.
*                        MODBUS_ERR_RANGE    the specified holding register is an invalid number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'reg' argument.
*
* Note(s)    : 1) You can perform the mapping of holding register number to the application's holding
*                 registers directly in this function or via a table lookup.  A table lookup would make
*                 sense if you had a lot of Holding Registers in your product.
*              2) If your product doesn't have holding registers, you could simply set '*err' to
*                 MODBUS_ERR_NONE and return 0.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
CPU_INT16U  MB_HoldingRegRd (CPU_INT16U   reg,
                             CPU_INT16U  *perr)
{
    CPU_INT16U  reg_val;
    CPU_SR      cpu_sr;

    /***********************************************
    * ������ ����ȷ�ϣ�ͨѶǰ�Ƚ�MB_DATA_ACC_PASSWORDд��reg0
    */
    if ( Ctrl.Para.dat.Password != MB_DATA_ACC_PASSWORD ) {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
        return 0;
    }
        
    //CPU_INT16U *preg       = (CPU_INT16U *)& Ctrl.Sen.H.AdcValue;//Para.buf2;
    CPU_INT16U *preg       = (CPU_INT16U *)& Ctrl.Para.buf2[0];
    /***********************************************
    * ������ ��ȡֵ
    */
    if ( reg < sizeof(Ctrl) / 2 ) {
        reg_val = preg[reg];
        *perr = MODBUS_ERR_NONE;
    } else {
        reg_val = 0;
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
    }
    
    return (reg_val);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                     GET THE VALUE OF A SINGLE 'FLOATING-POINT' HOLDING REGISTER
*
* Description: This function reads the value of a single Floating-Point Holding Register.
*              It is called by 'MBS_FC03_HoldingRegRd()' when the argument 'reg' is ABOVE or equal to the
*              value set by the configuration constant MODBUS_CFG_FP_START_IX (see MB_CFG.H).
*              You must 'map' the Holding Register to the actual application's corresponding floating-point
*              register.
*
* Arguments  : reg       is the Holding Register number that needs to be read.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified holding register is valid and your code is
*                                            returning its current value.
*                        MODBUS_ERR_RANGE    the specified holding register is an invalid number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'reg' argument.
*
* Note(s)    : 1) You can perform the mapping of holding register number to the application's holding
*                 registers directly in this function or via a table lookup.  A table lookup would make
*                 sense if you had a lot of Holding Registers in your product.
*              2) If your product doesn't have holding registers, you could simply set '*err' to
*                 MODBUS_ERR_NONE and return 0.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FP_EN   == DEF_ENABLED)
#if (MODBUS_CFG_FC03_EN == DEF_ENABLED)
CPU_FP32  MB_HoldingRegRdFP (CPU_INT16U   reg,
                             CPU_INT16U  *perr)
{
    (void)reg;    
    
    /***********************************************
    * ������ ����ȷ�ϣ�ͨѶǰ�Ƚ�MB_DATA_ACC_PASSWORDд��reg0
    */
    if ( Ctrl.Para.dat.Password != MB_DATA_ACC_PASSWORD ) {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
        return 0;
    }
    /***********************************************
    * ������ ����Ĵ���
    */
    CPU_FP32   *preg        = (CPU_FP32 *)&Ctrl.Sen.Para.buf3[0];
    reg        = reg - MODBUS_CFG_FP_START_IX;
    CPU_FP32   reg_val      = 0;
    
    /***********************************************
    * ������ ��ȡֵ
    */
    if ( reg < sizeof(UnionSenPara) / 4 ) {
        preg    += reg;
        reg_val     = *preg;
        *perr = MODBUS_ERR_NONE;
    } else {
        *perr = MODBUS_ERR_RANGE;
    }
    
    return ((CPU_FP32)reg_val);
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                            SET THE VALUE OF A SINGLE HOLDING REGISTER
*
* Description: This function is called to change the value of a single Integer Holding Register.
*              It is called by 'MBS_FC06_HoldingRegWr()' and 'MBS_FC16_HoldingRegWrMultiple()' when the argument
*              'reg' is BELOW to the value set by the configuration constant MODBUS_CFG_FP_START_IX (see MB_CFG.H).
*              You must 'map' the Holding Register to the actual application's corresponding integer register.
*
* Arguments  : reg       is the Holding Register number that needs to be read.
*
*              reg_val   is the desired value of the holding register.
*                        The value is specified as an unsigned integer even though it could actually be
*                        represented by a signed integer.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified holding register is valid and your code is
*                                            returning its current value.
*                        MODBUS_ERR_RANGE    the specified holding register is an invalid number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'reg' argument.
*                        MODBUS_ERR_WR       if the device is not able to write or accept the value
*
* Note(s)    : 1) You can perform the mapping of holding register number to the application's holding
*                 registers directly in this function or via a table lookup.  A table lookup would make
*                 sense if you had a lot of Holding Registers in your product.
*              2) If your product doesn't have holding registers, you could simply set '*err' to
*                 MODBUS_ERR_NONE and return 0.
*********************************************************************************************************
*/
#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC16_EN == DEF_ENABLED)
void  MB_HoldingRegWr (CPU_INT16U   reg,
                       CPU_INT16U   reg_val,
                       CPU_INT16U  *perr)
{
#if 0
    /* Access to your variable here! */
    (void)reg;
    (void)reg_val;
    
    /***********************************************
    * ������ ����ȷ�ϣ�ͨѶǰ�Ƚ�MB_DATA_ACC_PASSWORDд��reg0
    */
    if ( ( Ctrl.Para.dat.Password == MB_DATA_ACC_PASSWORD ) ||
         ( ( reg == 0 ) && ( reg_val == MB_DATA_ACC_PASSWORD ) ) ) {
    } else {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
        return;
    }
    
    CPU_INT16U *preg       = (CPU_INT16U *)& Ctrl.Para.buf2[0];
     
    /***********************************************
    * ������ ��ȡֵ
    */
    if ( reg < sizeof(Ctrl) / 2 ) {
        preg    += reg;
        /***********************************************
        * ������ д�����ģ��У׼����
        */    
        extern BOOL App_SetParaToSensor(INT08U dev, INT32U addr, INT08U *dat, INT16U len);
        if ( preg == &Ctrl.Para.buf2[58] ) {
            if ( ( reg_val & 0x0001 ) == 0x0001 ) {
                NVIC_SystemReset();                        // ������core_cm3.h�ļ��ṩ�ú�����
            }
        } else if ( ( preg > &Ctrl.Para.buf2[0] ) &&
             ( preg <= &Ctrl.Para.buf2[127] ) ) {
            BSP_I2CSetPort(2);
            
            INT16U addr = (preg - &Ctrl.Para.buf2[0])*2;
            //INT08U *pb  = (INT08U *)preg;
            
            if ( !BSP_EEP_WriteINT16U (addr, reg_val ) ) {
            //if ( !App_SetParaToSensor(2, addr, NULL, 2) ) {
                *perr = MODBUS_ERR_ILLEGAL_DATA_VAL;
                return;
            }
        /***********************************************
        * ������ д�봫����ģ��У׼����
        */
        } else if ( ( preg >= &Ctrl.Sen.Para.buf2[0] ) &&
                    ( preg <= &Ctrl.Sen.Para.buf2[128] ) ) {
            BSP_I2CSetPort(1);
            INT16U addr = (preg - Ctrl.Sen.Para.buf2)*2;
            //INT08U *pb  = (INT08U *)preg;
            
            //if ( App_SetParaToEep(addr, NULL, 2 ) ) {
            if ( !BSP_EEP_WriteINT16U (addr, reg_val ) ) {
                *perr = MODBUS_ERR_ILLEGAL_DATA_VAL;
                BSP_I2CSetPort(2);
                return;
            }
            BSP_I2CSetPort(2);
//        } else if ( ( preg > (INT16U *)&Ctrl.Tab ) &&
//             ( preg <= (INT16U *)&Ctrl.Tab.buf2[127] ) ) {
//            BSP_I2CSetPort(2);
//            
//            INT16U addr = (preg - (INT16U *)&Ctrl.Tab.buf2)*2+256;
//            INT08U *pb  = (INT08U *)preg;
//            
//            if ( !BSP_EEP_WriteINT16U (addr, reg_val ) ) {
//            //if ( App_SetParaToEep(addr, NULL, 2 ) ) { 
//                *perr = MODBUS_ERR_ILLEGAL_DATA_VAL;
//                return;
//            }
        /***********************************************
        * ������ д�����ģ��У׼����
        */
        }
        
        *preg       = reg_val;
        
        *perr = MODBUS_ERR_NONE;
    } else {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
    }    
#endif
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                     SET THE VALUE OF A SINGLE 'FLOATING-POINT' HOLDING REGISTER
*
* Description: This function is called to change the value of a single Floating-Point Holding Register.
*              It is called by 'MBS_FC06_HoldingRegWr()' and 'MBS_FC16_HoldingRegWrMultiple()' when the argument
*              'reg' is ABOVE or equal to the value set by the configuration constant MODBUS_CFG_FP_START_IX
*              (see MB_CFG.H).
*              You must 'map' the Holding Register to the actual application's corresponding floating-point
*              register.
*
* Arguments  : reg       is the Holding Register number that needs to be read.
*
*              reg_val   is the desired value of the holding register.
*                        The value is specified as an unsigned integer even though it could actually be
*                        represented by a signed integer.
*
*              perr      is a pointer to an error code variable.  You must either return:
*
*                        MODBUS_ERR_NONE     the specified holding register is valid and your code is
*                                            returning its current value.
*                        MODBUS_ERR_RANGE    the specified holding register is an invalid number in your
*                                            application (i.e. product).  YOUR product defines what the
*                                            valid range of values is for the 'reg' argument.
*                        MODBUS_ERR_WR       if the device is not able to write or accept the value
*
* Note(s)    : 1) You can perform the mapping of holding register number to the application's holding
*                 registers directly in this function or via a table lookup.  A table lookup would make
*                 sense if you had a lot of Holding Registers in your product.
*              2) If your product doesn't have holding registers, you could simply set '*err' to
*                 MODBUS_ERR_NONE and return 0.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FP_EN    == DEF_ENABLED)
#if (MODBUS_CFG_FC06_EN == DEF_ENABLED) || \
    (MODBUS_CFG_FC16_EN == DEF_ENABLED)
void  MB_HoldingRegWrFP (CPU_INT16U   reg,
                         CPU_FP32     reg_val_fp,
                         CPU_INT16U  *perr)
{
    (void)reg;
    (void)reg_val_fp;
        
    /***********************************************
    * ������ ����ȷ�ϣ�ͨѶǰ�Ƚ�MB_DATA_ACC_PASSWORDд��reg0
    */
    if ( Ctrl.Para.dat.Password != MB_DATA_ACC_PASSWORD ) {
        *perr = MODBUS_ERR_ILLEGAL_DATA_ADDR;
        return;
    }
    
    /***********************************************
    * ������ ����Ĵ���
    */
    CPU_FP32   *preg      = (CPU_FP32 *)&Ctrl.Sen.Para.buf3[0];
    reg        = reg - MODBUS_CFG_FP_START_IX;
    /***********************************************
    * ������ ��ȡֵ
    */
    if ( reg < sizeof(UnionSenPara) / 4 ) {
        preg    += reg;
        *preg    = reg_val_fp;
        *perr = MODBUS_ERR_NONE;
    } else {
        *perr = MODBUS_ERR_RANGE;
    }
}
#endif
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                              GET A SINGLE ENTRY FROM A RECORD IN A FILE
*
* Description: This function is called to read a single integer from a file.
*              As mentionned in the Modbus specifications, a file is an organization of records.
*              Each file can contain up to 10,000 records (addressed from 0 to 9999).
*              You must 'map' the File/Record/Ix to the actual application's corresponding data.
*
* Arguments  : file_nbr    is the number of the desired file.
*
*              record_nbr  is the desired record within the file
*
*              ix          is the desired entry in the specified record.
*
*              record_len  is the desired length of the record.  Note that this parameter is passed to
*                          this function to provide the 'requested' requested length from the MODBUS command.
*
*              perr        is a pointer to an error code variable.  You must either return:
*
*                          MODBUS_ERR_NONE     the specified file/record/entry is valid and your code is
*                                              returning its current value.
*                          MODBUS_ERR_FILE     if the specified 'file_nbr' is not a valid file number in
*                                              your product.
*                          MODBUS_ERR_RECORD   if the specified 'record_nbr' is not a valid record in the
*                                              specified file.
*                          MODBUS_ERR_IX       if the specified 'ix' is not a valid index into the specified
*                                              record.
*
* Note(s)    : 1) You can perform the mapping of file/record/ix to the application's data directly in
*                 this function or via a table lookup.  A table lookup would make sense if you had a lot
*                 data in your files.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC20_EN == DEF_ENABLED)
CPU_INT16U  MB_FileRd (CPU_INT16U   file_nbr,
                       CPU_INT16U   record_nbr,
                       CPU_INT16U   ix,
                       CPU_INT08U   record_len,
                       CPU_INT16U  *perr)
{
    (void)file_nbr;
    (void)record_nbr;
    (void)ix;
    (void)record_len;
    *perr  = MODBUS_ERR_NONE;
    return (0);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                               SET A SINGLE ENTRY OF A RECORD IN A FILE
*
* Description: This function is called to change a single integer value in a file.
*              As mentionned in the Modbus specifications, a file is an organization of records.
*              Each file can contain up to 10,000 records (addressed from 0 to 9999).
*              You must 'map' the File/Record/Ix to the actual application's corresponding data.
*
* Arguments  : file_nbr    is the number of the desired file.
*
*              record_nbr  is the desired record within the file
*
*              ix          is the desired entry in the specified record.
*
*              record_len  is the desired length of the record.  Note that this parameter is passed to
*                          this function to provide the 'requested' requested length from the MODBUS command.
*
*              val         is the new value to place in the file.
*
*              perr        is a pointer to an error code variable.  You must either return:
*
*                          MODBUS_ERR_NONE     the specified file/record/entry is valid and your code is
*                                              returning its current value.
*                          MODBUS_ERR_FILE     if the specified 'file_nbr' is not a valid file number in
*                                              your product.
*                          MODBUS_ERR_RECORD   if the specified 'record_nbr' is not a valid record in the
*                                              specified file.
*                          MODBUS_ERR_IX       if the specified 'ix' is not a valid index into the specified
*                                              record.
*
* Note(s)    : 1) You can perform the mapping of file/record/ix to the application's data directly in
*                 this function or via a table lookup.  A table lookup would make sense if you had a lot
*                 data in your files.
*********************************************************************************************************
*/

#if (MODBUS_CFG_FC21_EN == DEF_ENABLED)
void  MB_FileWr (CPU_INT16U   file_nbr,
                 CPU_INT16U   record_nbr,
                 CPU_INT16U   ix,
                 CPU_INT08U   record_len,
                 CPU_INT16U   val,
                 CPU_INT16U  *perr)
{
    (void)file_nbr;
    (void)record_nbr;
    (void)ix;
    (void)record_len;
    (void)val;
    *perr = MODBUS_ERR_NONE;
}
#endif

/***********************************************
* ������ 2015/12/07���ӣ����ڷ�MODBBUSͨ��
*        ��MODBUSͨ�ţ���֡ͷ֡β��ͨ�����ݴ���
*/
#if MB_NONMODBUS_EN == DEF_ENABLED
CPU_BOOLEAN NON_MBS_FCxx_Handler (MODBUS_CH  *pch)
{    
    /***********************************************
    * ������ ���ô������ݴ����ص�����
    */
    extern INT08U APP_CommRxDataDealCB(MODBUS_CH  *pch);
    
    return APP_CommRxDataDealCB(pch);
}
#endif


/***********************************************
* ������ 2016/01/08���ӣ����ڷ�MODBBUS IAP����ͨ��
*/
#if MB_IAPMODBUS_EN == DEF_ENABLED
CPU_BOOLEAN IAP_MBS_FCxx_Handler (MODBUS_CH  *pch)
{    
    /***********************************************
    * ������ ���ô������ݴ����ص�����
    */
    extern INT08U IAP_CommRxDataDealCB(MODBUS_CH  *pch);
    
    return IAP_CommRxDataDealCB(pch);
}
#endif