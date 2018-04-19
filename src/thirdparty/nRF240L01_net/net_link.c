/********************************************************************************
**
** �ļ���:     net_link.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ������·��
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/07/04 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "sys_includes.h"
#include "nrf24l01.h"
#include "net_typedef.h"
#include "net_link_node.h"
#include "net_link_recv.h"
#include "net_link_send.h"

#if DBG_SYSTEM > 0
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



/*
********************************************************************************
* ��̬����
********************************************************************************
*/
static INT8U s_local_addr[ADR_WIDTH] = {
    0x99,           /* ������ַ, �����ֳ���Ҫ���޸�(���ⲻͬ���ڵ��ַһ��) */
};

static const INT8U c_leve1_addr[ADR_WIDTH] = {
    0x00            /* һ�����������ַ���̶������޸� */
};

/*******************************************************************
** ������:      _ack_0x01
** ��������:    ��������Ӧ��
** ����:        [in] node  : �ڵ�
**              [in] result: Ӧ����
** ����:        ��
********************************************************************/
static void _ack_0x01(PACKET_BUF_T *packet, INT8U result)
{
    Stream_t wstrm;

    stream_init(&wstrm, packet->pdata, PLOAD_WIDTH-LINK_HEAD_LEN);
    stream_write_byte(&wstrm, result);
    net_link_send_dirsend(0x81, packet);
}

/*******************************************************************
** ������:      _ack_0x02
** ��������:    ��·����Ӧ��
** ����:        [in] node  : �ڵ�
**              [in] result: Ӧ����
** ����:        ��
********************************************************************/
static void _ack_0x02(PACKET_BUF_T *packet, INT8U result)
{
    Stream_t wstrm;

    stream_init(&wstrm, packet->pdata, PLOAD_WIDTH-LINK_HEAD_LEN);
    stream_write_byte(&wstrm, result);
    net_link_send_dirsend(0x82, packet);
}

/*******************************************************************
** ������:     _hdl_0x01
** ��������:   ��������Э�� 
** ����:       [in]cmd    : �������
**             [in]packet : ����ָ��
** ����:       ��
********************************************************************/
static void _hdl_0x01(INT8U cmd, PACKET_BUF_T *packet)
{
    BOOLEAN isnew;
    Stream_t rstrm;
    Net_link_node_t *node;
    INT8U flowseq, srcaddr[ADR_WIDTH], routeaddr[ADR_WIDTH];
    
    stream_init(&rstrm, packet->buf, PLOAD_WIDTH);
    stream_move_pointer(&rstrm, ADR_WIDTH);
    stream_read_data(&rstrm, srcaddr, ADR_WIDTH);
    stream_move_pointer(&rstrm, 1);                                            /* Э������ */
    stream_read_data(&rstrm, routeaddr, ADR_WIDTH);
    
    flowseq = stream_read_byte(&rstrm);                                        /* ��ȡ������ˮ�ţ��ն�ÿ�����������Զ���һ */

    node = net_link_node_find_exist(srcaddr);
    if (node == NULL) {
        isnew = true;
        node = net_link_node_new();
        if (node == NULL) {
            //_ack_0x01(packet, _FAILURE);                                       /* �ܾ�����Ӧ�� */
            return;
        }

        memcpy(node->addr, srcaddr, ADR_WIDTH);
    } else {
        isnew = false;
        if (node->flowseq == flowseq) {                                        /* ��ˮ��һ�£���ʾ�ն��Ѿ����������������֡������·��ת���� */
            return;
        }
    }

    node->flowseq = flowseq;
    memcpy(node->routeaddr, routeaddr, ADR_WIDTH);

    node->type  = stream_read_byte(&rstrm);                                    /* ��ȡ�ڵ����� */
    node->level = stream_read_byte(&rstrm);                                    /* ��ȡ�ڵ�·�ɴ��� */
    node->node_info.log_cnt++;

    if (isnew == true) {
        net_link_node_add(node);                                               /* ����¼��������ڵ� */
    } else {
        net_link_node_update(srcaddr);
    }

    packet->node = node;
    _ack_0x01(packet, _SUCCESS);                                               /* ��������Ӧ�� */
}    

/*******************************************************************
** ������:     _hdl_0x02
** ��������:   ��·ά������ 
** ����:       [in]cmd    : �������
**             [in]packet : ����ָ��
** ����:       ��
********************************************************************/
static void _hdl_0x02(INT8U cmd, PACKET_BUF_T *packet)
{
    return_if_fail(packet->node != NULL);

#if SEND_MODE == PASSIVE_SEND
    if (net_link_send_inform(packet->node) == true) {                          /* ����ֱ�����ڵ���ն˽ڵ㣬�����ն˽ڵ�ֻ�з��������ݺ��һ��ʱ���ڴ��ڽ���״̬(���͹���)������·�������ֻ�����յ��������ݺ��� */
        return;
    }
#endif

    _ack_0x02(packet, _SUCCESS);
}

/*
********************************************************************************
* ע��ص�����
********************************************************************************
*/
static FUNCENTRY_LINK_T s_functionentry[] = {
        0x01,                   _hdl_0x01
       ,0x02,                   _hdl_0x02
};
static INT8U s_funnum = sizeof(s_functionentry) / sizeof(s_functionentry[0]);

static void _nrf24l01_cfg(void)
{
    Rf_para_t rf_para;

    public_para_manager_read_by_id(RF_PARA_, (INT8U*)&rf_para, sizeof(Rf_para_t));
    SYS_DEBUG("<ch:%d,dr:%x,pwr:%x>\n", rf_para.rf_ch, rf_para.rf_dr, rf_para.rf_pwr);
    
    if (nrf24l01_init(rf_para.rf_ch, rf_para.rf_dr, rf_para.rf_pwr)) {
        SYS_DEBUG("<---nrf24l01 init success>\n");
    } else {
        SYS_DEBUG("<---nrf24l01 init fail>\n");
    }

    nrf24l01_src_addr_set(0, s_local_addr, ADR_WIDTH);                         /* ���ñ�����ַ */
    nrf24l01_src_addr_set(1, (INT8U*)c_leve1_addr, ADR_WIDTH);                 /* ����һ����ַ */
    nrf24l01_mode_switch(RX);
}

/*******************************************************************
** ������:      _para_change_informer
** ��������:    �����仯
** ����:        [in] reason
** ����:        true or false
********************************************************************/
static void _para_change_informer(INT8U reason)
{
    if (reason != PP_REASON_STORE) {
        return;
    }

    _nrf24l01_cfg();
}

/*******************************************************************
** ������:      net_link_init
** ��������:    ��·���ʼ��
** ����:        ��
** ����:        ture or false
********************************************************************/
BOOLEAN net_link_init(void)
{
	INT8U i;

    _nrf24l01_cfg();

    public_para_manager_reg_change_informer(RF_PARA_, _para_change_informer);
    
    net_link_recv_init();
    net_link_send_init();
    net_link_node_init();

    for (i = 0; i < s_funnum; i++) {
        net_link_register(s_functionentry[i].index, s_functionentry[i].entryproc);
    }

    return true;
}

/*******************************************************************
** ������:      net_link_get_local_addr
** ��������:    ��ȡ������ַ
** ����:        ��
** ����:        ������ַָ��
********************************************************************/
INT8U* net_link_get_local_addr(void)
{
    return s_local_addr;
}

