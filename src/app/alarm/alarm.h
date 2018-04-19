/********************************************************************************
**
** �ļ���:     alarm.h
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ��������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/09/24 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef ALARM_H
#define ALARM_H



typedef enum {
    V_LEAKAGE_POLE,               /* �Ƹ�©��ѹ */
    LAMP1_TURN_ON_FAULT,          /* ��1���ƹ��� */
    LAMP1_TURN_OFF_FAULT,         /* ��1�صƹ��� */
    ALARM_ID_MAX
} Alarm_id_e;



struct _Alarm;
typedef struct _Alarm Alarm_t;

typedef INT8S  (*AlarmScan)(Alarm_t* thiz);
typedef INT8S  (*AlarmCheck)(Alarm_t* thiz);
typedef void (*AlarmDestroy)(Alarm_t* thiz);

struct _Alarm {
	AlarmScan     scan;
	AlarmCheck    check;
	AlarmDestroy  destroy;
    Alarm_id_e    id;
	char priv[1];
};

static __inline INT8S alarm_scan(Alarm_t* thiz)
{
	return_val_if_fail(thiz != NULL && thiz->scan != NULL, -1);

	return thiz->scan(thiz);
}

static __inline INT8S alarm_check(Alarm_t* thiz)
{
	return_val_if_fail(thiz != NULL && thiz->check != NULL, -1);

	return thiz->check(thiz);
}

static __inline void alarm_destroy(Alarm_t* thiz)
{
	if(thiz != NULL && thiz->destroy != NULL)
	{
		thiz->destroy(thiz);
	}

	return;
}

#endif

