/********************************************************************************
**
** �ļ���:     net_typedef.h
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   common used type definition.
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/07/04 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef NET_TYPEDEF_H
#define NET_TYPEDEF_H

/*
********************************************************************************
* �궨��
********************************************************************************
*/

#ifndef	 _SUCCESS
#define  _SUCCESS                0
#endif

#ifndef	 _FAILURE
#define  _FAILURE                1
#endif

#ifndef  _OVERTIME
#define  _OVERTIME               2
#endif

#define ENDDEVICE                0
#define ROUTER                   1
/*
********************************************************************************
* ��������
********************************************************************************
*/
#define DIRECT_SEND          0    /* ֱ�ӷ��ͣ��ն˽ڵ㲻�ܽ�������ģʽ */
#define PASSIVE_SEND         1    /* �������ͣ��յ��ն����ݺ����Ϸ������ݸ��ն� */

#define SEND_MODE            PASSIVE_SEND


/*
********************************************************************************
* �������Ͷ���
********************************************************************************
*/
#define LINK_HEAD_LEN    3
#define PLOAD_WIDTH      32
#define ADR_WIDTH        1

typedef struct {
    INT8U destaddr[ADR_WIDTH];
    INT8U srcaddr[ADR_WIDTH];
    INT8U type;
    INT8U data[1];
} LINK_HEAD_T;


/*
********************************************************************************
* ��������
********************************************************************************
*/
#define NET_NODE_MAX     20
#define NODE_NAME_MAX    10
#define LIVE_TIME_MAX    5*60            /* �ڵ�����ʱ�䣬��λS */

#define ONLINE           0x00
#define OFFLINE          0x01

/*
********************************************************************************
* �ṹ����
********************************************************************************
*/
union bit_def
{
    INT8U b8;
    struct bit8_def
    {
        INT8U rain:1;
        INT8U motion:1;
        INT8U mq2:1;
        INT8U b3:1;
        INT8U b4:1;
        INT8U b5:1;
        INT8U b6:1;
        INT8U b7:1;
    } b;
};

typedef struct {
    INT16S temperature;
    
    union bit_def status;
} Sensor_t;

/* ͳ����Ϣ */
typedef struct {
    INT16U  log_cnt;                     /* �ն˵�½���� */

    
} Net_link_node_info_t;

/* ����ڵ���Ϣ */
typedef struct {
    INT8U  flowseq;                      /* ����������ˮ�ţ���������ͬһ����������Ĳ�ͬ·����ת���Ķ���� */
    INT8U  type;                         /* �ڵ����ͣ�Ŀǰ��·�������ն˽ڵ����� */
    INT8U  level;                        /* ���缶��-·�ɴ��� */
    INT8U  status;                       /* �ڵ�״̬��Ŀǰ�����ߺ͵������� */
    INT8U  addr[ADR_WIDTH];              /* �ڵ��ַ */
    INT8U  routeaddr[ADR_WIDTH];         /* ���ڵ��·�ɵ�ַ */
    INT8U  name[NODE_NAME_MAX];          /* �ڵ����� */
    INT16U live;                         /* ���ʱ�䵥λ��second���ڴ��ʱ�����޸��£���ɾ���ýڵ㣬�ҵ����������� */
    Net_link_node_info_t node_info;
    Sensor_t sensor;                     /* ��������Ϣ */
} Net_link_node_t;

typedef struct {
    Net_link_node_t *node;
    INT8U len;
    INT8U *pdata;
    INT8U buf[PLOAD_WIDTH];
} PACKET_BUF_T;






#endif 


