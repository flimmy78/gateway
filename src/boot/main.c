/********************************************************************************
**
** �ļ���:     main.c
** ��Ȩ����:   
** �ļ�����:   main����,�������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/06/12 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "stm32l1xx.h"  
#include "usart.h"
#include "ymodem.h"
#include "common.h"
/*
********************************************************************************
* ����ģ�����ò���
********************************************************************************
*/
#define APPLICATION0_ADDRESS      (uint32_t)0x08002000 
#define APPLICATION1_ADDRESS      (uint32_t)0x08011000
#define USER_FLASH_SIZE           (0xf000)

#define EEPROM_UPDATE_FLAG_ADDR   (0x080807F0)


/*
********************************************************************************
* ����ģ�����
********************************************************************************
*/
uint8_t g_filename[FILE_NAME_LENGTH];

extern void bsp_watchdog_feed(void);

/*******************************************************************
** ������:      _save_update_flag
** ��������:    ����update��־
** ����:        [in] flag
** ����:        none
********************************************************************/
static void _save_update_flag(uint8_t flag)
{
    uint8_t cnt = 0;

    DATA_EEPROM_Unlock();	
Retry:    

    if (DATA_EEPROM_FastProgramByte(EEPROM_UPDATE_FLAG_ADDR, flag) != FLASH_COMPLETE) {
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                       | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR);
        if (cnt++ < 5) {
            goto Retry;
        }
    }				
    DATA_EEPROM_Lock();
}

/*******************************************************************
** ������:      _get_update_flag
** ��������:    ��ȡupdate��־
** ����:        [out] flag
** ����:        none
********************************************************************/
static void _get_update_flag(uint8_t *flag)
{
    DATA_EEPROM_Unlock();	
    while (FLASH_GetStatus() == FLASH_BUSY);
    *flag = *(__IO uint8_t*)EEPROM_UPDATE_FLAG_ADDR;
    DATA_EEPROM_Lock();
}

/*******************************************************************
** ������:      _debug_usart_configuration
** ��������:    ���Դ��ڳ�ʼ��
** ����:        [in] usartx  : ���ں�
**              [in] baudRate: ������
** ����:        none
********************************************************************/
static void _debug_usart_configuration(COM_TypeDef com, uint32_t baudRate)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = baudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    usart_com_init(com, &USART_InitStructure);
}

/*******************************************************************
** ������:      _led_init
** ��������:    led��ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void _led_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Enable the GPIO_LED Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

#if 0
static void RCC_Configuration(void)
{  
  /* Enable HSI Clock */
  RCC_HSICmd(ENABLE);
  
  /*!< Wait till HSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
  {}

  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  
  RCC_MSIRangeConfig(RCC_MSIRange_6);
		
  /* Enable the GPIOs clocks */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC| RCC_AHBPeriph_GPIOD| RCC_AHBPeriph_GPIOE| RCC_AHBPeriph_GPIOH, ENABLE);     

  RCC_HSEConfig(RCC_HSE_OFF);  
  if(RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET )
  {
    while(1);
  }
}
#endif /* if 0. 2016-2-3 20:24:40 suyoujiang */

