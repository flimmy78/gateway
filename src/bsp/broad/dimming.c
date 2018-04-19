/********************************************************************************
**
** �ļ���:     dimming.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ��������ģ��
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/31 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "stm32l1xx.h"  

/*******************************************************************
** ������:      _dac_init
** ��������:    dac��ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
static void _dac_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef  DAC_InitStructure;

    /* Enable peripheral clocks */
    /* GPIOA Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    /* DAC Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* Once the DAC channel is enabled, the corresponding GPIO pin is automatically 
    connected to the DAC converter. In order to avoid parasitic consumption, 
    the GPIO pin should be configured in analog */
    // DAC1 ->PA4  DAC2->PA5
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* DAC channel1 Configuration */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits8_0;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);
    DAC_Init(DAC_Channel_2, &DAC_InitStructure);

    /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
    automatically connected to the DAC converter. */
    DAC_Cmd(DAC_Channel_1, ENABLE);
    DAC_Cmd(DAC_Channel_2, ENABLE);
}

/*******************************************************************
** ������:      _pwm_init
** ��������:    pwm��ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
static void _pwm_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* GPIOA Clocks enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
}

/*******************************************************************
** ������:      dimming_init
** ��������:    ��ʼ������ģ��
** ����:        none
** ����:        none
********************************************************************/
void dimming_init(void)
{
    /* DAC��ʼ������·DA��������ڵ��� */
    _dac_init();
    _pwm_init();
}

/*******************************************************************
** ������:      pwm_dimming
** ��������:    pwm��ʽ������ýӿ�
** ����:        [in] chn : ͨ����
**              [in] dim : ����ֵ
** ����:        true or false
********************************************************************/
uint8_t pwm_dimming(uint8_t chn, uint8_t dim)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t TimerPeriod = 0;
	uint16_t Channel1Pulse = 0, Channel2Pulse = 0;

	/* Compute the value to be set in ARR regiter to generate signal frequency at 17.57 Khz */
	TimerPeriod = (SystemCoreClock / 400 ) - 1;
	
	/* TIM3 counter disable */
	TIM_Cmd(TIM3, DISABLE);
	
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Channel 1, and 2 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	
	if (chn == 0) {
		if (dim > 0) {
			Channel1Pulse = (uint16_t) (((uint32_t) dim * (TimerPeriod - 1)) / 100);
        } else {
			Channel1Pulse = 0;
        }
		TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	} else if (chn == 1) {
		if (dim > 0) {
			Channel2Pulse = (uint16_t) (((uint32_t) dim * (TimerPeriod - 1)) / 100);
        } else {
			Channel2Pulse = 0;
        }
		TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
		TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	}

	/* TIM3 counter enable */
	TIM_Cmd(TIM3, ENABLE);
	return 0;
}

/*******************************************************************
** ������:      dac_dimming
** ��������:    dac��ʽ������ýӿ�
** ����:        [in] chn : ͨ����
**              [in] dim : ����ֵ
** ����:        true or false
********************************************************************/
uint8_t dac_dimming(uint8_t chn, uint8_t dim)
{
    #define BASEDATA    0xFFF
    
	/* Vout = VREF��DOR/4095 = �ο���ѹ��DAC���ݼĴ���ֵ/4095 */
	uint32_t data;
	data = dim*BASEDATA/100;

	if (chn == 0) {
		DAC_SetChannel1Data(DAC_Align_12b_R, data);
	}else if (chn == 1) {
		DAC_SetChannel2Data(DAC_Align_12b_R, data);
	}
	return 0;
}

