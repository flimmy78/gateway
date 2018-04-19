/********************************************************************************
**
** �ļ���:     wsn_sensor.c
** ��Ȩ����:   (c) 2013-2017
** �ļ�����:   ���ߴ���������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2017/8/4 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"

#if DBG_GENERAL > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif
/*
********************************************************************************
* ��������
********************************************************************************
*/




/*
********************************************************************************
* �ṹ����
********************************************************************************
*/


/*
********************************************************************************
* ��̬����
********************************************************************************
*/
#define DATA_BUF_SIZE    512

static INT8U s_buf[DATA_BUF_SIZE];

#define API_KEY          "49a79746d6be66694e6c6963dea1d4cc"
#define REMOTE_SERVER    "api.yeelink.net"


static INT16U _asm_http_post(char *pbuf, INT16U buflen, const char *device_id, const char *sensors_id, const char *value)
{
	char str_tmp[128] = {0};
    char http_content[32] = {0};                                               /* Http���ݣ������� */
    
    sprintf(str_tmp, "/v1.0/device/%s/sensor/%s/datapoints", device_id, sensors_id);
    // ȷ��HTTP���ύ���� {"value":20}
    sprintf(http_content, "{\"value\":%s}", value);
    // ȷ�� HTTP�����ײ�
    // ����POST /v1.0/device/98d19569e0474e9abf6f075b8b5876b9/1/1/datapoints/add HTTP/1.1\r\n
    sprintf(pbuf, "POST %s HTTP/1.1\r\n", str_tmp);
    // �������� ���� Host: api.machtalk.net\r\n
    sprintf(str_tmp, "Host:%s\r\n", REMOTE_SERVER);
    strcat(pbuf, str_tmp);

    // ��������
    sprintf(str_tmp, "U-ApiKey:%s\r\n", API_KEY);
    strcat(pbuf, str_tmp);

    strcat(pbuf, "Accept: */*\r\n");
    // �����ύ�����ݵĳ��� ���� Content-Length:12\r\n
    sprintf(str_tmp, "Content-Length:%d\r\n", strlen(http_content));
    strcat(pbuf, str_tmp);
    // ���ӱ������ʽ Content-Type:application/x-www-form-urlencoded\r\n
    strcat(pbuf, "Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(pbuf, "Connection: close\r\n");
    // HTTP�ײ���HTTP���� �ָ�����
    strcat(pbuf, "\r\n");
    // HTTP��������
    strcat(pbuf, http_content);

    return strlen(pbuf);
}

static void _sensor_report(PACKET_BUF_T *packet)
{
    float temperature;
	char value[16]={0};
    INT16U len;
    Net_link_node_t *node;

    node = packet->node;

#if 0
    if (node->addr[0] == 0x26) {
        temperature = (float)node->sensor.temperature/10;
        printf("<----addr:%d--_temp:%f>\n", node->addr[0], temperature);

        sprintf(value, "%f", temperature);        
        len = _asm_http_post((char *)s_buf, DATA_BUF_SIZE, "148459", "411009", value);
    } else if (node->addr[0] == 0x2d) {
        temperature = (float)node->sensor.temperature/10;
        sprintf(value, "%f", temperature);  
    
        len = _asm_http_post((char *)s_buf, DATA_BUF_SIZE, "148459", "411010", value);
    }  else if (node->addr[0] == 0x7e) {
        sprintf(value, "%d", node->sensor.status.b.rain);  
        len = _asm_http_post((char *)s_buf, DATA_BUF_SIZE, "148459", "411040", value);
    }  else if (node->addr[0] == 0xbd) {
        sprintf(value, "%d", node->sensor.status.b.motion);  
        len = _asm_http_post((char *)s_buf, DATA_BUF_SIZE, "148459", "411041", value);
    }
#endif /* if 0. 2017-8-10 15:22:08 syj */
    if (node->addr[0] == 0x26) {
        temperature = (float)node->sensor.temperature/10;
        len = sprintf((char *)s_buf, "��ַ26�¶�:%f��", temperature);        
    } else if (node->addr[0] == 0x2d) {
#if 0
        temperature = (float)node->sensor.temperature/10;
        len = sprintf((char *)s_buf, "��ַ2d�¶�:%f��", temperature); 
#endif /* if 0. 2017-8-24 17:26:20 syj */
        if (node->sensor.status.b.mq2) {
            len = sprintf((char *)s_buf, "��ַ2d������");
            tts_play(7, 1);
        } else {
            len = sprintf((char *)s_buf, "��ַ2d���������");  
            tts_play_stop(3);
            tts_play(8, 0);
        }
    }  else if (node->addr[0] == 0x7e) {
        if (node->sensor.status.b.rain) {
            len = sprintf((char *)s_buf, "��ַ7e��⵽����");
            tts_play(3, 1);
        } else {
            len = sprintf((char *)s_buf, "��ַ7e��⵽����");  
            tts_play_stop(3);
            tts_play(4, 0);
        }
    }  else if (node->addr[0] == 0xbd) {
        if (node->sensor.status.b.motion) {
            len = sprintf((char *)s_buf, "��ַbd��⵽����");   
            tts_play(1, 0);
        } else {
            len = sprintf((char *)s_buf, "��ַbd��⵽û��");  
            tts_play(2, 0);
        }
    }

#if EN_WIFI > 0
    drv_esp8266_write(s_buf, len);
#endif
}

/*******************************************************************
** ������:      _ack_0x03
** ��������:    0X03����Ӧ��
** ����:        [in] node  : �ڵ�
**              [in] result: Ӧ����
** ����:        ��
********************************************************************/
static void _ack_0x03(PACKET_BUF_T *packet, INT8U result)
{
    Stream_t wstrm;

    stream_init(&wstrm, packet->pdata, PLOAD_WIDTH-LINK_HEAD_LEN);
    stream_write_byte(&wstrm, result);
    net_link_send_dirsend(0x83, packet);
}

/*******************************************************************
** ������:     _hdl_0x03
** ��������:   �����������ϱ� 
** ����:       [in]cmd    : �������
**             [in]packet : ����ָ��
** ����:       ��
********************************************************************/
static void _hdl_0x03(INT8U cmd, PACKET_BUF_T *packet)
{
    BOOLEAN ack;
    INT8U type;
    Stream_t rstrm;
    Net_link_node_t *node;

    node = packet->node;
    stream_init(&rstrm, packet->pdata, packet->len);

    ack = stream_read_byte(&rstrm);

    if (ack) {                                                                 /* ��ҪӦ�� */
        _ack_0x03(packet, _SUCCESS);
    } else {
        #if SEND_MODE == PASSIVE_SEND
        net_link_send_inform(packet->node);                                    /* ����ֱ�����ڵ���ն˽ڵ㣬�����ն˽ڵ�ֻ�з��������ݺ��һ��ʱ���ڴ��ڽ���״̬(���͹���)������·�������ֻ�����յ��������ݺ��� */
        #endif
    }

    while (stream_get_left_len(&rstrm) > 0) {
        type = stream_read_byte(&rstrm);
        
        switch(type) {
            case 0x01:
                node->sensor.status.b8 = stream_read_byte(&rstrm);             /* ����״̬ */
                break;
            case 0x02:
                node->sensor.temperature = stream_read_half_word(&rstrm);      /* �¶� */
                break;
            default:
                goto EXIT;
        }
    }

EXIT:
    _sensor_report(packet);
}  

/*******************************************************************
** ������:     _hdl_0x05
** ��������:   S1�����ϱ�
** ����:       [in]cmd    : �������
**             [in]packet : ����ָ��
** ����:       ��
********************************************************************/
static void _hdl_0x05(INT8U cmd, PACKET_BUF_T *packet)
{
    #if SEND_MODE == PASSIVE_SEND
    net_link_send_inform(packet->node);                                        /* ����ֱ�����ڵ���ն˽ڵ㣬�����ն˽ڵ�ֻ�з��������ݺ��һ��ʱ���ڴ��ڽ���״̬(���͹���)������·�������ֻ�����յ��������ݺ��� */
    #endif
    
    tts_play_stop(3);
    tts_play(5, 0);
}

/*******************************************************************
** ������:     _hdl_0x06
** ��������:   S2�����ϱ�
** ����:       [in]cmd    : �������
**             [in]packet : ����ָ��
** ����:       ��
********************************************************************/
static void _hdl_0x06(INT8U cmd, PACKET_BUF_T *packet)
{
    #if SEND_MODE == PASSIVE_SEND
    net_link_send_inform(packet->node);                                        /* ����ֱ�����ڵ���ն˽ڵ㣬�����ն˽ڵ�ֻ�з��������ݺ��һ��ʱ���ڴ��ڽ���״̬(���͹���)������·�������ֻ�����յ��������ݺ��� */
    #endif
    
    tts_play(6, 0);
}

/*
********************************************************************************
* ע��ص�����
********************************************************************************
*/
static FUNCENTRY_LINK_T s_functionentry[] = {
        0x03,                   _hdl_0x03,
        0x05,                   _hdl_0x05,
        0x06,                   _hdl_0x06
};
static INT8U s_funnum = sizeof(s_functionentry) / sizeof(s_functionentry[0]);

/*******************************************************************
** ������:      wsn_sensor_init
** ��������:    �������ɼ���ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void wsn_sensor_init(void)
{
    INT8U i;

    for (i = 0; i < s_funnum; i++) {
        net_link_register(s_functionentry[i].index, s_functionentry[i].entryproc);
    }
}