static void _serial_download(void)
{
  static uint8_t s_tab_1024[1024] = {0};

  uint8_t Number[10] = "          ";
  int32_t Size = 0;

  //FLASH_If_Init();
  SerialPutString("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  Size = Ymodem_Receive(&s_tab_1024[0]);
  if (Size > 0)
  {
    SerialPutString("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
    SerialPutString(g_filename);
    Int2Str(Number, Size);
    SerialPutString("\n\r Size: ");
    SerialPutString(Number);
    SerialPutString(" Bytes\r\n");
    SerialPutString("-------------------\n");
  }
  else if (Size == -1)
  {
    SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (Size == -2)
  {
    SerialPutString("\n\n\rVerification failed!\n\r");
  }
  else if (Size == -3)
  {
    SerialPutString("\r\n\nAborted by user.\n\r");
  }
  else
  {
    SerialPutString("\n\rFailed to receive the file!\n\r");
  }
}

static int _app1_copy_to_app0(void)
{
    uint8_t error = 0;
    uint8_t buf[512];
    uint16_t i, j, max, pkt_len;
    uint32_t flashdest = APPLICATION0_ADDRESS;
    uint32_t flashsrc  = APPLICATION1_ADDRESS;

    max = (USER_FLASH_SIZE % sizeof(buf) == 0) ? 
                USER_FLASH_SIZE / sizeof(buf) :
                USER_FLASH_SIZE / sizeof(buf) + 1;

    for (i = 0; i < max; ) {
        if (i + 1 == max) {                                                    /* ���һ�� */
            if (USER_FLASH_SIZE % sizeof(buf) != 0) {                          /* ���һ������������ */
                pkt_len = USER_FLASH_SIZE % sizeof(buf);
            } else {
                pkt_len = sizeof(buf);                                         /* ���һ���������� */
            }
        } else {
            pkt_len = sizeof(buf);
        }

        for (j = 0; j < pkt_len; j++) {                                        /* ��ȡapp1�д���� */
            buf[j] = *(__IO uint8_t *)flashsrc++;
        }

ENTRY:
        bsp_watchdog_feed();
        if (FLASH_If_Write(&flashdest, (uint32_t*)buf, (uint16_t)pkt_len) == 0) {/* д�뵽boot�� */
            error = 0;
            i++;
        } else {
            /* ��ʱ����ͳ�ƴ������ */
            if (error++ > 5) {
                return 0;
            }            
            comDelay(1000000);
            SerialPutString(" wr error \n\r ");
            goto ENTRY;
        }
    }
    
    return 1;
}

static void _run_to_app(uint32_t aadd)
{
    typedef  void (*pfunction)(void);

    uint32_t jump_addr;
    pfunction jump_to_app;

    /* Jump to user application */
    jump_addr = *(__IO uint32_t*) (aadd + 4);
    jump_to_app = (pfunction) jump_addr;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) aadd);                                         /* ��Ҫ�ֶ�����ջ��ַ���ٷ�����������û������������ */
    jump_to_app();
}

/*******************************************************************
** ������:      _watchdog_Init
** ��������:    ��ʼ���������Ź�
** ����:        none
** ����:        none
********************************************************************/
static void _watchdog_Init(void)
{
    RCC_LSICmd(ENABLE);                                                        /* ��LSI */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);                          /* �ȴ�ֱ��LSI�ȶ� */

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);                              /* ��ʹ�ܣ���Ϊiwdg�ļĴ�����д����,������д��0x5555�����ܲ����Ĵ��� */
    IWDG_SetPrescaler(IWDG_Prescaler_256);                                     /* �������Ź�ʹ���ڲ���������LSI����LSI����256��Ƶ */
    IWDG_SetReload(1023);                                                      /* �趨�������Ź��������ļ���ֵ(0x000~0xFFF;0~4095)����λʱ��Ϊ6s */
    IWDG_ReloadCounter();                                                      /* ���ض������Ź�����������Ĵ���д��0xAAAA�����߸��¼���ֵ */
    IWDG_Enable();                                                             /* �������Ź�����Ĵ���д��0xCCCC���� */
}

/*******************************************************************
** ������:      bsp_watchdog_feed
** ��������:    ���Ź�ι������
** ����:        none
** ����:        none
********************************************************************/
void bsp_watchdog_feed(void)
{
	IWDG_ReloadCounter();
}

/*******************************************************************
** ������:      main
** ��������:    ���������
** ����:        [in] argc :
**              [in] argv :
** ����:        
********************************************************************/
int main(int argc, char* argv[])
{
    uint8_t update_flag = 0xff;

    /* ι����ʱ����Ϊ6s */
    _watchdog_Init();
    
    //RCC_Configuration();
    _debug_usart_configuration(COM1, 115200);
    _led_init();

    if (0 != CkeckKeyDown()) {                                                 /* �������յ��κ����ݣ�����ymodem���� */
        _serial_download();
    }	

    bsp_watchdog_feed();
    _get_update_flag(&update_flag);
    if (update_flag == 1) {                                                    /* ��Ҫ���� */
        /* �������ַ4���ֽ��ǣ�ָ��ջ������˸õ�ַ�϶������ڴ������ڣ���������ֻ������ջ��С��û�취ָ��λ�� */
        /* stm32l1xx�ڴ�ռ���ʵ��ַ��0x20000000������L100оƬram������0x10000=64k */
        /* ��˿���ͨ���ж�ջ����Ч��Χ���������жϳ����Ƿ���Ч */
        if (((*(__IO uint32_t*)APPLICATION1_ADDRESS) & 0x2FFE0000) == 0x20000000) {
            SerialPutString("\n\r _app1_copy_to_app0");            
            if (_app1_copy_to_app0() == 0) {
                SerialPutString(" fail \n\r ");
                NVIC_SystemReset();
            } else {
                update_flag = 0;
                _save_update_flag(update_flag);
                SerialPutString(" success \n\r ");
            }
        }
    }

    bsp_watchdog_feed();
    _run_to_app(APPLICATION0_ADDRESS);

    return 0;
}

