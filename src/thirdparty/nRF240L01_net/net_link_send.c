/********************************************************************************
**
** �ļ���:     net_link_send.c
** ��Ȩ����:   (c) 2013-2014 
** �ļ�����:   Э���װ���ͣ������������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2014/10/18 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "sys_includes.h"
#include "nrf24l01.h"
#include "net_typedef.h"
#include "net_link.h"
#include "net_link_node.h"
#include "net_link_send.h"

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

#define NUM_MEM              10
#define PERIOD_SEND          2*SECOND
#define OVERTIME             7
/*
********************************************************************************
* ����ģ�����ݽṹ
********************************************************************************
*/
typedef struct {
    INT8U        type;                       /* �������� */
    INT8U        ct_send;                    /* �ط����� */
    INT16U       ct_time;                    /* �ط��ȴ�ʱ����� */
    INT16U       flowtime;                   /* �ط��ȴ�ʱ�� */
    PACKET_BUF_T packet;
    void         (*fp)(INT8U result);        /* ���ͽ��֪ͨ�ص� */
} CELL_T;
/*
********************************************************************************
* ����ģ�����
********************************************************************************
*/
static struct {
    NODE   reserve;
    CELL_T cell;
} s_memory[NUM_MEM];

static INT8U s_sendtmr;
static LIST_T s_sendlist, s_waitlist, s_freelist;

/*******************************************************************
** ������:      net_link_frame_send
** ��������:    ��·֡���ݷ���
** ����:        [in] type : ֡����
**              [in] pbuf : �������ݰ�
** ����:        true or false
********************************************************************/
static BOOLEAN net_link_frame_send(INT8U type, PACKET_BUF_T* pbuf)
{
    INT8U ret;
    INT8U offset;
    Net_link_node_t *node;

    node = pbuf->node;
    nrf24l01_mode_switch(STANDBY);

    memcpy(pbuf->buf, node->addr, ADR_WIDTH);                                  /* Ŀ�ĵ�ַ */

    nrf24l01_dest_addr_set(node->routeaddr, ADR_WIDTH);                        /* ���÷��͵�ַ-·�ɵ�ַ */
    
    offset = ADR_WIDTH;
    memcpy(&pbuf->buf[offset], net_link_get_local_addr(), ADR_WIDTH);
    offset += ADR_WIDTH;
    pbuf->buf[offset] = type;

    ret = nrf24l01_packet_send(pbuf->buf, PLOAD_WIDTH);  
    nrf24l01_mode_switch(RX);
    //api_led_ctl(PIN_LED_GREEN, TURN_ON, 1);                                    /* �յ�������˸led */
    SYS_DEBUG("<nrf24l01 send: type:0x%0.2x, addr:%0.2x, ret:%d>\n", type, node->addr[0], ret);

    return true;
}

/*******************************************************************
** ������:     del_cell
** ��������:   ɾ���ڵ�
** ����:       [in] cell:����ڵ�
** ����:       [in] result:���
** ����:       ��
********************************************************************/
static void del_cell(CELL_T *cell, INT8U result)
{
    void (*fp)(INT8U result);
    
    fp = cell->fp;
    dlist_append_ele(&s_freelist, (LISTMEM *)cell);            
    if (fp != 0) { 
        fp(result);
    }
}

