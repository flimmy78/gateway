/********************************************************************************
**
** �ļ���:     nrf24l01p.c
** ��Ȩ����:   
** �ļ�����:   ʵ��nrf24l01p����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����       | ����   |  �޸ļ�¼
**===============================================================================
**| 2012/08/14 | ���ѽ� |  �������ļ�
*********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"
#include "nrf24l01.h"

#if EN_NRF24L01 > 0
/*
********************************************************************************
* NRF24L01�Ĵ�����������
********************************************************************************
*/
#define NRF_READ_REG    0x00                                                   /* �����üĴ���,��5λΪ�Ĵ�����ַ */
#define NRF_WRITE_REG   0x20                                                   /* д���üĴ���,��5λΪ�Ĵ�����ַ */
#define RD_RX_PLOAD     0x61                                                   /* ��RX��Ч����,1~32�ֽ� */
#define WR_TX_PLOAD     0xA0                                                   /* дTX��Ч����,1~32�ֽ� */
#define FLUSH_TX        0xE1                                                   /* ���TX FIFO�Ĵ���.����ģʽ���� */
#define FLUSH_RX        0xE2                                                   /* ���RX FIFO�Ĵ���.����ģʽ���� */
#define REUSE_TX_PL     0xE3                                                   /* ����ʹ����һ������,CEΪ��,���ݰ������Ϸ���. */
#define W_ACK_PAYLOAD   0xA8
#define W_TX_PAYLOAD_NO_ACK 0xB0

#define NOP             0xFF                                                   /* �ղ���,����������״̬�Ĵ���	  */

/*
********************************************************************************
* NRF24L01�Ĵ�����ַ
********************************************************************************
*/
#define CONFIG          0x00                                                   /* ���üĴ�����ַ                             */
#define EN_AA           0x01                                                   /* ʹ���Զ�Ӧ����  */
#define EN_RXADDR       0x02                                                   /* ���յ�ַ���� */
#define SETUP_AW        0x03                                                   /* ���õ�ַ���(��������ͨ��) */
#define SETUP_RETR      0x04                                                   /* �����Զ��ط� */
#define RF_CH           0x05                                                   /* RFͨ�� */
#define RF_SETUP        0x06                                                   /* RF�Ĵ��� */
#define STATUS          0x07                                                   /* ״̬�Ĵ��� */
#define OBSERVE_TX      0x08                                                   /* ���ͼ��Ĵ��� */
#define CD              0x09                                                   /* �ز����Ĵ��� */
#define RX_ADDR_P0      0x0A                                                   /* ����ͨ��0���յ�ַ */
#define RX_ADDR_P1      0x0B                                                   /* ����ͨ��1���յ�ַ */
#define RX_ADDR_P2      0x0C                                                   /* ����ͨ��2���յ�ַ */
#define RX_ADDR_P3      0x0D                                                   /* ����ͨ��3���յ�ַ */
#define RX_ADDR_P4      0x0E                                                   /* ����ͨ��4���յ�ַ */
#define RX_ADDR_P5      0x0F                                                   /* ����ͨ��5���յ�ַ */
#define TX_ADDR         0x10                                                   /* ���͵�ַ�Ĵ��� */
#define RX_PW_P0        0x11                                                   /* ��������ͨ��0��Ч���ݿ��(1~32�ֽ�) */
#define RX_PW_P1        0x12                                                   /* ��������ͨ��1��Ч���ݿ��(1~32�ֽ�) */
#define RX_PW_P2        0x13                                                   /* ��������ͨ��2��Ч���ݿ��(1~32�ֽ�) */
#define RX_PW_P3        0x14                                                   /* ��������ͨ��3��Ч���ݿ��(1~32�ֽ�) */
#define RX_PW_P4        0x15                                                   /* ��������ͨ��4��Ч���ݿ��(1~32�ֽ�) */
#define RX_PW_P5        0x16                                                   /* ��������ͨ��5��Ч���ݿ��(1~32�ֽ�) */
#define FIFO_STATUS     0x17                                                   /* FIFO״̬�Ĵ��� */
#define FEATURE         0x1D                                                   /* �����Ĵ��� */

/*
********************************************************************************
* ���ò�������
********************************************************************************
*/
#define PLOAD_WIDTH      32

