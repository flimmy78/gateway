/********************************************************************************
**
** �ļ���:     i2c_ds1338.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ʱ��оƬDS1338����,��������������DS1307��DS1338��DS1338ZоƬ
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/7/6 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "stm32l1xx.h"  
#include "i2c_ds1338.h"


/******************************************************************************
                                    �������
******************************************************************************/

Time_Typedef TimeValue;                 /* ����ʱ�仺��ָ�� */

#define EE_SCL_SET() GPIOB->BSRRL |= GPIO_Pin_10;
#define EE_SCL_CLR() GPIOB->ODR   &= ~GPIO_Pin_10;
#define EE_SDA_SET() GPIOB->BSRRL |= GPIO_Pin_11;
#define EE_SDA_CLR() GPIOB->ODR   &= ~GPIO_Pin_11;
#define EE_SDA_RD() GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)

#define EE_DEALY() delay_us(1);
static void delay_us(uint32_t us)
{
	uint32_t i;
	for (i = 0; i < us*3+us/2; i++);
}

static void IIC_Start(void)                                                    /* ��ʼ�ź� */
{
	EE_DEALY();
    EE_SDA_SET();
	EE_DEALY();
    EE_SCL_SET();
    EE_DEALY();
    EE_SDA_CLR();
    EE_DEALY();
    EE_SCL_CLR();	  
}

static void IIC_Stop(void)                                                     /* ֹͣ�ź� */
{
	EE_DEALY();	
    EE_SDA_CLR();
	EE_DEALY();	
    EE_SCL_SET();
    EE_DEALY();	
    EE_SDA_SET();
	EE_DEALY();	 
}

static uint8_t EE_waitack(void)                                                /* �ȴ�Ӧ���ź� */
{
	uint16_t i=0;
	uint8_t status=0;
	static uint8_t ccc=0;
	
	EE_SDA_SET();
	EE_DEALY();
    EE_SCL_SET();

    while(EE_SDA_RD()==1)
	{
		if(i++>500)
		{
			status =1;
			ccc++;
			break;
		}	
	}
	if((i<500)||(EE_SDA_RD()==0))
	{
		status=0;
		ccc=0;
	}
	EE_DEALY();	
    EE_SCL_CLR();
	return status;	
}

static void EE_noack(void)                                                   /* ��Ӧ���ź� */
{
	  EE_SDA_SET();	                                                           /* sda=1 */
	  EE_DEALY();			                                                   /* ��ʱ5us */
	  EE_SCL_SET();	                                                           /* scl=1 */
	  EE_DEALY();			                                                   /* ��ʱ5us */
	  EE_SCL_CLR();	                                                           /* scl=0 */
	  EE_DEALY();			                                                   /* ��ʱ5us */
	  EE_SDA_CLR();	                                                           /* sda=0 */
	  EE_DEALY();
}

static void EE_ack(void)	                                                   /* Ӧ���ź� */
{ 
	  EE_SDA_CLR();	                                                           /* sda=0 */
	  EE_DEALY();			                                                   /* ��ʱ5us */
	  EE_SCL_SET();	                                                           /* scl=1 */
	  EE_DEALY(); 			                                                   /* ��ʱ5us */
	  EE_SCL_CLR();	                                                           /* scl=0 */
	  EE_DEALY();				                                               /* ��ʱ5us */
	  EE_SDA_SET();	                                                           /* sda=1 */
	  EE_DEALY();
}

static void IIC_Ack(uint8_t ack)
{
    if (ack == 0) {
        EE_ack();
    } else {
        EE_noack();
    }
}

static uint8_t IIC_Send_Byte(uint8_t input)	                               /* дһ���ֽ����� */
{
    unsigned char i;
    
    for(i=0;i<8;i++)
    {
        if((input&0x80)>0)
        {
            EE_DEALY();
            EE_SDA_SET();
            EE_DEALY();
            EE_SCL_SET();
            EE_DEALY();
            EE_SCL_CLR();
        }
        else
        {
            EE_DEALY();
            EE_SDA_CLR();
            EE_DEALY();
            EE_SCL_SET();
            EE_DEALY();
            EE_SCL_CLR();
        }
        input=input<<1;
    }	

    return EE_waitack();
}

