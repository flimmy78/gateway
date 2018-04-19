/********************************************************************************
**
** �ļ���:     general_manager.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ͨ��ҵ�����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/10/01 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"

#if DBG_GENERAL > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif
/*
********************************************************************************
* ��������
********************************************************************************
*/




/*
********************************************************************************
* �ṹ����
********************************************************************************
*/




#if 0
typedef struct {

} Priv_t;
#endif /* if 0. 2015-10-1 22:36:05 suyoujiang */

/*
********************************************************************************
* ��̬����
********************************************************************************
*/
extern Sys_run_info_t g_sys_run_info;

/*******************************************************************
** ������:      _handle_0x8001
** ��������:    ƽ̨ͨ��Ӧ��
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8001(Comm_pkt_recv_t *packet)
{
    Stream_t rstrm;
    INT8U result;
    INT16U ackflowseq, acktypeid;    	

    stream_init(&rstrm, packet->pdata, packet->len);
    ackflowseq = stream_read_half_word(&rstrm);
    acktypeid  = stream_read_half_word(&rstrm);
    result     = stream_read_byte(&rstrm);

    comm_send_confirm(ackflowseq, result);
    
#if 1
    switch (acktypeid) {
        case 0: 
            if (result == 0) {
 
            } else {
 
            }     

        default:
            break;
    }
#endif /* if 0. 2015-10-1 21:20:26 suyoujiang */
}

/*******************************************************************
** ������:      _handle_0x8202
** ��������:    ״̬��ѯ
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8202(Comm_pkt_recv_t *packet)
{
    if (packet->ack) {
        INT8U *psendbuf;
        Stream_t wstrm;
        Comm_pkt_send_t send_pkt;

        psendbuf = comm_protocol_asm_status(&wstrm);
        if (psendbuf != NULL) {
            send_pkt.len   = stream_get_len(&wstrm);
            send_pkt.pdata = psendbuf;
            send_pkt.msgid = 0x0202;
            comm_send_dirsend(&send_pkt);
            mem_free(psendbuf);
        }
    }
}

/*******************************************************************
** ������:      _handle_0x8204
** ��������:    ���ܲ�ѯ
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8204(Comm_pkt_recv_t *packet)
{
    INT8U i, sendbuf[4+4+4+1+1+LAMP_NUM_MAX*20];
    Comm_pkt_send_t send_pkt;

    if (packet->ack) {
        Stream_t wstrm;
        
        stream_init(&wstrm, sendbuf, sizeof(sendbuf));
        stream_write_long(&wstrm, *((INT32U*)&g_power.v_eff_avr));
        stream_write_long(&wstrm, *((INT32U*)&g_power.i_leakage_eff_avr));
        stream_write_long(&wstrm, *((INT32U*)&g_power.v_leakage_eff_avr));
        stream_write_byte(&wstrm, g_power.fr);
        stream_write_byte(&wstrm, LAMP_NUM_MAX);

        for (i = 0; i < LAMP_NUM_MAX; i++) {
            stream_write_long(&wstrm, *((INT32U*)&g_power.lamp[i].i_eff_avr));
            stream_write_long(&wstrm, *((INT32U*)&g_power.lamp[i].p_avr));
            stream_write_long(&wstrm, *((INT32U*)&g_power.lamp[i].s_avr));
            stream_write_long(&wstrm, *((INT32U*)&g_power.lamp[i].ws));
            stream_write_long(&wstrm, *((INT32U*)&g_power.lamp[i].pf));
        }
        
        send_pkt.len   = stream_get_len(&wstrm);
        send_pkt.pdata = sendbuf;
        send_pkt.msgid = 0x0204;
        comm_send_dirsend(&send_pkt);
    }
}

/*******************************************************************
** ������:      _handle_0x8205
** ��������:    ���Բ�ѯ
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8205(Comm_pkt_recv_t *packet)
{
    char *p_ver;
    INT8U sendbuf[3+2*15+12];                                                  /* 15ָ���������Ӳ���汾���г��������Ƴ��� */
    Comm_pkt_send_t send_pkt;
    
    if (packet->ack) {
        Stream_t wstrm;
        
        stream_init(&wstrm, sendbuf, sizeof(sendbuf));
        stream_write_byte(&wstrm, DEVICE_TYPE);                                /* �ն����� */
        p_ver = HARDWARE_VERSION_STR;
        stream_write_byte(&wstrm, strlen(p_ver)); 
        stream_write_string(&wstrm, p_ver);
        
        p_ver = SOFTWARE_VERSION_STR;
        stream_write_byte(&wstrm, strlen(p_ver)); 
        stream_write_string(&wstrm, p_ver);        

        stream_write_long(&wstrm, g_sys_run_info.restartcnt);
        stream_write_long(&wstrm, g_sys_run_info.wdgcnt);
        stream_write_long(&wstrm, g_systime_sec);

        send_pkt.len   = stream_get_len(&wstrm);
        send_pkt.pdata = sendbuf;
        send_pkt.msgid = 0x0205;
        comm_send_dirsend(&send_pkt);
    }
}