/* ���üĴ���CONFIG��ز��� */
#define MASK_RX_DR   0x40                                                      /* �����ж����ο���, 0 �������ж�ʹ��;1 �������жϹر� */
#define MASK_TX_DS   0x20                                                      /* �����ж����ο��� */
#define MASK_MAX_RT  0x10                                                      /* ����ط������ж����ο��� */
#define EN_CRC       0x08                                                      /* 0 ���ر�CRC;1 ������CRC */
#define CRCO         0x04                                                      /* CRC�������ã� 0 ��1byte;1 ��2 bytes */
#define PWR_UP       0x02                                                      /* �ض�/����ģʽ����;0 ���ض�ģʽ;1 ������ģʽ */
#define PRIM_RX      0x01                                                      /* ����/�������ã�1 ������ģʽ ;0 ������ģʽ;ֻ����Shutdown��Standby�¸��� */

#define DEF_COFIG    MASK_MAX_RT | EN_CRC | CRCO                               /* ��������/�����жϣ�ʹ��CRCУ�飬CRCΪ2byte */

/* ���üĴ���SETUP_AW��ز��� */
#define AW           3
#define AW_3         0x01                                                      /* 01:3bytes */
#define AW_4         0x10                                                      /* 10:4bytes */
#define AW_5         0x11                                                      /* 11:5bytes */

/* STATUS�Ĵ���bitλ���� */
#define TX_OK   	0x20                                                       /* TX��������ж�*/
#define RX_OK   	0x40  	                                                   /* ���յ������ж� */
/*
********************************************************************************
* �ṹ����
********************************************************************************
*/

typedef struct {
    WORK_MODE_E curmode;
    INT8U src_addr[AW];
    INT8U dest_addr[AW];
} PCB_T;

/*
********************************************************************************
* ��̬����
********************************************************************************
*/
static PCB_T s_pcb = {
    INVALID,
    {0x99, 0x99, 0x99},
    {0x99, 0x99, 0x99}     
};


//24L01������
#define NRF24L01_CE(x)   _spi_ce_select(x)  /* 24L01Ƭѡ�ź� */
#define NRF24L01_CSN(x)  _spi_csn_select(x) /* SPIƬѡ�ź� */   

static void _spi_ce_select(INT8U value)
{
    if (value) {
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
    }
}

static void _spi_csn_select(INT8U value)
{
    if (value) {
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    }
}

/**
  * @brief  ����ָ��SPI������
  * @retval None
  */
static void _spi_gpio_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* PB13->SCK,PB14->MISO,PB15->MOSI */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* PB12->CSN ��ʼ��Ƭѡ������ţ�PB1->CE��ʼ��CE������� */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_1);

    /* PB0->IRQ��ʼ�������ж�IRQ�������� */
    
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;  
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* GPIO_AF_SPI2 */
    /* Connect PXx to sEE_SPI_SCK */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);

    /* Connect PXx to sEE_SPI_MISO */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2); 

    /* Connect PXx to sEE_SPI_MOSI */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); 
}

/*******************************************************************************
* Function Name  : spi2_set_speed
* Description    : SPI2�����ٶ�
* Input          : u8 Speed 
* Output         : None
* Return         : None
*******************************************************************************/
static void spi2_set_speed(INT8U SpeedSet)
{
	SPI2->CR1&=0XFFC7; 
	SPI2->CR1|=SpeedSet;
	SPI_Cmd(SPI2,ENABLE); 
} 

/**
  * @brief  �����ⲿSPI�豸����SPI��ز���
  * @retval None
  */
static void spi_config(void)
{
    SPI_InitTypeDef SPI_InitStruct;
 
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
 
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;            /* SPI����Ϊ˫��˫��ȫ˫�� */
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;		                           /* SPI���� */
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;		                       /* ���ͽ���8λ֡�ṹ */
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;		                               /* ʱ�����յ� */
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;	                               /* ���ݲ����ڵ�1��ʱ���� */
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;		                               /* NSS�ź���������� */
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;		   /* ���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16 */
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;	                           /* ���ݴ����MSBλ��ʼ */
	SPI_InitStruct.SPI_CRCPolynomial = 7;	                                   /* CRCֵ����Ķ���ʽ */

	SPI_Init(SPI2, &SPI_InitStruct);                                           /* ����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ��� */     

    _spi_gpio_config();
 
    SPI_SSOutputCmd(SPI2, ENABLE);
    SPI_Cmd(SPI2, ENABLE);

    spi2_set_speed(SPI_BaudRatePrescaler_8);                                    /* spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��  */
}

