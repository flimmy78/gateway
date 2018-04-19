/********************************************************************************
**
** �ļ���:     os_systime.h
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ʵʱʱ�����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/09/06 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef SYSTIME_H
#define SYSTIME_H



typedef INT32U Systime_sec_t;

typedef struct {
	INT8S tm_sec;	/* �� - ȡֵ����Ϊ[0,59] */
	INT8S tm_min;	/* �� - ȡֵ����Ϊ[0,59] */
	INT8S tm_hour;	/* ʱ - ȡֵ����Ϊ[0,23] */
	INT8S tm_mday;	/* һ�����е����� - ȡֵ����Ϊ[1,31] */
	INT8S tm_mon;	/* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
	INT8S tm_year;	/* ��ݣ���ֵ��1900��ʼ */
	INT8S tm_wday;	/* ���ڨCȡֵ����Ϊ[0,6]������0���������죬1��������һ���Դ����� */
	INT16S tm_yday;	/* ��ÿ���1��1�տ�ʼ�������Cȡֵ����Ϊ[0,365]������0����1��1�գ�1����1��2�գ��Դ����� */
	INT8S tm_isdst;	/* ����ʱ��ʶ����ʵ������ʱ��ʱ��tm_isdstΪ������ʵ������ʱ�Ľ���tm_isdstΪ0�����˽����ʱ��tm_isdst()Ϊ����*/ 
} Systime_t;

#if 0/* ��׼���еĶ��� */
    #include <time.h>
    
    int tm_sec;   /* seconds after the minute, 0 to 60
                     (0 - 60 allows for the occasional leap second) */
    int tm_min;   /* minutes after the hour, 0 to 59 */
    int tm_hour;  /* hours since midnight, 0 to 23 */
    int tm_mday;  /* day of the month, 1 to 31 */
    int tm_mon;   /* months since January, 0 to 11 */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* days since Sunday, 0 to 6 */
    int tm_yday;  /* days since January 1, 0 to 365 */
    int tm_isdst; /* Daylight Savings Time flag */
#endif /* if 0. 2015-10-3 14:33:10 suyoujiang */

extern Systime_t g_systime;
extern volatile Systime_sec_t g_systime_sec;
extern volatile INT32U g_run_sec;


void os_systime_init(void);

Ret os_systime_set(Systime_t *tm);

BOOLEAN os_systime_is_valid(void);
//Systime_t *os_systime_convert(const Systime_sec_t *timer, Systime_t *tmbuf);

#endif

