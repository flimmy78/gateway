/********************************************************************************
**
** �ļ���:     spi_w5500.c
** ��Ȩ����:   (c) 2013-201 
** �ļ�����:   SPI��������ʵ��
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/06/12 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "stm32l1xx.h"  
#include "stm32l1xx_spi.h"
#include "sys_swconfig.h"
#if EN_ETHERNET > 0

static void delay_us(uint32_t time)
{
    uint16_t i = 0;

    while(time--) {
        i = 10;                                                                /* ����ʵ��������е��� */
        while(i--);
    }
}

static void delay_ms(uint16_t time)
{
    uint16_t i = 0;

    while(time--) {
        i = 12000;                                                             /* ����ʵ��������е��� */
        while(i--);
    }
}

/**
  * @brief  ʹ��SPIʱ��
  * @retval None
  */
static void SPI_RCC_Configuration(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
}

/**
  * @brief  ����ָ��SPI������
  * @retval None
  */
static void SPI_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;


    /* PB12->CS,PB13->SCK,PB14->MISO,PB15->MOSI */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* ��ʼ��Ƭѡ������� */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    /* ��ʼw5500reset������� */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_SetBits(GPIOC, GPIO_Pin_6);

    /* GPIO_AF_SPI2 */
    /* Connect PXx to sEE_SPI_SCK */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);

    /* Connect PXx to sEE_SPI_MISO */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2); 

    /* Connect PXx to sEE_SPI_MOSI */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); 
}

/**
  * @brief  �����ⲿSPI�豸����SPI��ز���
  * @retval None
  */
void spi_configuration(void)
{
    SPI_InitTypeDef SPI_InitStruct;
 
    SPI_RCC_Configuration();
 
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2,&SPI_InitStruct);
     
    SPI_GPIO_Configuration();
 
    SPI_SSOutputCmd(SPI2, ENABLE);
    SPI_Cmd(SPI2, ENABLE);
}

/**
  * @brief  д1�ֽ����ݵ�SPI����
  * @param  byte д�����ߵ�����
  * @retval None
  */
void spi_write_byte(uint8_t byte)
{          

    while((SPI2->SR&SPI_I2S_FLAG_TXE)==0);                                     /* �ȴ���������          */
    SPI2->DR = byte;                                                           /* ����һ��byte  */
    while((SPI2->SR&SPI_I2S_FLAG_RXNE)==0);                                    /* �ȴ�������һ��byte  */
    SPI2->DR;        
}
/**
  * @brief  ��SPI���߶�ȡ1�ֽ�����
  * @retval ����������
  */
uint8_t spi_read_byte(void)
{   
    while((SPI2->SR&SPI_I2S_FLAG_TXE)==0);                                     /* �ȴ���������              */
    SPI2->DR=0xFF;                                                             /* ����һ�������ݲ����������ݵ�ʱ��  */
    while((SPI2->SR&SPI_I2S_FLAG_RXNE)==0);                                    /* �ȴ�������һ��byte  */
    return SPI2->DR;                             
}
/**
  * @brief  �����ٽ���
  * @retval None
  */
void spi_crisenter(void)
{
    __set_PRIMASK(1);
}
/**
  * @brief  �˳��ٽ���
  * @retval None
  */
void spi_crisexit(void)
{
    __set_PRIMASK(0);
}
 
/**
  * @brief  Ƭѡ�ź�����͵�ƽ
  * @retval None
  */
void spi_cs_select(void)
{
    GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}
/**
  * @brief  Ƭѡ�ź�����ߵ�ƽ
  * @retval None
  */
void spi_cs_deselect(void)
{
    GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

void reset_w5500(void)
{
  GPIO_ResetBits(GPIOC, GPIO_Pin_6);
  delay_us(100);
  GPIO_SetBits(GPIOC, GPIO_Pin_6);
  delay_ms(200);
}
#endif /* end of EN_ETHERNET */
/*********************************END OF FILE**********************************/

