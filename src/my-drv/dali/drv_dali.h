/********************************************************************************
**
** �ļ���:     drv_dali.h
** ��Ȩ����:   (c) 2013-2016 
** �ļ�����:   DALI��������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2016/03/16 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef DRV_DALI_H
#define DRV_DALI_H      1

#define DALI_HOST
#define DALI_BC     0xff00

typedef enum {
	I_DALI_BC_LAMP_OFF = 0,
	I_DALI_BC_LAMP_ON,
	I_DALI_BC_DIM_UP,
	I_DALI_BC_DIM_DOWN,
	I_DALI_BC_IS_LAMP_EXIST,
	I_DALI_BC_IS_LAMP_ERR,	
	I_DALI_CMD_MAX,
} daliCmdIdxType;

typedef enum {
	DALI_LAMP_OFF       = 0x0000,
	DALI_LAMP_ON        = 0x0005,	                                              /* �򿪵�������ֵ */
	DALI_DIM_UP         = 0x0008,                                                 /* ���ٵ���һ�� */
	DALI_DIM_DOWN       = 0x0004,                                                 /* ����һ�� */

	DALI_IS_LAMP_EXIST  = 0xff91,                                                 /* �������Ƿ���� */
	DALI_IS_LAMP_ERR    = 0xff92,

	DALI_ADDR_INI       = 0xa500,                                                 /* 2times between 100ms */
	DALI_ADDR_RAND      = 0XA700,                                                 /* 2times between 100ms */
	DALI_CMP_ADDR_H     = 0xB100,
	DALI_CMP_ADDR_M     = 0XB300,
	DALI_CMP_ADDR_L     = 0XB500,
	DALI_CMP_CMD        = 0XA900,
	DALI_CMP_PHY_SEL    = 0XBD00,
	DALI_SHORT_ADDR_SET = 0XB700,
	DALI_CHK_SHORT_ADDR = 0XB900,
	DALI_ADDR_EXIT      = 0XAB00,
} daliCmdType;

typedef enum {
	DALI_NO_ACTION=0,
    DALI_SENDING_DATA,
    DALI_RECEIVING_DATA,
    DALI_ERR
} dali_action_enum_type;

void drv_dali_init(void);

/*******************************************************************
** ������:      dali_send_data
** ��������:    Send answer to the controller device
** ����:        [in] sendCmd
**              [in] isWaitRecv
** ����:        none
********************************************************************/
void dali_send_data(INT16U sendCmd, INT8U isWaitRecv);

/*******************************************************************
** ������:      dali_send_tick
** ��������:    DALI glbRtuPara physical layer for slave device
** ����:        none
** ����:        none
********************************************************************/
void dali_send_tick(void);

void dali_receive_data(void);

void dali_receive_tick(void);

/*******************************************************************
** ������:      dali_auto_address
** ��������:    �������ӻ������ַ
** ����:        none
** ����:        true or false
********************************************************************/
INT8U dali_auto_address(void);

#endif /* end of DRV_DALI_H */