static INT8U spi_read_write_byte(INT8U byte)
{
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);             /* �ȴ��������� */
    /* Send byte through the SPI2 peripheral */
    SPI2->DR=byte;
    /* Wait to receive a byte */
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    /* Return the byte read from the SPI bus */
    return SPI2->DR;
}

/*******************************************************************
** ������:      write_regiter
** ��������:    ��24L01�ļĴ���дֵ��һ���ֽڣ�
** ����:        [in] reg   : Ҫд�ļĴ�����ַ
**              [in] value : ���Ĵ���д��ֵ
** ����:        status ״ֵ̬
********************************************************************/
static INT8U write_regiter(INT8U reg,INT8U value)
{
	INT8U status;

	NRF24L01_CSN(0);                                                           /* CSN=0;   */
  	status  = spi_read_write_byte(reg);		                                   /* ���ͼĴ�����ַ,����ȡ״ֵ̬ */
	spi_read_write_byte(value);
	NRF24L01_CSN(1);                                                           /* CSN=1; */

	return status;
}

/*******************************************************************
** ������:      read_regiter
** ��������:    ��24L01�ļĴ���ֵ��һ���ֽڣ�
** ����:        [in] reg : Ҫ���ļĴ�����ַ
** ����:        value �����Ĵ�����ֵ
********************************************************************/
static INT8U read_regiter(INT8U reg)
{
 	INT8U value;

	NRF24L01_CSN(0);                                                           /* CSN=0;   */
  	spi_read_write_byte(reg);			                                       /* ���ͼĴ���ֵ(λ��),����ȡ״ֵ̬ */
	value = spi_read_write_byte(NOP);
	NRF24L01_CSN(1);             	                                           /* CSN=1; */

	return value;
}

/*******************************************************************
** ������:      read_buf
** ��������:    ��24L01�ļĴ���ֵ������ֽڣ�
** ����:        [in]  reg    : �Ĵ�����ַ
**              [out] packet : �����Ĵ���ֵ�Ĵ��ָ��
**              [in]  len    : ����
** ����:        status ״ֵ̬
********************************************************************/
static INT8U read_buf(INT8U reg,INT8U *packet,INT8U len)
{
	INT8U status, u8_ctr;
    
	NRF24L01_CSN(0);                     	                                   /* CSN=0 */
  	status  = spi_read_write_byte(reg);				                           /* ���ͼĴ�����ַ,����ȡ״ֵ̬   	   */
 	for(u8_ctr = 0; u8_ctr < len; u8_ctr++) {
    	packet[u8_ctr] = spi_read_write_byte(0XFF);		                       /* �������� */
    }
	NRF24L01_CSN(1);                 		                                   /* CSN=1 */
	
  	return status;        			                                           /* ���ض�����״ֵ̬ */
}

/*******************************************************************
** ������:      write_buf
** ��������:    ��24L01�ļĴ���ֵ������ֽڣ�
** ����:        [in]  reg    : Ҫд�ļĴ�����ַ
**              [out] packet : д��Ĵ���ֵ�Ĵ��ָ��
**              [in]  len    : ����
** ����:        status ״ֵ̬
********************************************************************/
static INT8U write_buf(INT8U reg, INT8U *packet, INT8U len)
{
	INT8U status, u8_ctr;
    
	NRF24L01_CSN(0);
  	status  = spi_read_write_byte(reg);			                               /* ���ͼĴ���ֵ(λ��),����ȡ״ֵ̬ */
  	for (u8_ctr = 0; u8_ctr < len; u8_ctr++) {
        //printf("%0.2x ", *packet);
    	spi_read_write_byte(*packet++); 				                       /* д������ */
    }
	NRF24L01_CSN(1);
    
  	return status;          		                                           /* ���ض�����״ֵ̬ */
}	

