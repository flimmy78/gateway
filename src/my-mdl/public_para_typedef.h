/********************************************************************************
**
** �ļ���:     public_para_typedef.h
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ʵ�ֹ��������ṹ�嶨��Ͳ�������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/28 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef PUBLIC_PARA_TYPEDEF_H
#define PUBLIC_PARA_TYPEDEF_H   1

typedef enum {
    LAMP1_INDEX = 0,
    LAMP_NUM_MAX
} Lamp_e;

/*
********************************************************************************
* ϵͳ��Ϣ�ṹ����
********************************************************************************
*/
/*
********************************************************************************
* ϵͳ��Ϣ�ṹ����
********************************************************************************
*/
#define GROUP_NUM   3

typedef struct {
    INT32U uid;               /* �ն�uid */
    INT8U group[GROUP_NUM];   /* �ն˷��� */
} Sys_base_info_t;            /* ����һЩ�����޸ģ����Һ���Ҫ��ϵͳ��Ϣ */

typedef struct {
    INT32U restartcnt;        /* �ն��������� */
    INT32U wdgcnt;            /* ���Ź���λ���� */
    
} Sys_run_info_t;



/*
********************************************************************************
* �ƿ����¼��ṹ����
********************************************************************************
*/
typedef enum {
    LAMP_EVENT_ALARM,
    LAMP_EVENT_CENTER,
    LAMP_EVENT_AUTO,
    LAMP_EVENT_MAX
} Lamp_envnt_e;

typedef enum {
    LAMP_CLOSE,
    LAMP_OPEN
} Lmap_ctl_e;


typedef struct {
    INT8U event;
    INT8U ctl;
    INT8U dimming;
} Lamp_event_t;

/*
********************************************************************************
* �洢���ܲ���
********************************************************************************
*/
typedef struct {
	float  lamp_ws[LAMP_NUM_MAX];           /* ���� */
    INT32U t_light[LAMP_NUM_MAX];           /* ����ʱ�� */
} Power_total_t;

/*
********************************************************************************
* �������
********************************************************************************
*/
typedef struct { 
    INT8U   mac[6];                         /* Source Mac Address */  
    INT8U   ip[4];                          /* Source IP Address */ 
    INT8U   sn[4];                          /* Subnet Mask */ 
    INT8U   gw[4];                          /* Gateway IP Address */
    INT8U   dns[4];                         /* DNS server IP Address */ 
    INT8U   dhcp;                           /* 1 - Static, 2 - DHCP */ 
    INT8U   server_ip[4];                   /* Dest IP Address */ 
    INT16U  server_port;                    /* Dest port */ 
} Ip_para_t;

typedef struct {
    char    ssid[30];                       /* ��������� */
    char    pwd[30];                        /* ���� */
    char    server[30];                     /* ip or domain name */
    INT16U  port;                           /* Dest port */     
} Wifi_para_t;

/* ���nRF24L01 */
typedef struct {
    INT8U rf_ch;                            /* 6:0��0000010������оƬ����ʱ���ŵ����ֱ��Ӧ1~125 ����,�ŵ����Ϊ1MHZ, Ĭ��Ϊ02��2402MHz   */
    INT8U rf_dr;                            /* ���ݴ������ʣ�������Ƶ������Ϊ250kbps ��1Mbps ��2Mbps */
    INT8U rf_pwr;                           /* ����TX���书��111: 7dBm,  000:-12dBm */
} Rf_para_t;

/*
********************************************************************************
* ����������
********************************************************************************
*/
typedef enum {
    TASK_TIME,                              /* ʱ������ */
    TASK_LONG_LAT,                          /* ��γ������ */
    TASK_LIGHT,                             /* �������� */
    TASK_TYPE_MAX
} Task_type_e;

typedef union {
    INT8U type;
struct { 
        INT8U weekday:1;                    /* ���� */
		INT8U monday:1;                     /* ��һ */
		INT8U thes:1;                       /* �ܶ� */
		INT8U wed:1;                        /* ���� */
		INT8U thur:1;                       /* ���� */
		INT8U fri:1;                        /* ���� */
		INT8U sat:1;                        /* ���� */
		INT8U dt:1;                         /* ģʽ 0:RTCģʽ, 1:���� */
	} ch;
} Task_time_type_t;

typedef struct {
    Time_t time;                            /* ��ʱ */
    INT8U  ctl;                             /* ִ�ж��� */
    INT8U  dimming;                         /* ������ */
} Time_action_t;

#define ACTION_NUM_MAX	7                   /* һ������"��ʱ/����ʱ"���Ķ������� */
#define TASK_NUM_MAX    7                   /* ֧��"��ʱ/����ʱ"��������� */
typedef struct {
    INT8U num;                              /* �������� */
    Task_time_type_t type;                  /* ʱ������ */
    Time_action_t action[ACTION_NUM_MAX];
} Task_time_t;

typedef struct {
    double lon;                             /* ���� */
    double lat;                             /* γ�� */
} Long_lat_t;

typedef  struct 
{
    INT8U mode;                                    /* ����ǰ����ģʽ */
    Long_lat_t long_lat;                           /* ��γ�� */
    INT8U time_num[LAMP_NUM_MAX];                  /* ��ʱ/����ʱ ���� */
	Task_time_t time[LAMP_NUM_MAX][TASK_NUM_MAX];  /* ��ʱ/����ʱ ���� */
} Task_t;

    
/*
********************************************************************************
*                        Define Default PubParas
********************************************************************************
*/
#ifdef GLOBALS_PP_REG
const INT32U c_uid = 0xffffffff;

#if EN_ETHERNET > 0
const Ip_para_t c_ip_para = {
    {0xc8, 0x9c, 0xdc, 0x33, 0x91, 0xf9},
    {192, 168,  31, 120},
    {255, 255, 255,   0},
    {192, 168,  31,   1},
    {  0,   0,   0,   0},
       2,                                     /* 1 - Static, 2 - DHCP */ 
    {192, 168,  31, 130},
    8600
};
#endif

#if EN_WIFI > 0
const Wifi_para_t c_wifi_para = {
        "syj0925",
        "23258380",
        "api.yeelink.net",
        80
};
#endif

#if EN_NRF24L01 > 0
const Rf_para_t c_rf_para = {
        100,                      /* �ֱ��Ӧ1~125 ����,�ŵ����Ϊ1MHZ, Ĭ��Ϊ02��2402MHz */
        0x20,                     /* 250kbps:0x20, 1Mbps:0x00, 2Mbps:0x08; */
        0x07                      /* 0:-12dbm, 1:-6dbm, 2:-4dbm, 3:0dbm, 4:1dbm, 5:3dbm, 6:4dbm, 7:7dbm */
};
#endif

const Task_t c_task = {
    TASK_LONG_LAT,//TASK_TIME,
    {118.068377, 24.569995},
    {3},
    {
        {
            {2, {0x7f}, {{{0, 2, 0}, LAMP_OPEN, 50}, {{0, 5, 0}, LAMP_CLOSE, 0}}},
            {2, {0x7f}, {{{1, 0, 0}, LAMP_OPEN, 50}, {{1, 5, 0}, LAMP_CLOSE, 0}}},
            {2, {0x7f}, {{{2, 0, 0}, LAMP_OPEN, 50}, {{2, 5, 0}, LAMP_CLOSE, 0}}}
        }
    }
};


#endif
#endif

