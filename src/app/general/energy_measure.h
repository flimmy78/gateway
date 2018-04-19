/********************************************************************************
**
** �ļ���:     energy_measure.h
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ���ܼ���
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/31 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef ENERGY_MEASURE_H
#define ENERGY_MEASURE_H



#define   AVR_DIVISOR  50
#define   SAMPLE       32
//------���ܲ���AC or DC----------
#define   MEASURE_AC
//#define  MEASURE_DC

#define V_LEAKAGE

//-------�ο���ѹ5V or 4.096-------
#define REFERENCE_3V3

//----------У׼ϵ��--------------
#define V_CK        	1.002		   //��ѹУ׼ϵ��
//#define  I_CK[4]        0.9990
//----------
#ifdef REFERENCE_5V
#define  INTO_CURRENT(x) (0.005086*(x))           /* (*5/4096)*1000/20/12---rtu09 */
#define  INTO_VOLTAGE(x) (0.366945*(x))	          /* (*5/4096)*150000/10/49.9*(x)---rtu09  */
#endif
#ifdef REFERENCE_4_096V
#define  INTO_CURRENT(x) (0.004167*(x))           /* (*4.096/4096)*1000/20/12---rtu06c */
#define  INTO_VOLTAGE(x) (0.300601*(x))	          /* (*4.096/4096)*150000/10/49.9*(x)---RTU06C */
#endif
#ifdef REFERENCE_3V3
#define  INTO_CURRENT(x) (0.003357*(x))           /* (*3.3/4096)*1000/20/12---rtu09 */
#define  INTO_VOLTAGE(x) (0.242183*(x))	          /* (*3.3/4096)*150000/10/49.9*(x)---rtu09  */
#endif

#ifdef MEASURE_DC
#define  INTO_DC_CURRENT(x)   (0.001299*(x))      /* (*3.3/4096)/31/0.02(x) */
#define  INTO_DC_VOLTAGE(x)   (0.010474*(x))	  /* (*3.3/4096)*13(x) */
#endif

typedef struct {
	float   i_eff_avr;		           /* ������Чֵ */
	float   p_avr;			           /* �й����� */
	//float q_avr;			           /* �޹����� */
	float   s_avr;			           /* ���ڹ��� */
	float   pf;		                   /* �������� */
	float   ws;		                   /* ���� */
    INT32U  t_light;                   /* ����ʱ�� */
} Lamp_power_t;

typedef struct {
	float   v_eff_avr;		           /* ��ѹ��Чֵ */
	float   i_leakage_eff_avr;
	float   v_leakage_eff_avr;
	float   p_all_lamps;
	INT8U   fr;
    Lamp_power_t lamp[LAMP_NUM_MAX];
} Power_t;




extern Power_t g_power;

/*******************************************************************
** ������:      energy_measure_init
** ��������:    ���ܲ���ģ���ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void energy_measure_init(void);

/*******************************************************************
** ������:      energy_measure_power_store
** ��������:    �洢�������ݵ�eeprom��
** ����:        none
** ����:        none
********************************************************************/
void energy_measure_power_store(void);

#endif