/*******************************************************************
** ������:     send_tmr_proc
** ��������:   ɨ�趨ʱ��
** ����:       [in] pdata:��ʱ������ֵ
** ����:       ��
********************************************************************/
static void send_tmr_proc(void *pdata)
{
    CELL_T *cell, *next;
    
    pdata = pdata;

#if SEND_MODE == PASSIVE_SEND
    if (dlist_item(&s_sendlist) + dlist_item(&s_waitlist) == 0) {
        os_timer_stop(s_sendtmr);
        return;
    }
#else
    if (dlist_item(&s_sendlist) == 0) {
        os_timer_stop(s_sendtmr);
        return;
    }
#endif
    
    os_timer_start(s_sendtmr, PERIOD_SEND);
       
    cell = (CELL_T *)dlist_get_head(&s_sendlist);                              /* ɨ����������ش�ʱ���Ƿ�ʱ */
    for (;;) {
        if (cell == 0) break;
        if (++cell->ct_time > cell->flowtime) {                                /* �ش�ʱ�䳬ʱ */
            cell->ct_time = 0;
            if (--cell->ct_send == 0) {                                        /* ��������ش����� */
                next = (CELL_T *)dlist_del_ele(&s_sendlist,(LISTMEM *)cell);
                del_cell(cell, _OVERTIME);
                cell = next;
                continue;
            } else {
                net_link_send_dirsend(cell->type, &cell->packet);
            }
        }
        cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);
    }
    
#if SEND_MODE == PASSIVE_SEND
    cell = (CELL_T *)dlist_get_head(&s_waitlist);                              /* ɨ����������ش�ʱ���Ƿ�ʱ */
    for (;;) {
        if (cell == 0) break;
        
        if ((cell->ct_send == 0) || (++cell->ct_time > cell->flowtime)) {      /* �ش�ʱ�䳬ʱ */
            cell->ct_time = 0;
            next = (CELL_T *)dlist_del_ele(&s_waitlist, (LISTMEM *)cell);
            SYS_DEBUG("<Over time>\n");
            del_cell(cell, _OVERTIME);
            cell = next;
            continue;
        }
        cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);
    }
#endif
}

/*******************************************************************
** ������:     net_link_send_init
** ��������:   link����ģ���ʼ��
** ����:       ��
** ����:       ��
********************************************************************/
void net_link_send_init(void)
{
    dlist_init(&s_sendlist);
#if SEND_MODE == PASSIVE_SEND    
    dlist_init(&s_waitlist);
#endif
    dlist_mem_init(&s_freelist, (LISTMEM *)s_memory, sizeof(s_memory)/sizeof(s_memory[0]), sizeof(s_memory[0]));
    s_sendtmr = os_timer_create((void *)0, send_tmr_proc);
}

/*******************************************************************
** ������:     net_link_send_listsend
** ��������:   link����������
** ����:       [in] type   : Э������
**             [in] packet : ����ָ��
**             [in] ct_send: ���ʹ���
**             [in] ct_time: �ط��ȴ�ʱ�䣬��λ����
**             [in] fp     : ���ͽ��֪ͨ
** ����:       �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN net_link_send_listsend(INT8U type, PACKET_BUF_T *packet, INT8U ct_send, INT16U ct_time, void(*fp)(INT8U))
{
    CELL_T *cell;         
    Net_link_node_t *node;
   
    return_val_if_fail((packet != NULL) && (packet->node != NULL), false);

#if SEND_MODE == PASSIVE_SEND
    node = packet->node;
    if ((node->type == ENDDEVICE) && (strncmp((char*)node->addr, (char*)node->routeaddr, ADR_WIDTH) == 0)) {
        if ((cell = (CELL_T *)dlist_del_head(&s_freelist)) != 0) {             /* ��������ڵ� */
            memcpy(&cell->packet, packet, sizeof(PACKET_BUF_T));
            
            cell->type     = type;
            cell->ct_send  = ct_send;
            cell->flowtime = OVERTIME;                                         /* �ȴ��ش�ʱ�� */
            cell->ct_time  = 0;
            cell->fp       = fp;

            dlist_append_ele(&s_waitlist, (LISTMEM *)cell);                    /* �������ڵ����������� */

            if (!os_timer_is_run(s_sendtmr)) {
                os_timer_start(s_sendtmr, PERIOD_SEND);
            }
            
            return true;
        } else {
            return false;
        }
    }
