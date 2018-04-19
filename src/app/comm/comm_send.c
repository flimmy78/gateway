/********************************************************************************
**
** �ļ���:     comm_send.c
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
#include "bsp.h"
#include "sys_includes.h"

#if DBG_COMM > 0
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
#define PERIOD_SEND          SECOND

/*
********************************************************************************
* ����ģ�����ݽṹ
********************************************************************************
*/
typedef struct {
    INT16U          flowseq;
    INT8U           ct_send;                    /* �ط����� */
    INT8U           ct_time;                    /* �ط��ȴ�ʱ����� */
    INT8U           flowtime;                   /* �ط��ȴ�ʱ�� */
    Comm_pkt_send_t packet;
                                               /* ���ͽ��֪ͨ�ص� */
    void           (*fp)(INT16U flowseq, Comm_pkt_send_t *packet, INT8U result);
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
static LIST_T s_sendlist, s_freelist;

extern Sys_base_info_t g_sys_base_info;

/*******************************************************************
** ������:     _get_flow_seq
** ��������:   �õ�����֡��ˮ��
** ����:       ��
** ����:       ����֡��ˮ��(ǿ�ƴ�1��ʼ, 0��ʾ��Ч�ķ�����ˮ��)
********************************************************************/
INT16U _get_flow_seq(void)
{
    static INT16U s_flowseq = 0xffff;

    if (s_flowseq == 0xffff) {
        s_flowseq = 1;
    } else {
        ++s_flowseq;
    }
    return s_flowseq;
}

/*******************************************************************
** ������:      comm_frame_send
** ��������:    ��·֡���ݷ���
** ����:        [in] flowseq : ��ˮ��
**              [in] packet  : �������ݰ�
** ����:        true or false
********************************************************************/
static BOOLEAN _comm_frame_send(INT16U flowseq, Comm_pkt_send_t* packet)
{
    INT8U *pframebuf, *psendbuf, *ptr;
    INT16U framelen, sendlen;
    Stream_t wstrm;
    
    framelen  = packet->len + FRAMEHEAD_LEN + FRAMETAIL_LEN;
    pframebuf = (INT8U*)mem_malloc(framelen);
    
    if (pframebuf == NULL) {
        return false;
    } 

    stream_init(&wstrm, pframebuf, framelen);
    stream_write_half_word(&wstrm, packet->msgid);
    stream_write_half_word(&wstrm, packet->len);
    stream_write_half_word(&wstrm, g_sys_base_info.uid);
    stream_write_half_word(&wstrm, flowseq);
    stream_write_data(&wstrm, packet->pdata, packet->len);
    stream_write_byte(&wstrm, chksum_xor(stream_get_start_pointer(&wstrm), stream_get_len(&wstrm)));

    sendlen = 2*framelen + 2;
    psendbuf = (INT8U*)mem_malloc(sendlen);
    
    if (psendbuf == NULL) {
        mem_free(pframebuf);
        return false;
    } 

    sendlen = assemble_by_rules(psendbuf, pframebuf, framelen, (Asmrule_t *)&g_comm_rules);
    /* ���÷��ͺ��� */
    ptr = psendbuf;

#if EN_ETHERNET > 0
    drv_ethernet_write(psendbuf, sendlen);
#endif

#if EN_ZIGBEE > 0
    drv_zigbee_write(psendbuf, sendlen);
#endif

#if EN_WIFI > 0
    drv_esp8266_write(psendbuf, sendlen);
#endif

    SYS_DEBUG("frame_send len[%d] ", sendlen);
    while (sendlen-- > 0) {
        SYS_DEBUG("%0.2x ", *ptr);
        
#if 0                                                                          /* ��ʱ��ΪTTS������ */
		while (USART_GetFlagStatus(EVAL_COM3, USART_FLAG_TC) == RESET);
		USART_SendData(EVAL_COM3, *ptr);
#endif /* if 0. 2017-8-14 14:32:50 syj */
        ptr++;
    }

    SYS_DEBUG("\n");

    mem_free(pframebuf);
    mem_free(psendbuf);
    
    return true;
}

/*******************************************************************
** ������:     _del_cell
** ��������:   ɾ���ڵ�
** ����:       [in] cell:����ڵ�
** ����:       [in] result:���
** ����:       ��
********************************************************************/
static void _del_cell(CELL_T *cell, INT8U result)
{
    void (*fp)(INT16U flowseq, Comm_pkt_send_t *packet, INT8U result);
    
    fp = cell->fp;
    if (fp != 0) { 
        fp(cell->flowseq, &cell->packet, result);
    }
    mem_free(cell->packet.pdata);
    
    dlist_append_ele(&s_freelist, (LISTMEM *)cell);
}

/*******************************************************************
** ������:     _send_tmr_proc
** ��������:   ɨ�趨ʱ��
** ����:       [in] pdata:��ʱ������ֵ
** ����:       ��
********************************************************************/
static void _send_tmr_proc(void *pdata)
{
    CELL_T *cell, *next;
    
    pdata = pdata;

    if (dlist_item(&s_sendlist) == 0) {
        os_timer_stop(s_sendtmr);
        return;
    }
    
    os_timer_start(s_sendtmr, PERIOD_SEND);
       
    cell = (CELL_T *)dlist_get_head(&s_sendlist);                              /* ɨ����������ش�ʱ���Ƿ�ʱ */
    for (;;) {
        if (cell == 0) break;
        if (++cell->ct_time > cell->flowtime) {                                /* �ش�ʱ�䳬ʱ */
            cell->ct_time = 0;
            if (--cell->ct_send == 0) {                                        /* ��������ش����� */
                next = (CELL_T *)dlist_del_ele(&s_sendlist, (LISTMEM *)cell);
                _del_cell(cell, _OVERTIME);
                cell = next;
                continue;
            } else {
                _comm_frame_send(cell->flowseq, &cell->packet);
            }
        }
        cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);
    }
}

