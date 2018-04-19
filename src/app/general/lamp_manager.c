/********************************************************************************
**
** �ļ���:     lamp_manager.c
** ��Ȩ����:   (c) 2013-2015
** �ļ�����:   �ƹ���ģ��
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/29 | ���ѽ� |  �������ļ�
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
#define EN_STEP_DIMMING     1
#define DIMMING_STEP        5


/*
********************************************************************************
* �ṹ����
********************************************************************************
*/
typedef struct {
    BOOLEAN executed;       /* ִ�б�־λ��0��ʾδִ�У�1��ʾ��ִ�� */
    INT8U prio_event;       /* ��ǰ���ȼ���ߵ��¼���0xff��ʾ��ǰû���¼� */
    INT8U ctl;
    INT8U dimming;
} Lamp_st_t;


typedef struct {
    Lamp_st_t lamp_st[LAMP_NUM_MAX];
} Priv_t;

/*
********************************************************************************
* ��̬����
********************************************************************************
*/
static INT8U s_lamptmr;
static Lamp_event_t s_event[LAMP_NUM_MAX][LAMP_EVENT_MAX];
static Priv_t s_priv;

static void _lamp_ctl(INT8U lamp, INT8U ctl, INT8U dimming)
{
    switch (lamp) {
        case LAMP1_INDEX:
            if (ctl == LAMP_OPEN) {                                            /* ���ƺ͵��� */
                GPIO_ResetBits(GPIOC, GPIO_Pin_4);
                pwm_dimming(lamp, dimming);
                dac_dimming(lamp, dimming);
            } else if (ctl == LAMP_CLOSE) {
                GPIO_SetBits(GPIOC, GPIO_Pin_4);                               /* �صƺ͵���0 */
                pwm_dimming(lamp, 0);
                dac_dimming(lamp, 0);
            }
            break;
        default:
            break;
    }
}

/*******************************************************************
** ������:      _update_lamp_status
** ��������:    ���յ��¼�����µ�ǰ״̬
** ����:        none
** ����:        none
********************************************************************/
static void _update_lamp_status(void)
{
    INT8U i, j;

    for (i = 0; i < LAMP_NUM_MAX; i++) {
        s_priv.lamp_st[i].executed = 0;
        s_priv.lamp_st[i].prio_event = 0xff;
        
        for (j = 0; j < LAMP_EVENT_MAX; j++) {
            if (s_event[i][j].event == 0xff) {
                continue;
            }
            
            /* ��¼��ǰ���ȼ���ߵ��¼� */
            s_priv.lamp_st[i].prio_event = j;
            if (s_event[i][j].ctl == LAMP_OPEN) {
                s_priv.lamp_st[i].ctl = LAMP_OPEN;
            }
            break;
        }
        
    }
}

/*******************************************************************
** ������:     _lamp_manager_tmr
** ��������:   lamp manager��ʱ��
** ����:       [in] index  : ��ʱ������
** ����:       ��
********************************************************************/
static void _lamp_manager_tmr(void *index)
{    
    INT8U i;
    index = index;

    for (i = 0; i < LAMP_NUM_MAX; i++) {
        if (s_priv.lamp_st[i].executed) {
            continue;
        }
        if (s_priv.lamp_st[i].prio_event == 0xff) {
            s_priv.lamp_st[i].executed = 1;
            s_priv.lamp_st[i].ctl = LAMP_CLOSE;
            s_priv.lamp_st[i].dimming = 0;
        } else {
            #if EN_STEP_DIMMING
            if (s_priv.lamp_st[i].dimming + DIMMING_STEP < s_event[i][s_priv.lamp_st[i].prio_event].dimming) {
                s_priv.lamp_st[i].dimming += DIMMING_STEP;
            } else if (s_priv.lamp_st[i].dimming > s_event[i][s_priv.lamp_st[i].prio_event].dimming + DIMMING_STEP) {
                s_priv.lamp_st[i].dimming -= DIMMING_STEP;
            } else {
                s_priv.lamp_st[i].executed = 1;
                s_priv.lamp_st[i].ctl     = s_event[i][s_priv.lamp_st[i].prio_event].ctl;
                s_priv.lamp_st[i].dimming = s_event[i][s_priv.lamp_st[i].prio_event].dimming;
            }
            #else
            s_priv.lamp_st[i].executed = 1;
            s_priv.lamp_st[i].ctl     = s_event[i][s_priv.lamp_st[i].prio_event].ctl;
            s_priv.lamp_st[i].dimming = s_event[i][s_priv.lamp_st[i].prio_event].dimming;
            #endif
        }
        
        _lamp_ctl(i, s_priv.lamp_st[i].ctl, s_priv.lamp_st[i].dimming);
        //SYS_DEBUG("lamp no:[%d], ctl:[%d], dim:[%d]\n", i, s_priv.lamp_st[i].ctl, s_priv.lamp_st[i].dimming);
    }

    for (i = 0; i < LAMP_NUM_MAX; i++) {
        if (!s_priv.lamp_st[i].executed) {
            os_timer_start(s_lamptmr, MILTICK);
            return;
        }
    }
    os_timer_stop(s_lamptmr);
}