#endif

    if ((cell = (CELL_T *)dlist_del_head(&s_freelist)) != 0) {                 /* ��������ڵ� */
        memcpy(&cell->packet, packet, sizeof(PACKET_BUF_T));
        
        cell->type     = type;
        cell->ct_send  = ct_send;
        cell->flowtime = ct_time;                                              /* �ȴ��ش�ʱ�� */
        cell->ct_time  = 0;
        cell->fp       = fp;

        dlist_append_ele(&s_sendlist, (LISTMEM *)cell);                        /* �������ڵ����������� */
        
        if (!os_timer_is_run(s_sendtmr)) {
            os_timer_start(s_sendtmr, PERIOD_SEND);
        }

        net_link_send_dirsend(type, &cell->packet);
        return true;
    } else {
        return false;
    }
}

/*******************************************************************
** ������:     net_link_send_dirsend
** ��������:   ֱ�ӷ��ͣ����ҵ�������
** ����:       [in] type  : Э������
**             [in] packet: ���ݰ�
** ����:       ���ͳɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN net_link_send_dirsend(INT8U type, PACKET_BUF_T *packet)
{
    return_val_if_fail((packet != NULL) && (packet->node != NULL), false);

    net_link_frame_send(type, packet);
    return true;
}

/*******************************************************************
** ������:     net_link_send_confirm
** ��������:   Ӧ��ȷ��
** ����:       [in] type: Э������
**             [in] result: ���
** ����:       �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN net_link_send_confirm(INT8U type, PACKET_BUF_T *packet, INT8U result)
{
    CELL_T  *cell;
    
    result = result;
    
    cell = (CELL_T *)dlist_get_head(&s_sendlist);
    for (;;) {                                                                 /* ���ҷ������� */
        if (cell == 0) break;
        if ((cell->type == type) &&                                            /* ���ҵ�ƥ��Ľڵ� */
            (strncmp((char*)cell->packet.node->addr, (char*)packet->node->addr, ADR_WIDTH) == 0)) {
            dlist_del_ele(&s_sendlist, (LISTMEM *)cell);
            del_cell(cell, _SUCCESS);
            return true;
        } else {
            cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);                  /* ɨ���¸��ڵ� */
        }
    }

#if SEND_MODE == PASSIVE_SEND
    cell = (CELL_T *)dlist_get_head(&s_waitlist);
    for (;;) {                                                                 /* ���ҵȴ����� */
        if (cell == 0) break;
        if ((cell->type == type) &&                                            /* ���ҵ�ƥ��Ľڵ� */
            (strncmp((char*)cell->packet.node->addr, (char*)packet->node->addr, ADR_WIDTH) == 0)) {
            SYS_DEBUG("<send_confirm:addr:%x, type:%0.2x>\n", cell->packet.node->addr[0], type);
            
            dlist_del_ele(&s_waitlist, (LISTMEM *)cell);
            del_cell(cell, _SUCCESS);
            return true;
        } else {
            cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);                  /* ɨ���¸��ڵ� */
        }
    }
#endif
    return false;        
}

#if SEND_MODE == PASSIVE_SEND
/*******************************************************************
** ������:      net_link_send_inform
** ��������:    ֪ͨ�յ�����
** ����:        [in] node:
** ����:        ture or false
********************************************************************/
BOOLEAN net_link_send_inform(Net_link_node_t *node)
{
    CELL_T *cell/*, *next*/;
        
    if (dlist_item(&s_waitlist) == 0) {
        return false;
    }

    cell = (CELL_T *)dlist_get_head(&s_waitlist);                              /* ɨ����������ش�ʱ���Ƿ�ʱ */
    for (;;) {
        if (cell == 0) break;
        if (strncmp((char*)cell->packet.node->addr, (char*)node->addr, ADR_WIDTH) == 0) {
            SYS_DEBUG("<send_inform:addr:%x>\n", node->addr[0]);
            net_link_send_dirsend(cell->type, &cell->packet);
            cell->ct_time = 0;
            --cell->ct_send;
            return true;
        }
        cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);
    }

    return false;
}
#endif