/*******************************************************************
** ������:      nrf24l01_init
** ��������:    nrf24l01ģ���ʼ��
** ����:        [in] rf_ch:  6:0��0000010������оƬ����ʱ���ŵ����ֱ��Ӧ1~125 ����,�ŵ����Ϊ1MHZ, Ĭ��Ϊ02��2402MHz
** ����:        [in] rf_dr:  ���ݴ������ʣ�������Ƶ������Ϊ250kbps ��1Mbps ��2Mbps
** ����:        [in] rf_pwr: ����TX���书��111: 7dBm,  000:-12dBm
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_init(INT8U rf_ch, INT8U rf_dr, INT8U rf_pwr)
{    
    spi_config();

    s_pcb.curmode = INVALID;
    nrf24l01_mode_switch(SHUTDOWN);
    
  	write_regiter(NRF_WRITE_REG+RF_CH, rf_ch);                                 /* ����RFͨ�� �շ�����һ�£�0Ϊ2.4GHz */
  	write_regiter(NRF_WRITE_REG+RF_SETUP, rf_dr | rf_pwr);                     /* ���ô������ʺͷ��͹��� */

    write_regiter(NRF_WRITE_REG+FEATURE, 0x01);                                /* NO ACK ģʽ��ʹ������W_TX_PAYLOAD_NOACK */   
	write_regiter(NRF_WRITE_REG+RX_PW_P0, PLOAD_WIDTH);                        /* ���ý������ݳ��� */
	write_regiter(NRF_WRITE_REG+RX_PW_P1, PLOAD_WIDTH);                        /* ���ý������ݳ��� */

  	write_regiter(NRF_WRITE_REG+EN_AA, 0x00);                                  /* ���н���ͨ�����ر��Զ�Ӧ�� */
  	write_regiter(NRF_WRITE_REG+EN_RXADDR, 0x03);                              /* ʹ��ͨ��0��1�Ľ��յ�ַ */

    if (AW == 3) {
         write_regiter(NRF_WRITE_REG+SETUP_AW, AW_3);                          /* ���õ�ַ����Ϊ3bytes ���䷽/���շ���ַ��� */
    } else if (AW == 4) {
         write_regiter(NRF_WRITE_REG+SETUP_AW, AW_4);                          /* ���õ�ַ����Ϊ3bytes ���䷽/���շ���ַ��� */
    } else if (AW == 5) {
         write_regiter(NRF_WRITE_REG+SETUP_AW, AW_5);                          /* ���õ�ַ����Ϊ3bytes ���䷽/���շ���ַ��� */
    }

	write_regiter(FLUSH_RX, 0xff);                                             /* ���RX FIFO�Ĵ��� */
	write_regiter(FLUSH_TX, 0xff);                                             /* ���TX FIFO�Ĵ��� */

    return nrf24l01_check();
}

/*******************************************************************
** ������:      nrf24l01_check
** ��������:    nrf24l01ģ����
** ����:        ��
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_check(void)
{
	INT8U i, addr[3] = {0xA5, 0xA5, 0xA5};

	//spi2_set_speed(SPI_BaudRatePrescaler_4);                                  /* spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz�� */
	write_buf(NRF_WRITE_REG+TX_ADDR, addr, sizeof(addr));                      /* д��3���ֽڵĵ�ַ */
	read_buf(TX_ADDR, addr, sizeof(addr));                                     /* ����д��ĵ�ַ  */
	
	for(i = 0; i < sizeof(addr); i++) {
        if (addr[i] != 0xA5) {
            break;                                
        }
    }

	if (i != sizeof(addr)) {
        return false;                                                          /* ���24L01���� */
    } else {
    	return true;		                                                   /* ��⵽24L01 */
    }
} 	 

/*******************************************************************
** ������:      nrf24l01_mode_switch
** ��������:    ����ģʽ�л�
** ����:        [in] mode: ����ģʽ
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_mode_switch(WORK_MODE_E mode)
{
    if (s_pcb.curmode == mode) {
        return true;
    }

    NRF24L01_CE(0);                                                            /* ģʽ�л�ǰ������ģʽ�ص�����ģʽ/�ض�ģʽ */
    if (mode == SHUTDOWN) {
        write_regiter(NRF_WRITE_REG+CONFIG, DEF_COFIG);
    } else if (mode == STANDBY) {
        write_regiter(NRF_WRITE_REG+CONFIG, DEF_COFIG | PWR_UP);
    } else if (mode == RX) {
        write_regiter(FLUSH_RX, 0xff);                                         /* ���RX FIFO�Ĵ��� */
        write_regiter(NRF_WRITE_REG+STATUS, 0x4e);                             /* ���RX_DR�жϱ�־ */
        write_regiter(NRF_WRITE_REG+CONFIG, DEF_COFIG | PWR_UP | PRIM_RX);
        NRF24L01_CE(1);
    } else if (mode == IDLE_TX) {
    	write_regiter(FLUSH_TX, 0xff);                                         /* ���TX FIFO�Ĵ��� */
        write_regiter(NRF_WRITE_REG+STATUS, 0x2e);                             /* ���TX_DS�жϱ�־ */
        write_regiter(NRF_WRITE_REG+CONFIG, DEF_COFIG | PWR_UP);
        NRF24L01_CE(1);
    }

    s_pcb.curmode = mode;
    return true;
}