/*******************************************************************
** ������:      _handle_0x8208
** ��������:    �ն˿�������
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8208(Comm_pkt_recv_t *packet)
{
    Stream_t rstrm;
    INT8U result = 0, ctlcmd;

    stream_init(&rstrm, packet->pdata, packet->len);
    ctlcmd = stream_read_byte(&rstrm);

    switch (ctlcmd) {
        case 1:
            os_msg_post(TSK_SYS_MSG, MSG_PP_RESET, 0, 0, NULL);
            break;
        case 2:
            os_msg_post(TSK_SYS_MSG, MSG_PP_RESTOREFACTORY, 0, 0, NULL);
            break;            
        case 3:
            
            break;
        default:
            break;
    }

    if (packet->ack) {
        comm_protocol_common_ack(packet->flowseq, 0x8208, result);
    }
}

/*
********************************************************************************
* ע��ص�����
********************************************************************************
*/
static const FUNCENTRY_COMM_T s_functionentry[] = {
        0x8001, _handle_0x8001      /* ƽ̨ͨ��Ӧ�� */
       ,0x8202, _handle_0x8202      /* �ն�״̬��ѯ */
       ,0x8204, _handle_0x8204      /* �ն˵��ܲ�ѯ */
       ,0x8205, _handle_0x8205      /* �ն����Բ�ѯ */
       ,0x8208, _handle_0x8208      /* �ն˿������� */
};
static const INT8U s_funnum = sizeof(s_functionentry) / sizeof(s_functionentry[0]);

/*******************************************************************
** ������:      general_manager_init
** ��������:    ͨ��ҵ������ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void general_manager_init(void)
{
    INT8U i;

#if 0
    memset(&s_priv, 0, sizeof(Priv_t));
    s_lamptmr = os_timer_create(0, _general_manager_tmr);
    os_timer_start(s_lamptmr, 1);
#endif /* if 0. 2015-10-1 20:39:14 suyoujiang */

    for (i = 0; i < s_funnum; i++) {
        comm_recv_register(s_functionentry[i].index, s_functionentry[i].entryproc);
    }
}

/*******************************************************************
** ������:     OS_MsgReset
** ��������:   ��λ����
** ����:       [in]  lpara:
** ����:       ��
********************************************************************/
void OS_MsgReset(INT32U lpara, INT32U hpara, void *p)
{
    energy_measure_power_store();

    public_para_manager_reset_inform(0, NULL, 0, 0);
    __set_PRIMASK(1);
    NVIC_SystemReset();
}

/*******************************************************************
** ������:     OS_MsgRestoreFactory
** ��������:   �ָ���������
** ����:       [in]  lpara:
** ����:       ��
********************************************************************/
void OS_MsgRestoreFactory(INT32U lpara, INT32U hpara, void *p)
{
    public_para_manager_del_all_and_rst();
    __set_PRIMASK(1);
    NVIC_SystemReset();
}