static void _handle_0x8201(Comm_pkt_recv_t *packet)
{
    Stream_t rstrm;
    INT8U i, lamp_num, lamp_index;
    Lamp_event_t event;

    stream_init(&rstrm, packet->pdata, packet->len);
    lamp_num = stream_read_byte(&rstrm);
    //event.event = LAMP_EVENT_CENTER;
    event.event = LAMP_EVENT_ALARM;
    for (i = 0; i < lamp_num; i++) {
        lamp_index = stream_read_byte(&rstrm);
        event.ctl  = stream_read_byte(&rstrm);
        if (event.ctl == LAMP_OPEN) {
            event.dimming = stream_read_byte(&rstrm);
        } else {
            event.dimming = 0;
        }

        lamp_event_create((Lamp_e)lamp_index, &event);
    }

    if (packet->ack) {
        INT8U *psendbuf;
        Stream_t wstrm;
        Comm_pkt_send_t send_pkt;

        psendbuf = comm_protocol_asm_status(&wstrm);
        if (psendbuf != NULL) {
            send_pkt.len   = stream_get_len(&wstrm);
            send_pkt.pdata = psendbuf;
            send_pkt.msgid = 0x0201;
            comm_send_dirsend(&send_pkt);
            mem_free(psendbuf);
        }
    }
}

/*
********************************************************************************
* ע��ص�����
********************************************************************************
*/
static const FUNCENTRY_COMM_T s_functionentry[] = {
        0x8201, _handle_0x8201
};
static const INT8U s_funnum = sizeof(s_functionentry) / sizeof(s_functionentry[0]);

/*******************************************************************
** ������:      lamp_manager_init
** ��������:    lamp��ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void lamp_manager_init(void)
{
    INT8U i, j;

    memset(&s_priv, 0, sizeof(Priv_t));
    memset((INT8U*)s_event, 0, sizeof(s_event));

    if (public_para_manager_check_valid_by_id(LAMP_EVENT_)) {
        SYS_DEBUG("LAMP_EVENT_ is valid\n");
        public_para_manager_read_by_id(LAMP_EVENT_, (INT8U*)s_event, sizeof(s_event));
    } else {
        for (i = 0; i < LAMP_NUM_MAX; i++) {
            for (j = 0; j < LAMP_EVENT_MAX; j++) {
                s_event[i][j].event = 0xff;
            }
        }
    }

    _update_lamp_status();
    s_lamptmr = os_timer_create(0, _lamp_manager_tmr);
    os_timer_start(s_lamptmr, 1);

    for (i = 0; i < s_funnum; i++) {
        comm_recv_register(s_functionentry[i].index, s_functionentry[i].entryproc);
    }
}

/*******************************************************************
** ������:      lamp_event_create
** ��������:    ����һ��lamp event
** ����:        [in] lamp    : �ƾ߱��
**              [in] event   : �����¼�ָ��
** ����:        true or false
********************************************************************/
BOOLEAN lamp_event_create(Lamp_e lamp, Lamp_event_t *event)
{
    return_val_if_fail((lamp < LAMP_NUM_MAX) &&
        (event->event < LAMP_EVENT_MAX) && (event->dimming <= 100), FALSE);

    if (memcmp(&s_event[lamp][event->event], event, sizeof(Lamp_event_t)) == 0) {
        return TRUE;                                                           /* �����¼�һ�£����˳����������´洢 */
    }
    
    memcpy(&s_event[lamp][event->event], event, sizeof(Lamp_event_t));
    public_para_manager_store_by_id(LAMP_EVENT_, (INT8U*)s_event, sizeof(s_event));
    _update_lamp_status();

    os_timer_start(s_lamptmr, 1);
    //SYS_DEBUG("lamp_event_create\n");
    return TRUE;
}

/*******************************************************************
** ������:      lamp_ctl
** ��������:    ɾ��һ��lamp event
** ����:        [in] lamp    : �ƾ߱��
**              [in] event   : �����¼�
** ����:        true or false
********************************************************************/
BOOLEAN lamp_event_delete(Lamp_e lamp, Lamp_envnt_e event)
{
    return_val_if_fail((lamp < LAMP_NUM_MAX) && (event < LAMP_EVENT_MAX), FALSE);

    if (s_event[lamp][event].event == 0xff) {
        return TRUE;
    }

    s_event[lamp][event].event = 0xff; 
    public_para_manager_store_by_id(LAMP_EVENT_, (INT8U*)s_event, sizeof(s_event));
    _update_lamp_status();
    
    os_timer_start(s_lamptmr, 1);
    //SYS_DEBUG("lamp_event_delete\n");
    return TRUE;
}

/*******************************************************************
** ������:      lamp_get_status
** ��������:    ��ȡָ���ƺŵ�ǰ��״̬
** ����:        [in] lamp    : �ƾ߱��
** ����:        Lamp_tvent_t
********************************************************************/
Lamp_event_t* lamp_get_status(Lamp_e lamp)
{
    return_val_if_fail((lamp < LAMP_NUM_MAX), NULL);

    if (s_priv.lamp_st[lamp].prio_event == 0xff) {
        return NULL;
    } else {
        return &s_event[lamp][s_priv.lamp_st[lamp].prio_event];
    }
}

/*******************************************************************
** ������:      lamp_is_open
** ��������:    �жϵƿ�/��״̬
** ����:        [in] lamp:
** ����:        true or false
********************************************************************/
BOOLEAN lamp_is_open(Lamp_e lamp)
{
    return_val_if_fail((lamp < LAMP_NUM_MAX), NULL);
    
    if (s_priv.lamp_st[lamp].ctl == LAMP_OPEN) {
        return true;
    } else {
        return false;
    }
}

/*******************************************************************
** ������:      lamp_get_dimming
** ��������:    ��ȡ����ֵ
** ����:        [in] lamp:
** ����:        dimming
********************************************************************/
INT8U lamp_get_dimming(Lamp_e lamp)
{
    return_val_if_fail((lamp < LAMP_NUM_MAX), NULL);
    return s_priv.lamp_st[lamp].dimming;
}