/*******************************************************************
** ������:      nrf24l01_dest_addr_set
** ��������:    Ŀ�ĵ�ַ����
** ����:        [in] addr: ��ַָ��
**              [in] len : ��ַ����
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_dest_addr_set(INT8U* addr, INT8U len)
{
    if(len > AW) {
        return false;
    }

    memcpy(s_pcb.dest_addr, addr, len);

  	write_buf(NRF_WRITE_REG+TX_ADDR, s_pcb.dest_addr, AW);                     /* д����Ŀ�ĵ�ַ */
    return true;
}

/*******************************************************************
** ������:      nrf24l01_src_addr_set
** ��������:    Ŀ�ĵ�ַ����
** ����:        [in] no  : ����ͨ��
**              [in] addr: ��ַָ��
**              [in] len : ��ַ����
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_src_addr_set(INT8U no, INT8U* addr, INT8U len)
{
    INT8U reg;

    if(len > AW) {
        return false;
    }
    
    switch(no)
    {
        case 0:
            reg = NRF_WRITE_REG+RX_ADDR_P0;
            break;
        case 1:
            reg = NRF_WRITE_REG+RX_ADDR_P1;
            break;
        case 2:
            reg = NRF_WRITE_REG+RX_ADDR_P2;
            break;
        case 3:
            reg = NRF_WRITE_REG+RX_ADDR_P3;
            break;
        case 4:
            reg = NRF_WRITE_REG+RX_ADDR_P4;
            break;
        case 5:
            reg = NRF_WRITE_REG+RX_ADDR_P5;
            break;
        default:
            return false;
    }

    memcpy(s_pcb.src_addr, addr, len);
  	write_buf(reg, s_pcb.src_addr, AW);                                        /* д������ַ */
    return true;
}

/*******************************************************************
** ������:      nrf24l01_irq
** ��������:    ����/������ɼ��
** ����:        ��
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_irq(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);                           /* IRQ������������ */
}

/*******************************************************************
** ������:      nrf24l01_packet_send
** ��������:    ���ݷ��ͽӿ�
** ����:        [in] packet: ��������ָ��
**              [in] len   : ���ݳ���
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_packet_send(INT8U* packet, INT8U len)
{
	INT8U state;

    len = len;
    nrf24l01_mode_switch(IDLE_TX);
  	write_buf(W_TX_PAYLOAD_NO_ACK, packet, PLOAD_WIDTH);
	while (nrf24l01_irq() == 1);      									       /* �ȴ�������� */

	state = read_regiter(STATUS);                                              /* ��ȡ״̬�Ĵ��� */	   
	write_regiter(NRF_WRITE_REG+STATUS, state);                                /* ���TX_DS�жϱ�־ */

	if (state&TX_OK) {                                                         /* ���ͳɹ� */
		return true;
	}
    return false;
}

/*******************************************************************
** ������:      nrf24l01_packet_send
** ��������:    ���ݷ��ͽӿ�
** ����:        [in] packet: ��������ָ��
**              [in] len   : ���ݳ���
** ����:        �������ݳ��ȣ�0Ϊ�޽��յ�����
********************************************************************/
INT8U nrf24l01_packet_recv(INT8U* packet, INT8U len)
{
	INT8U state;

    len = len;

	state = read_regiter(STATUS); 	 
    write_regiter(NRF_WRITE_REG+STATUS, state);                                /* ���RX_DR�жϱ�־ */
	if (state&RX_OK) {                                                         /* ���յ����� */
		read_buf(RD_RX_PLOAD, packet, PLOAD_WIDTH);
		return PLOAD_WIDTH; 
	}	   
	return 0;
}

/*******************************************************************
** ������:      nrf24l01_get_signal
** ��������:    ��ȡ�ź�ǿ�ȣ�1��ʾ�ź�ǿ�ȴ�Լ-60dBm
** ����:        ��
** ����:        true or false
********************************************************************/
BOOLEAN nrf24l01_get_signal(void)
{
    INT8U state;
    
	state = read_regiter(CD);                                                  /* ��ȡ�ز����Ĵ��� */	 

    return state&0x01;
}

#endif /* end of EN_NRF24L01 > 0 */

