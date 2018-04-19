/********************************************************************************
**
** �ļ���:     alarm_v_leakage.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ©��ѹ����
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
#include "alarm_v_leakage.h"

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
#define UPPER_LIMIT     36         /* ©��ѹ��������������ֵ */
#define LOWER_LIMIT     34         /* ©��ѹ�������������ֵ */
#define COUNT_MAX       3

/*
********************************************************************************
* �ṹ����
********************************************************************************
*/
typedef struct {
    INT8S status;           /* ����״̬ */
    INT8U count;
} Priv_t;

static INT8S _alarm_v_leakage_scan(Alarm_t* thiz)
{
	DECL_PRIV(thiz, priv);
	return_val_if_fail(priv != NULL, -1);

    if (thiz->id == V_LEAKAGE_POLE) {
        if ((priv->status != 1) && (g_power.v_leakage_eff_avr >= UPPER_LIMIT)) {
            if (priv->count++ > COUNT_MAX) {
                priv->count  = 0;
                priv->status = 1;
                SYS_DEBUG("<alarm_v_leakage trigger>\n");
            }
        } else if ((priv->status == 1) && (g_power.v_leakage_eff_avr <= LOWER_LIMIT)) {
            if (priv->count++ > COUNT_MAX) {
                priv->count  = 0;
                priv->status = 0;
                SYS_DEBUG("<alarm_v_leakage relieve>\n");
            }
        } else {
            priv->count = 0;
        }
    } else {
        priv->status = -1;                                                     /* -1����δ֪״̬ */
    }

    return priv->status;
}

static INT8S _alarm_v_leakage_check(Alarm_t* thiz)
{
	DECL_PRIV(thiz, priv);

	return priv->status;
}

static void _alarm_v_leakage_destroy(Alarm_t* thiz)
{
	if (thiz != NULL) {
		//DECL_PRIV(thiz, priv);
		mem_free(thiz);
	}

	return;
}

Alarm_t* alarm_v_leakage_create(Alarm_id_e id)
{
	Alarm_t* thiz = (Alarm_t*)mem_malloc(sizeof(Alarm_t) + sizeof(Priv_t));

	if (thiz != NULL) {
		DECL_PRIV(thiz, priv);
        thiz->id       = id;
		thiz->scan     = _alarm_v_leakage_scan;
		thiz->check    = _alarm_v_leakage_check;
		thiz->destroy  = _alarm_v_leakage_destroy;
        
        priv->status   = 0;
        priv->count    = 0;
	}

	return thiz;
}

