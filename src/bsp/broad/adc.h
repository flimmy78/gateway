/********************************************************************************
**
** �ļ���:     adc.h
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ʵ��ADC����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/31 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef ADC_H
#define ADC_H


#define  SAMPLE_NUM     33

typedef enum {
    AD_CH_LEAKA_V = 0,    /* �ɼ��Ƹ�©��ѹADͨ�� */
    AD_CH_LAMPS_V,        /* �ɼ��ն˵ĵ�ѹADͨ�� */
    AD_CH_LAMP1_I,        /* �ɼ��� 1�ĵ���ADͨ�� */
    AD_CH_MAX
} Ad_channle_e;


typedef struct {
	uint8_t   sample;	
	int16_t   buf[AD_CH_MAX][SAMPLE_NUM];	 /* 16bit,3·AD������35�� */
}Ad_sample_t;



extern Ad_sample_t g_ad_cvs;



void adc_config(uint8_t ADC_SampleTime_Cycles);

void timer2_init(void);

uint8_t adc_timer_start(void);

uint8_t adc_timer_stop(void);

#endif