static uint8_t IIC_Read_Byte(void)		                                   /* ��һ���ֽ����� */
{
    unsigned char num=0xff,mm=0x01,uu=0xfe;
    unsigned char j;
	                                                                           /* EE_SDA_SET(); */
    for(j=0;j<8;j++)
    {
        EE_DEALY();
        EE_SCL_SET();
        EE_DEALY();
        num<<=1;
        if(EE_SDA_RD()==0)
            num=(num&uu);
        else
        {
            num=(num|mm);
        }		
        EE_SCL_CLR();
        EE_DEALY();
    }

    return(num);
}

static void ds1338_i2c_init(void) 
{                 
    GPIO_InitTypeDef GPIO_InitStructure;	
    /*SDA SCL GPIO clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB , ENABLE);

    /*!< Configure AT24LCxxx_I2C pins: SCL SDA */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
} 
        
/*******************************************************************
** ������:      ds1338_write_byte
** ��������:    �Ĵ���д��һ���ֽ�����
** ����:        [in] reg_addr��Ҫ�����Ĵ�����ַ
**              [in] dat��Ҫд�������
** ����:        none
********************************************************************/
void ds1338_write_byte(uint8_t reg_addr, uint8_t dat)
{
	IIC_Start();
	if(!(IIC_Send_Byte(DS1307_Write)))                                        /* ����д������Ӧ��λ */
	{
		IIC_Send_Byte(reg_addr);
		IIC_Send_Byte(dat);
	}
	IIC_Stop();
}

/*******************************************************************
** ������:      ds1338_read_byte
** ��������:    �Ĵ�����ȡһ���ֽ�����
** ����:        [in] reg_addr��Ҫ�����Ĵ�����ַ
** ����:        ��ȡ������ֵ
********************************************************************/
uint8_t ds1338_read_byte(uint8_t reg_addr)
{
	uint8_t rcv = 0;

	IIC_Start();
	if(!(IIC_Send_Byte(DS1307_Write)))                                         /* ����д������Ӧ��λ */
	{
        IIC_Send_Byte(reg_addr);                                               /* ����Ҫ�����ļĴ�����ַ */
        IIC_Start();                                                           /* �������� */
        IIC_Send_Byte(DS1307_Read);                                            /* ���Ͷ�ȡ���� */
        rcv = IIC_Read_Byte();
        IIC_Ack(0x01);                                                         /* ���ͷ�Ӧ���ź� */
	}
	IIC_Stop();
	return rcv;
}

/*******************************************************************
** ������:      ds1338_operate_register
** ��������:    ��ʱ�������Ĵ���������д�����ݻ��߶�ȡ����
**              ����д��n�ֽڻ���������ȡn�ֽ�����
** ����:        [in] reg_addr��Ҫ�����Ĵ�����ʼ��ַ
**              [in] pbuf:   д�����ݻ���
**              [in] num��д����������
**              [in] mode������ģʽ��0��д�����ݲ�����1����ȡ���ݲ���
** ����:        none
********************************************************************/
void ds1338_operate_register(uint8_t reg_addr,uint8_t *pbuf,uint8_t num,uint8_t mode)
{
	uint8_t i;
	if(mode)	                                                               /* ��ȡ���� */
	{
		IIC_Start();
		if(!(IIC_Send_Byte(DS1307_Write)))	                                   /* ����д������Ӧ��λ */
		{
			IIC_Send_Byte(reg_addr);	                                       /* ��λ��ʼ�Ĵ�����ַ */
			IIC_Start();	                                                   /* �������� */
			IIC_Send_Byte(DS1307_Read);	                                       /* ���Ͷ�ȡ���� */
			for(i = 0;i < num;i++)
			{
				*pbuf = IIC_Read_Byte();	                                   /* ��ȡ���� */
				if(i == (num - 1))	IIC_Ack(0x01);	                           /* ���ͷ�Ӧ���ź� */
				else IIC_Ack(0x00);	                                           /* ����Ӧ���ź� */
				pbuf++;
			}
		}
		IIC_Stop();	
	}
	else	                                                                   /* д������ */
	{		 	
		IIC_Start();
		if(!(IIC_Send_Byte(DS1307_Write)))	                                   /* ����д������Ӧ��λ */
		{
			IIC_Send_Byte(reg_addr);	                                           /* ��λ��ʼ�Ĵ�����ַ */
			for(i = 0;i < num;i++)
			{
				IIC_Send_Byte(*pbuf);	                                       /* д������ */
				pbuf++;
			}
		}
		IIC_Stop();
	}
}

