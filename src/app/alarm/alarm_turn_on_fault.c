/********************************************************************************
**
** �ļ���:     alarm_turn_on_fault.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ���ƹ��ϱ���
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/09/26 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"
#include "alarm.h"
#include "alarm_turn_on_fault.h"

#if DBG_ALARM > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif
/*
********************************************************************************
* ��������
********************************************************************************
*/
#define COUNT_MAX       3

/*
********************************************************************************
* �ṹ����
********************************************************************************
*/
typedef struct {
	INT16U pf_lower_limit;
	INT16U p_higher_limit;  
	INT16U p_lower_limit;  
	INT16U i_higher_limit;
	INT16U i_lower_limit;
} Para_t;

typedef struct {
    Lamp_e lamp;                   /* �ƾ������� */
    INT8S  status;                 /* ����״̬ */
    INT8U  count;
} Priv_t;


static const Para_t c_fault_para[1] = {
    {1000, 0,   40,    0,   400  }   /* �жϿ��ƹ��ϲ�����ֵ */
};

static INT8S _alarm_turn_on_scan(Alarm_t* thiz)
{
	DECL_PRIV(thiz, priv);
	return_val_if_fail(priv != NULL, -1);

    if (priv->lamp < LAMP_NUM_MAX) {
        INT8U index;
    	INT16U lamp_pf = (INT16U)(g_power.lamp[priv->lamp].pf*1000);
    	INT16U lamp_i  = (INT16U)(g_power.lamp[priv->lamp].i_eff_avr*1000);
    	INT16U lamp_p  = (INT16U)(g_power.lamp[priv->lamp].p_avr*10);
        
        if (!lamp_is_open(priv->lamp) || (lamp_get_dimming(priv->lamp) < 30)) {/* �ص�״̬���Ͳ����жϿ��ƹ��ϣ�����ԭ���ı���״̬ */
            return priv->status;
        }
        
        index = 0;
		/* Ҫ<=��>=����Ȼ�����<��>������0<0��0>0������ */
        if ((lamp_pf <= c_fault_para[index].pf_lower_limit)		
           &&(lamp_i <= c_fault_para[index].i_lower_limit)
           &&(lamp_i >= c_fault_para[index].i_higher_limit)			
           &&(lamp_p <= c_fault_para[index].p_lower_limit)
           &&(lamp_p >= c_fault_para[index].p_higher_limit))
        {	
            if (priv->status != 1) {
                if (priv->count++ > COUNT_MAX) {
                    priv->count  = 0;
                    priv->status = 1;
                    SYS_DEBUG("<alarm_turn_on trigger>\n");
                }
            }
        } else {
            if (priv->status == 1) {
                if (priv->count++ > COUNT_MAX) {
                    priv->count  = 0;
                    priv->status = 0;
                    SYS_DEBUG("<alarm_turn_on relieve>\n");
                }
            }
        }
    } else {
        priv->status = -1;                                                     /* -1����δ֪״̬ */
    }

    return priv->status;
}

static INT8S _alarm_turn_on_check(Alarm_t* thiz)
{
	DECL_PRIV(thiz, priv);

	return priv->status;
}

static void _alarm_turn_on_destroy(Alarm_t* thiz)
{
	if (thiz != NULL) {
		//DECL_PRIV(thiz, priv);
		mem_free(thiz);
	}

	return;
}

Alarm_t* alarm_turn_on_create(Alarm_id_e id)
{
	Alarm_t* thiz = (Alarm_t*)mem_malloc(sizeof(Alarm_t) + sizeof(Priv_t));

	if (thiz != NULL) {
		DECL_PRIV(thiz, priv);
        thiz->id       = id;
		thiz->scan     = _alarm_turn_on_scan;
		thiz->check    = _alarm_turn_on_check;
		thiz->destroy  = _alarm_turn_on_destroy;

        if (id == LAMP1_TURN_ON_FAULT) {
            priv->lamp = LAMP1_INDEX;
        } else {
            priv->lamp = LAMP_NUM_MAX;
        }
        priv->status   = 0;
        priv->count    = 0;
	}

	return thiz;
}