/*******************************************************************
** ������:     comm_send_init
** ��������:   ����ģ���ʼ��
** ����:       ��
** ����:       ��
********************************************************************/
void comm_send_init(void)
{
    dlist_init(&s_sendlist);

    dlist_mem_init(&s_freelist, (LISTMEM *)s_memory, sizeof(s_memory)/sizeof(s_memory[0]), sizeof(s_memory[0]));
    s_sendtmr = os_timer_create((void *)0, _send_tmr_proc);
}

/*******************************************************************
** ������:     comm_send_listsend
** ��������:   link����������
** ����:       [in] packet : ����ָ��
**             [in] ct_send: ���ʹ���
**             [in] ct_time: �ط��ȴ�ʱ�䣬��λ����
**             [in] fp     : ���ͽ��֪ͨ
** ����:       ������ˮ�ţ�0:��ʾ�ҽӷ�������ʧ��
********************************************************************/
INT16U comm_send_listsend(Comm_pkt_send_t *packet, INT8U ct_send, INT16U ct_time, void(*fp)(INT16U, Comm_pkt_send_t*, INT8U))
{
    CELL_T *cell;         
   
    return_val_if_fail((packet != NULL), false);

    if ((cell = (CELL_T *)dlist_del_head(&s_freelist)) != 0) {                 /* ��������ڵ� */
        memcpy(&cell->packet, packet, sizeof(Comm_pkt_send_t));
        cell->packet.pdata = (INT8U*)mem_malloc(cell->packet.len);
        
        if (cell->packet.pdata == NULL) {
            dlist_append_ele(&s_freelist, (LISTMEM *)cell);
            return 0;
        } else {
            memcpy(cell->packet.pdata, packet->pdata, packet->len);
        }
        
        cell->flowseq  = _get_flow_seq();
        cell->ct_send  = ct_send;
        cell->flowtime = ct_time;                                              /* �ȴ��ش�ʱ�� */
        cell->ct_time  = 0;
        cell->fp       = fp;

        dlist_append_ele(&s_sendlist, (LISTMEM *)cell);                        /* �������ڵ����������� */
        
        if (!os_timer_is_run(s_sendtmr)) {
            os_timer_start(s_sendtmr, PERIOD_SEND);
        }

        _comm_frame_send(cell->flowseq, &cell->packet);
        return cell->flowseq;
    } else {
        return 0;
    }
}

/*******************************************************************
** ������:     comm_send_dirsend
** ��������:   ֱ�ӷ��ͣ����ҵ�������
** ����:       [in] packet : ���ݰ�
** ����:       ���ͳɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN comm_send_dirsend(Comm_pkt_send_t *packet)
{
    return_val_if_fail((packet != NULL), false);

    _comm_frame_send(_get_flow_seq(), packet);

    return true;
}

/*******************************************************************
** ������:     comm_send_confirm
** ��������:   Ӧ��ȷ��
** ����:       [in] flowseq : ��ˮ��
**             [in] result  : ������ɹ� or ʧ��
** ����:       �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN comm_send_confirm(INT16U flowseq, INT8U result)
{
    CELL_T  *cell;
    
    result = result;
    
    cell = (CELL_T *)dlist_get_head(&s_sendlist);
    for (;;) {                                                                 /* ���ҵȴ����� */
        if (cell == 0) break;
        if (cell->flowseq == flowseq) {
            dlist_del_ele(&s_sendlist, (LISTMEM *)cell);
            _del_cell(cell, result);
            return true;
        } else {
            cell = (CELL_T *)dlist_next_ele((LISTMEM *)cell);                  /* ɨ���¸��ڵ� */
        }
    }
    
    return false;        
}