/*******************************************************************
** ������:      ds1338_readWrite_time
** ��������:    ��ȡ����д��ʱ����Ϣ
** ����:        [in] mode������ģʽ��0��д�����ݲ�����1����ȡ���ݲ���
** ����:        none
********************************************************************/
void ds1338_readWrite_time(uint8_t mode)
{
	uint8_t Time_Register[8];	                                               /* ����ʱ�仺�� */
	
	if(mode)	                                                               /* ��ȡʱ����Ϣ */
	{
		ds1338_operate_register(Address_second,Time_Register,7,1);	           /* �����ַ��0x00����ʼ��ȡʱ���������� */
		
	    /******�����ݸ��Ƶ�ʱ��ṹ���У��������������******/
		TimeValue.second = Time_Register[0] & Shield_secondBit;	               /* ������ */
		TimeValue.minute = Time_Register[1] & Shield_minuteBit;	               /* �������� */
		TimeValue.hour   = Time_Register[2] & Shield_hourBit;	               /* Сʱ���� */
		TimeValue.week   = Time_Register[3] & Shield_weekBit;	               /* �������� */
		TimeValue.date   = Time_Register[4] & Shield_dateBit;	               /* ������ */
		TimeValue.month  = Time_Register[5] & Shield_monthBit;	               /* ������ */
		TimeValue.year   = Time_Register[6];	                               /* ������ */
	}
	else
	{
	    /******��ʱ��ṹ���и������ݽ���******/
		Time_Register[0] = TimeValue.second | Control_Chip_Run;	               /* �룬����оƬ */
		Time_Register[1] = TimeValue.minute;	                               /* ���� */
		Time_Register[2] = TimeValue.hour | Hour_Mode24;	                   /* Сʱ��24Сʱ�� */
		Time_Register[3] = TimeValue.week;	                                   /* ���� */
		Time_Register[4] = TimeValue.date;	                                   /* ��		 */
		Time_Register[5] = TimeValue.month;	                                   /* �� */
		Time_Register[6] = TimeValue.year;	                                   /* �� */

    #if Chip_Type==1                                                           /* ��������ˣ���ʹ�õ���DS1307оƬ */
		Time_Register[7] = CLKOUT_f32768;	                                   /* Ƶ��������� */
    #else                                                                      /* û���壬��ʹ�õ���DS1338����DS1338ZоƬ */
		Time_Register[7] = CLKOUT_f32768 | OSF_Enable;	                       /* Ƶ��������� */
    #endif

		ds1338_operate_register(Address_second,Time_Register,8,0);	           /* �����ַ��0x00����ʼд��ʱ���������� */
    	ds1338_write_byte(RAM_Address55, CHECK_FLAG);                          /* �����һ��RAM��ַд��ʶ��ֵ */
	}
}

/*******************************************************************
** ������:      ds1338_check
** ��������:    ��⺯�� ��DS1307оƬ��RAM�����һ����ַд��һ�����ݲ��������ж�
**              ���ϴ�д���ֵ��ȣ����ǵ�һ���ϵ磬�������ʼ��ʱ��
** ����:        none
** ����:        0���豸���������ǵ�һ���ϵ�, 1���豸�����������
********************************************************************/
uint8_t ds1338_check(void)
{
	if(ds1338_read_byte(RAM_Address55) == CHECK_FLAG)    return 0;              /* �豸���������ǵ�һ���ϵ� */
	else    return 1;
}

