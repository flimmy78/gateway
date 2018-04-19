/********************************************************************************
**
** �ļ���:     sys_debugcfg.h
** ��Ȩ����:   (c) 2013-2014 
** �ļ�����:   debugϵͳ����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2016/01/15 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef SYS_DEBUGCFG_H
#define SYS_DEBUGCFG_H        1


/**********************************************************
**              ������Բ���
**********************************************************/
#define EN_DEBUG                    1              /* �ܱ�����Կ��� */

#define DEBUG_UART_NO               COM1           /* COM1~COM3 */
#define DEBUG_UART_BAUD             115200         /* ������ */


/*
*********************************************************
**              ����APP����Կ���
*********************************************************
*/
#if EN_DEBUG > 0

#define DBG_SYSTIME               0        /* ϵͳʱ�� */
#define DBG_PBULIC_PARA           1        /* �������� */
#define DBG_SYSTEM                1        /* ϵͳ���� */
#define DBG_ENERGY_MEASURE        0        /* ����ͳ�� */
#define DBG_ZIGBEE                0        /* ZIGBEE */
#define DBG_WIFI                  1        /* WIFI */
#define DBG_ETHERNET              0        /* ��̫�� */
#define DBG_COMM                  1        /* ͨ����� */
#define DBG_GENERAL               0        /* ����Ӧ�� */
#define DBG_ALARM                 0        /* ���� */
#define DBG_YEELINK               0        /* yeelink */
#define DBG_DALI                  0        /* DALI���� */
#define DBG_NRF24L01              1        /* NRF24L01���� */

#else

#define DBG_SYSTIME               0        /* ϵͳʱ�� */
#define DBG_PBULIC_PARA           0        /* �������� */
#define DBG_SYSTEM                0        /* ϵͳ���� */
#define DBG_ENERGY_MEASURE        0        /* ����ͳ�� */
#define DBG_ZIGBEE                0        /* ZIGBEE */
#define DBG_WIFI                  0        /* WIFI */
#define DBG_ETHERNET              0        /* ��̫�� */
#define DBG_COMM                  0        /* ͨ����� */
#define DBG_GENERAL               0        /* ����Ӧ�� */
#define DBG_ALARM                 0        /* ���� */
#define DBG_YEELINK               0        /* yeelink */
#define DBG_DALI                  0        /* DALI���� */
#define DBG_NRF24L01              0        /* NRF24L01���� */

#endif

extern BOOLEAN g_debug_sw;                                                     /* ���ڶ�̬�����ر�printf */

extern int OS_DEBUG(const char *format,...);
//#define OS_DEBUG     printf

#endif

