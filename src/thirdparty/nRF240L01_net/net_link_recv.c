/********************************************************************************
**
** �ļ���:     net_link_recv.c
** ��Ȩ����:   (c) 2013-2014 
** �ļ�����:   link ����ģ��
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2014/10/16 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "sys_includes.h"
#include "nrf24l01.h"
#include "net_typedef.h"
#include "net_link_node.h"

//#include "led_ctl.h"

#if DBG_SYSTEM > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif

/*
********************************************************************************
* �궨��
********************************************************************************
*/
#define MAX_REG              8
#define PERIOD_SCAN          1*MILTICK

/*
********************************************************************************
* ����ģ�����ݽṹ
********************************************************************************
*/
typedef struct protocolreg {
    struct protocolreg  *next;
    INT8U type;
    void  (*handler)(INT8U type, PACKET_BUF_T *packet);
} PROTOCOL_REG_T;


/*
********************************************************************************
* ����ģ�����
********************************************************************************
*/
static INT8U s_scantmr;
static PROTOCOL_REG_T s_reg_tbl[MAX_REG];
static PROTOCOL_REG_T *s_usedlist, *s_freelist;
static PACKET_BUF_T   s_packet_buf;


/*******************************************************************
** ������:     _hdl_recv_data
** ��������:   Э�鴦��
** ����:       [in] pframe:
** ����:       ��
********************************************************************/
static void _hdl_recv_data(PACKET_BUF_T *packet_buf)
{
    INT8U type;
    LINK_HEAD_T     *pframe;
    PROTOCOL_REG_T  *curptr;
    Net_link_node_t *node = NULL;

    pframe = (LINK_HEAD_T*)packet_buf->buf;
    type   = pframe->type;

    SYS_DEBUG("<nrf24l01 recv: type:0x%0.2x, addr:%0.2x, signal:%d>\r\n", type, pframe->srcaddr[0], nrf24l01_get_signal());

    if (type == 0x01) {                                                        /* �������� */
        node = NULL;       
    } else {
        node = net_link_node_update(pframe->srcaddr);

        if (node == NULL) {                                                    /* �ڵ㲻���ڣ����������� */
            return;
        }
    }

    packet_buf->node  = node;
    packet_buf->pdata = pframe->data;
    packet_buf->len   = PLOAD_WIDTH - LINK_HEAD_LEN;

    curptr = s_usedlist;
    while (curptr != 0) {
        if (curptr->type == type) {                                            /* ������Ӧ���� */
            return_if_fail(curptr->handler != 0);                              /* ע��Ĵ���������Ϊ�� */
            curptr->handler(type, packet_buf);
            break;
        } else {
            curptr = curptr->next;
        }
    }
}

/*******************************************************************
** ������:     ScanTmrProc
** ��������:   ɨ�趨ʱ��
** ����:       [in] pdata:��ʱ������ֵ
** ����:       ��
********************************************************************/
static void scan_tmr_proc(void *pdata)
{
    (void)pdata;

	if ((nrf24l01_irq() != 0) || (nrf24l01_packet_recv(s_packet_buf.buf, PLOAD_WIDTH) == 0)) {
        return;
	}
    //api_led_ctl(PIN_LED_RED, TURN_ON, 1);                                      /* �յ�������˸led */

    _hdl_recv_data(&s_packet_buf);
}

/*******************************************************************
** ������:     net_link_recv_init
** ��������:   link����ģ���ʼ��
** ����:       ��
** ����:       ��
********************************************************************/
void net_link_recv_init(void)
{
    INT8U i;
    
    for (i = 0; i < MAX_REG - 1; i++) {
        s_reg_tbl[i].next = &s_reg_tbl[i + 1];
        s_reg_tbl[i].type = 0;
        s_reg_tbl[i].handler = 0;
    }
    s_reg_tbl[i].next      = 0;
    s_reg_tbl[i].type      = 0;
    s_reg_tbl[i].handler   = 0;
    
    s_freelist             = &s_reg_tbl[0];
    s_usedlist             = 0;
    
    s_scantmr = os_timer_create((void *)0, scan_tmr_proc);
    os_timer_start(s_scantmr, 1);
}

/*******************************************************************
** ������:     net_link_register
** ��������:   link���մ���ע��
** ����:       [in] type:   Э������
**             [in] handler:Э�鴦����
** ����:       ע��ɹ�����true��ע��ʧ�ܷ���false
********************************************************************/
BOOLEAN net_link_register(INT8U type, void (*handler)(INT8U type, PACKET_BUF_T *packet))
{
    PROTOCOL_REG_T *curptr;

    curptr = s_usedlist;
    while (curptr != 0) {                                                      /* �����ظ�ע�� */
        return_val_if_fail(curptr->type != type, FALSE);
        curptr = curptr->next;
    }

    return_val_if_fail(handler != 0, FALSE);                                   /* ע��Ĵ���������Ϊ�� */
    return_val_if_fail(s_freelist != 0, FALSE);                                /* ע������������ */

    curptr = s_freelist;                                                       /* ��ע�� */
    if (curptr != 0) {
        s_freelist        = curptr->next;
        curptr->next      = s_usedlist;
        s_usedlist        = curptr;
        curptr->type      = type;
        curptr->handler   = handler;                                           /* ������ */
        return true;
    } else {
        return false;
    }
}