#if 0/* ����flash�ռ䲻������ʱ������������ram��д���� */
/*******************************************************************
** ������:      ds1338_ram_write_data
** ��������:    ���õ�RAMд���ݲ���
** ����:        [in] pbuf��д���ݴ����
**              [in] WRadd����д��ʼ��ַ����Χ��RAM_Address0 ~ RAM_Address55֮�䣬���һλ��ַ��������;
**              [in] num����д�ֽ����ݵ���������Χ��1 ~ 55֮��
** ����:        none
********************************************************************/
void ds1338_ram_write_data(uint8_t* pbuf, uint8_t WRadd, uint8_t num)
{
	uint8_t i;
	uint8_t ADDremain;                                                           /* д���������� */
    
	/******�ж�д�����ݵ���ʼ��ַ��Χ******/
	if(WRadd >= RAM_Address55)  return;                                        /* ���һ��RAM��Ԫ������ֱ���˳� */

	/******�жϷ������ݵ�����Ŀ******/
	if((WRadd + num) >= (RAM_Address55 - 1))    ADDremain = RAM_Address55 - 1 - WRadd;  /* ������Χ��д�����µĿռ� */
	else    ADDremain = num;                                                   /* û�����ռ䣬ֱ��д�� */

	IIC_Start();
	if(!(IIC_Send_Byte(DS1307_Write)))                                         /* ����д������Ӧ���ź� */
	{
		IIC_Send_Byte(WRadd);                                                  /* ����д�������׵�ַ */
		for(i = 0;i < ADDremain;i++)
		{
			IIC_Send_Byte(pbuf[i]);                                           /* д������ */
		}
	}
	IIC_Stop();
}

/*******************************************************************
** ������:      ds1338_ram_read_data
** ��������:    ���õ�RAM�����ݲ���
** ����:        [in] pbuf�������ݴ����
**              [in] WRadd����д��ʼ��ַ����Χ��RAM_Address0 ~ RAM_Address55֮�䣬���һλ��ַ��������;
**              [in] num����д�ֽ����ݵ���������Χ��1 ~ 55֮��
** ����:        none
********************************************************************/
void ds1338_ram_read_data(uint8_t* pbuf, uint8_t WRadd, uint8_t num)
{
	uint8_t i;
	uint8_t ADDremain;

	/******�ж϶�ȡ���ݵ���ʼ��ַ��Χ******/
	if(WRadd >= RAM_Address55)  return;                                        /* ���һ��RAM��Ԫ������ֱ���˳� */

	/******���һ����ַ���������DS1307���ã����Բ������һ����ַ����******/
	if((WRadd + num) >= RAM_Address55)  ADDremain = RAM_Address55 - 1 - WRadd; /* ������Χ�ˣ���ȡ��ʼ��ַ�������ڶ�����ַ�ռ������ */
	else    ADDremain = num;                                                   /* û������ַ��Χ��ȫ����ȡ�� */

	IIC_Start();
	if(!(IIC_Send_Byte(DS1307_Write)))                                         /* ����д������Ӧ���ź� */
	{
		IIC_Send_Byte(WRadd);                                                  /* ���Ͷ�ȡ���ݿ�ʼ�Ĵ�����ַ */
		IIC_Start();
		if(!(IIC_Send_Byte(DS1307_Read)))                                      /* ���Ͷ�ȡ������Ӧ���ź� */
		{
			for(i = 0;i < ADDremain;i++)
			{
				pbuf[i] = IIC_Read_Byte();                                    /* ��ʼ����num������ */
				if(i == (ADDremain - 1))    IIC_Ack(0x01);                     /* ��ȡ�����һ�����ݣ����ͷ�Ӧ���ź� */
				else    IIC_Ack(0x00);                                         /* ����Ӧ���ź� */
			}
		}
	}
	IIC_Stop();
}
#endif /* if 0. 2015-7-9 09:06:07 syj */

/*******************************************************************
** ������:      ds1338_init
** ��������:    ʱ��������ʼ��
** ����:        [in] TimeVAL��RTCоƬ�Ĵ���ֵָ��
** ����:        none
********************************************************************/
void ds1338_init(void)
{	
    ds1338_i2c_init();

}

