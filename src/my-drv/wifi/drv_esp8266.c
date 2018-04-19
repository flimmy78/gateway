/********************************************************************************
**
** �ļ���:     drv_esp8266.c
** ��Ȩ����:   (c) 2013-2016 
** �ļ�����:   esp8266����������͸��ģʽ�����㿪��������ֻ��֧��һ������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2016/12/09 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"

#if EN_WIFI > 0

#if DBG_WIFI > 0
#define SYS_DEBUG           OS_DEBUG
#else
#define SYS_DEBUG(...)      do{}while(0)
#endif

/*
********************************************************************************
* ע�� 
********************************************************************************
*/


/*
********************************************************************************
* ����ģ�����ò���
********************************************************************************
*/
#ifdef OK
#undef OK
#endif

#ifdef ERROR
#undef ERROR
#endif

#define OK 		"OK"
#define ERROR 	"ERROR"

static const char *modetbl[2] = {"TCP", "UDP"};                                /* ����ģʽ */

/*
********************************************************************************
* ����ģ��ṹ
********************************************************************************
*/

typedef enum {	
    /* ����AT���� */
	ATE,           /* �رջ��Թ��� */
	ATRST,         /* ����ģ�� */
	ATRESTORE,     /* �ָ��������ã�ģ������� */
	ATGMR,         /* ��ѯ�汾��Ϣ */
	
    /* WIFI����AT���� */
	ATCWMODE,      /* ����WIFIӦ��ģʽ:1 Station 2 AP 3 AP+Station */
	ATCWDHCP,      /* ���� DHCP���� mode 0:����  AP 1:����  STA 2:����  AP ��  STA en 0:ȥ��  DHCP 1:ʹ��  DHCP */
 
        /* STA��� */
	ATCWLAP,       /* �г���ǰ���õ�AP */
	ATCWJAP,       /* ����AP <ssid ��������� <pwd �����64�ֽ�ASCII���� */
	ATCWJAP_Q,     /* ��ѯ�Ƿ����AP */
	ATCWQAP,       /* �˳���AP������ */   
	ATCWAUTOCON,   /* ���� STA�����Զ����� enable 0:��������STA�Զ����� 1:����ʹ��STA�Զ����� */     
	ATCIPSTAMAC,   /* ����ģ��STAģʽ�� MAC��ַ */     
	ATCIPSTA,      /* ����ģ��STAģʽ�� IP��ַ  */     
    ATSMART_ON,    /* ������������ */
    ATSMART_OFF,   /* ֹͣ�������� */

    /* TCPIP������� */
	ATCIPSTATUS,   /* �������״̬ */
	ATCIPSTART,    /* ����TCP���ӻ���ע��UDP�˿ں� */
	ATCIPSEND,     /* �������� */
	ATCIPCLOSE,    /* �ر� TCP�� UDP */
	ATCIFSR,       /* ��ñ���IP��ַ */
	ATCIPMUX,      /* ���������� */
	ATCIPSERVER,   /* ����Ϊ������ */
	ATCIPMODE,     /* ����ģ�鴫��ģʽ */
	ATCIPSTO,      /* ���÷�������ʱʱ�� */
	ATPING,        /* PING���� */ 
    
} Esp8266_e;

typedef enum {
    AT_STEP_FREE,                    /* ����״̬ */
    AT_STEP_SEND,                    /* ����at���� */
    AT_STEP_RECV,                    /* ����atӦ������ */
} At_step_e;  

typedef enum {
    STEP_POWER_OFF = 0,
    STEP_POWER_ON,
    STEP_QUIT_TRANS,                 /* �˳�͸��ģʽ��esp8266�ϵ��ϵ��Ĭ����atģʽ�����������Ϊ�˱��գ� */
    STEP_CLOSE_TRANS,                /* �ر�͸��ģʽ */
    STEP_SMART_ON,                   /* ����smart config */
    STEP_SMART_WAIT,                 /* �ȴ�smart config */
    STEP_SMART_OFF,                  /* �ر�smart config */
    STEP_SET_STA,                    /* ����ΪSTAģʽ */
    STEP_RST,                        /* ���� */
    STEP_CLOSE_ATE,                  /* �رջ���ʾ */
    STEP_JOIN_AP,                    /* ����AP */
    STEP_QUERY_AP,                   /* ��ѯAP״̬ */
    STEP_TCP_CONNECT,                /* TCP���� */
    STEP_OPEN_TRANS,                 /* ����͸��ģʽ */
    STEP_TRANS,                      /* ��ʼ͸�� */
    STEP_OK,
} Esp82666_step_e;

typedef struct {
    INT8U     at_tmr;
    At_step_e at_step;               /* ��¼AT����ִ��״̬ */
    const At_cmd_t *at_cmd;          /* ��ǰҪִ�е�AT���� */
    char      para[AT_PARA_NUM][AT_PARA_SIZE]; /* AT������� */
    INT16S    wait;                  /* �ȴ�ʱ�䣬��λms */
    BOOLEAN   rflag;                 /* �������ݱ�ʶ�����չ����ݣ�����1 */
    INT16S    rlen;                  /* �������ݳ��� */
    INT8U    *precv;                 /* ָ��ǰ����λ�� */
    INT8U     rbuf[128];             /* �������ݻ��� */
    At_ret    ret;                   /* AT����ִ�н�� */
    void (*fp)(At_ret result);       /* ִ�н����ص� */
} At_t;

typedef struct {
    INT8U   link_tmr;
    INT8U   step;
    INT8U   failcnt;
    INT8U   qjapcnt;                /* ��ѯ�������� */
    BOOLEAN smartcfg;
    
    Wifi_para_t para;
} Priv_t;

/*
********************************************************************************
* ����ģ�����ò���
********************************************************************************
*/
static const At_cmd_t c_at_cmd_sets[]={
    /* ����AT���� */
	{ATE,        "ATE0\r\n",                      OK,      ERROR,    3000,    0,    NULL},           /* �رջ��Թ��� */
	{ATRST,      "AT+RST\r\n",                    OK,      ERROR,    3000,    0,    NULL},           /* ����ģ�� */
	{ATRESTORE,  "AT+RESTORE\r\n",                OK,      ERROR,    3000,    0,    NULL},           /* �ָ��������ã�ģ������� */
	{ATGMR,      "AT+GMR\r\n",                    OK,      ERROR,    3000,    0,    NULL},           /* ��ѯ�汾��Ϣ */
	
    /* WIFI����AT���� */
	{ATCWMODE,   "AT+CWMODE=1\r\n",               OK,      ERROR,    3000,    0,    NULL},            /* ����WIFIӦ��ģʽ:1 Station 2 AP 3 AP+Station */
	{ATCWDHCP,   "AT+CWDHCP=1,1\r\n",             OK,      ERROR,    3000,    0,    NULL},            /* ����DHCP���� mode 0:����AP 1:����STA 2:����AP��STA en 0:ȥ��DHCP 1:ʹ��DHCP */

        /* STA��� */
	{ATCWLAP,    "AT+CWLAP\r\n",                  OK,      ERROR,    3000,    0,    NULL},            /* �г���ǰ���õ�AP */
	{ATCWJAP,    "AT+CWJAP=\"$\",\"$\"\r\n",      OK,      "FAIL",  20000,    2,    NULL},            /* ����AP <ssid ��������� <pwd �����64�ֽ�ASCII���� */
	{ATCWJAP_Q,  "AT+CWJAP?\r\n",                 OK,      ERROR,    3000,    0,    NULL},            /* ��ѯ�Ƿ����AP */
	{ATCWQAP,    "AT+CWQAP\r\n",                  OK,      ERROR,    3000,    0,    NULL},            /* �˳���AP������ */   
	{ATCWAUTOCON,"AT+CWAUTOCONN=$\r\n",           OK,      ERROR,    3000,    1,    NULL},            /* ���� STA�����Զ����� enable 0:��������STA�Զ����� 1:����ʹ��STA�Զ����� */     
	{ATCIPSTAMAC,"AT+CIPSTAMAC=\"$\"\r\n",        OK,      ERROR,    3000,    1,    NULL},            /* ����ģ��STAģʽ�� MAC��ַ */     
	{ATCIPSTA,   "AT+CIPSTA=\"$\"\r\n",           OK,      ERROR,    3000,    1,    NULL},            /* ����ģ��STAģʽ�� IP��ַ  */     
	{ATSMART_ON, "AT+CWSMARTSTART=2\r\n",         OK,      ERROR,    3000,    0,    NULL},            /* ������������ */     
	{ATSMART_OFF,"AT+CWSMARTSTOP\r\n",            OK,      ERROR,    3000,    0,    NULL},            /* ֹͣ�������� */     

    /* TCPIP������� */
	{ATCIPSTATUS,"AT+CIPSTATUS\r\n",              OK,      ERROR,    3000,    0,    NULL},   /* �������״̬ */
	{ATCIPSTART, "AT+CIPSTART=\"$\",\"$\",$\r\n","CONNECT",ERROR,   10000,    3,    NULL},   /* ����TCP���ӻ���ע��UDP�˿ں� */
	{ATCIPSEND,  "AT+CIPSEND\r\n",               ">",      ERROR,    3000,    0,    NULL},   /* �������� */
	{ATCIPCLOSE, "AT+CIPCLOSE=$\r\n",             OK,      ERROR,    3000,    1,    NULL},   /* �ر�TCP��UDP */
	{ATCIFSR,    "AT+CIFSR\r\n",                  OK,      ERROR,    3000,    0,    NULL},   /* ��ñ���IP��ַ */
	{ATCIPMUX,   "AT+CIPMUX=1\r\n",               OK,      ERROR,    3000,    0,    NULL},   /* ���������� */
	{ATCIPSERVER,"AT+CIPSERVER=$,$\r\n",          OK,      ERROR,    3000,    2,    NULL},   /* ����Ϊ������ */
	{ATCIPMODE,  "AT+CIPMODE=$\r\n",              OK,      ERROR,    3000,    1,    NULL},   /* ����ģ�鴫��ģʽ */
	{ATCIPSTO,   "AT+CIPSTO=$\r\n",               OK,      ERROR,    3000,    1,    NULL},   /* ���÷�������ʱʱ�� */
	{ATPING,     "AT+PING=\"$\"\r\n",             OK,      ERROR,    3000,    1,    NULL},   /* PING���� */ 
     
};

static INT8U s_recv_buf[512];
static RoundBuf_t s_roundbuf = {0};
static At_t s_at;
static Priv_t s_priv;

/* �����s_roundbuf��ʼ�����ڿ���usart2�Ľ����жϣ������п��ܴ������� */
void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		roundbuf_write_byte(&s_roundbuf, (USART_ReceiveData(USART2) & 0xFF));
	}
}

static INT32S _conn_write(INT8U* pdata, INT16U datalen)
{
    INT32S ret = datalen;

    while (datalen-- > 0) {
		while (USART_GetFlagStatus(EVAL_COM2, USART_FLAG_TC) == RESET);
		USART_SendData(EVAL_COM2, *pdata++);
    }

    return ret;
}

static INT32S _conn_read(INT8U* pdata, INT16U datalen)
{
	INT32S ret = 0;
    INT16S recv;

    while (datalen-- > 0) {
        if ((recv = roundbuf_read_byte(&s_roundbuf)) == -1) {
            break;
        }
        pdata[ret++] = recv;
    }
    return ret;      
}

static void _para_replace(INT8U *src, INT16U *srclen, INT8U *cmd, INT8U paranum, void *p)
{
	INT16U i, j, n, len, paralen;
    char (*para)[AT_PARA_SIZE] = (char (*)[AT_PARA_SIZE])p;  

	len = strlen((char*)cmd);
	for (i = 0, j = 0, n = 0; (i < len) && (j+1 < *srclen); i++) {
		if (cmd[i] == '$') {
            if (paranum-- == 0) {
                break;
            }

            paralen = strlen((char*)para[n]);
            if (j + paralen >= *srclen) {
                break;
            }
			memcpy(&src[j], para[n++], paralen);
			j += paralen;
		} else {
            src[j++] = cmd[i];
        }
	}

    src[j] = '\0';
    *srclen = j;
}
 
/*******************************************************************
** ������:     _at_handle_tmr
** ��������:   at handle��ʱ��
** ����:       [in] index  : ��ʱ������
** ����:       ��
********************************************************************/
static void _at_handle_tmr(void *index)
{	

    os_timer_start(s_at.at_tmr, 1*MILTICK);                                    /* 100ms */
    switch (s_at.at_step) {
        case AT_STEP_SEND:
        {
            INT8U at_cmd_buf[256];
            INT16U cmd_buf_len = sizeof(at_cmd_buf);
            
            if (s_at.at_cmd->paranum > 0) {
                _para_replace(at_cmd_buf, &cmd_buf_len, s_at.at_cmd->cmd, 
                    s_at.at_cmd->paranum, s_at.para);
            } else {
                if (strlen((char*)s_at.at_cmd->cmd) < cmd_buf_len) {
                    cmd_buf_len = strlen((char*)s_at.at_cmd->cmd);
                } else {
                    cmd_buf_len--;                                             /* �ַ�����β������\0 */
                }
                
                memcpy(at_cmd_buf, s_at.at_cmd->cmd, cmd_buf_len);
                at_cmd_buf[cmd_buf_len] = '\0';
            }

            SYS_DEBUG("<write at:%s>\n", at_cmd_buf);
            roundbuf_reset(&s_roundbuf);
            _conn_write(at_cmd_buf, cmd_buf_len);
            s_at.ret   = AT_DOING;
            s_at.rflag = 0;
            s_at.rlen  = 0;
            s_at.precv = s_at.rbuf;
            s_at.at_step = AT_STEP_RECV;
            break;
        }
        case AT_STEP_RECV:
        {
            s_at.wait += 100;                                                  /* �ȴ����100ms */
            if (s_at.wait < s_at.at_cmd->timeout) {
                INT16S ret;
                
                if (s_at.rlen >= sizeof(s_at.rbuf)) {
                    goto PARSE;
                }

                ret = _conn_read(s_at.precv, sizeof(s_at.rbuf) - s_at.rlen);
                if (ret > 0) {
                    s_at.rflag = 1;
                    s_at.rlen += ret;
                    s_at.precv += ret;
                } else {
                    if (s_at.rflag) {
                        goto PARSE;
                    }
                }
                break;
            } else {
                if (s_at.rflag) {
                    goto PARSE;
                }

                s_at.ret = AT_TIMEOUT;
                goto END;
            }
PARSE:
            if (s_at.rlen >= sizeof(s_at.rbuf)) {
                s_at.rbuf[sizeof(s_at.rbuf)-1] = '\0';
            } else {
                s_at.rbuf[s_at.rlen] = '\0';
            }
    
            SYS_DEBUG("<read at:[%d]%s>\n", s_at.rlen, s_at.rbuf);

            if (mem_find_str(s_at.rbuf, s_at.rlen, (char*)s_at.at_cmd->scuesscode)) {
                s_at.ret = AT_OK;
            } else {
                s_at.ret = AT_FAIL;
            }
            goto END;
        }
        default:
            s_at.ret = AT_INVALID_PARAMS;
            goto END;
    }
    return;

END:
    if (s_at.fp != NULL) {
        s_at.fp(s_at.ret);
    }
    s_at.at_step = AT_STEP_FREE;
    os_timer_stop(s_at.at_tmr);
}

static void _at_write_and_read(const At_cmd_t *at_cmd, void *para, void (*callback)(At_ret result))
{
    if (at_cmd->paranum > 0) {
        return_if_fail(at_cmd->paranum <= AT_PARA_NUM);
        if (para != NULL) {
            memcpy(s_at.para, para, at_cmd->paranum*AT_PARA_SIZE);
        } else {
            return_if_fail(at_cmd->para != NULL);
            memcpy(s_at.para, at_cmd->para, at_cmd->paranum*AT_PARA_SIZE);
        }
    } else {
        memset(s_at.para, 0, AT_PARA_NUM*AT_PARA_SIZE);
    }

    s_at.at_cmd  = at_cmd;
    s_at.at_step = AT_STEP_SEND;
    s_at.wait    = 0;
    s_at.rflag   = 0;
    s_at.rlen    = 0;
    s_at.precv   = s_at.rbuf;
    s_at.ret     = AT_DOING;
    s_at.fp      = callback;
    
    os_timer_start(s_at.at_tmr, 1*MILTICK);
}

/*******************************************************************
** ������:     _link_callback
** ��������:   atִ�лص�����
** ����:       [in] result  : ִ�н��
** ����:       ��
********************************************************************/
static void _link_callback(At_ret result)
{
    if (result == AT_OK) {
        s_priv.failcnt = 0;
        os_timer_start(s_priv.link_tmr, 1);
    
        switch (s_priv.step) {
            case STEP_CLOSE_TRANS:
            {
                if (s_priv.smartcfg == TRUE) {
                    s_priv.step = STEP_SMART_ON;
                } else {
                    s_priv.step = STEP_SET_STA;
                }    
                break;
            }
            case STEP_SMART_ON:
            {
                s_priv.step = STEP_SMART_WAIT;
                break;
            }
            case STEP_SMART_OFF:
            {
                s_priv.step = STEP_SET_STA;
                break;
            }            
            case STEP_SET_STA:
            {
                s_priv.step = STEP_RST;
                break;
            }
            case STEP_RST:
            {
                s_priv.step = STEP_CLOSE_ATE;
                os_timer_start(s_priv.link_tmr, 2*SECOND);
                break;
            }   
            case STEP_CLOSE_ATE:
            {
                s_priv.step = STEP_JOIN_AP;
                break;
            }
            case STEP_JOIN_AP:
            {
                s_priv.step = STEP_QUERY_AP;
                break;
            }
            case STEP_QUERY_AP:
            {
                if (mem_find_str(s_at.rbuf, s_at.rlen, "No AP")) {
                    ;
                } else if (mem_find_str(s_at.rbuf, s_at.rlen, "+CWJAP:")) { 
                    s_priv.step = STEP_TCP_CONNECT;
                    break;
                } else { 
                    ;
                    /* ���һֱ���Ӳ���·����������Ҫ�������� */
                }
                os_timer_start(s_priv.link_tmr, 10*SECOND);
                s_priv.qjapcnt++;
                if (s_priv.qjapcnt == 12) {
                    s_priv.step = STEP_JOIN_AP;
                } else if (s_priv.qjapcnt == 24) {
                    s_priv.step = STEP_POWER_OFF;
                }
                break;
            }
            case STEP_TCP_CONNECT:
            {
                s_priv.step = STEP_OPEN_TRANS;
                break;
            }
            case STEP_OPEN_TRANS:
            {
                s_priv.step = STEP_TRANS;
                break;
            }
            case STEP_TRANS:
            {
                s_priv.step = STEP_OK;
                break;
            }
            default:
                break;
        }

    } else {
        SYS_DEBUG("<cb fail step:%d, result:%d>\n", s_priv.step, result);

        switch (s_priv.step) {
            case STEP_JOIN_AP:
            {
                s_priv.step = STEP_QUERY_AP;
                os_timer_start(s_priv.link_tmr, 1);
                break;
            }
            case STEP_TCP_CONNECT:
            {
                s_priv.failcnt++;
                os_timer_start(s_priv.link_tmr, s_priv.failcnt*5*SECOND);
                if (s_priv.failcnt > 20) {
                    s_priv.failcnt = 0;
                    s_priv.step = STEP_POWER_OFF;
                }
                
                break;
            }
            default:
            {
                s_priv.failcnt++;

                os_timer_start(s_priv.link_tmr, s_priv.failcnt*5*SECOND);
                if (s_priv.failcnt > 10) {
                    s_priv.failcnt = 0;
                    s_priv.step = STEP_POWER_OFF;
                }
                break;
            }
        }
    }

}

/*******************************************************************
** ������:     _link_tmr
** ��������:   link��ʱ��
** ����:       [in] index  : ��ʱ������
** ����:       ��
********************************************************************/
static void _link_tmr(void *index)
{
    char para[4][AT_PARA_SIZE];
    index = index;

    switch (s_priv.step) {
        case STEP_POWER_OFF:                                                   /* �ϵ�ģ��1s */
        {
            SYS_DEBUG("<_link_tmr: POWER OFF>\n");
            /* TO DO ִ��power off */

            s_priv.qjapcnt = 0;
            s_priv.step = STEP_POWER_ON;
            os_timer_start(s_priv.link_tmr, 1*SECOND);
            break;
        }
        case STEP_POWER_ON:                                                    /* �ϵ���ʱ1s */
        {
            SYS_DEBUG("<_link_tmr: POWER ON>\n");
            
            /* TO DO ִ��power on */

            s_priv.step = STEP_QUIT_TRANS;
            os_timer_start(s_priv.link_tmr, 1*SECOND);
            break;
        }
        case STEP_QUIT_TRANS:                                                  /* �˳�͸��ģʽ */
        {
            SYS_DEBUG("<_link_tmr: QUIT_TRANS>\n");
            
            _conn_write((INT8U*)"+++", 3);

            s_priv.step = STEP_CLOSE_TRANS;
            os_timer_start(s_priv.link_tmr, 5*MILTICK);
            break;
        }
        case STEP_CLOSE_TRANS:                                                 /* �ر�͸��ģʽ */
        {
            SYS_DEBUG("<_link_tmr: CLOSE_TRANS>\n");

            sprintf(para[0], "%d", 0);
            _at_write_and_read(&c_at_cmd_sets[ATCIPMODE], para, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }    
        case STEP_SMART_ON:                                                    /* ����smartcfg */
        {
            SYS_DEBUG("<_link_tmr: SMART_ON>\n");

            _at_write_and_read(&c_at_cmd_sets[ATSMART_ON], NULL, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }
        case STEP_SMART_WAIT:                                                  /* �ȴ�smartcfg */
        {
            INT8U i, *ptr;
            INT32S ret;
            static INT8U wait =0;
            
            SYS_DEBUG("<_link_tmr: SMART_WAIT>\n");
            os_timer_start(s_priv.link_tmr, 3*SECOND);

            ret = _conn_read(s_at.rbuf, sizeof(s_at.rbuf));
            if (ret > 0) {
                if ((ptr = mem_find_str(s_at.rbuf, ret, "SSID:")) == NULL) {
                    goto ERR;
                }

                ptr += strlen("SSID:");
                for (i = 0; i < sizeof(s_priv.para.ssid) - 1; i++) {
                    if (*ptr == '\r' || *ptr == '\n') {
                        break;
                    }
                    s_priv.para.ssid[i] = *ptr++;
                }

                s_priv.para.ssid[i] = '\0';

                if ((ptr = mem_find_str(s_at.rbuf, ret, "PASSWORD:")) == NULL) {
                    goto ERR;
                }

                ptr += strlen("PASSWORD:");
                for (i = 0; i < sizeof(s_priv.para.pwd) - 1; i++) {
                    if (*ptr == '\r' || *ptr == '\n') {
                        break;
                    }
                    s_priv.para.pwd[i] = *ptr++;
                }
                
                s_priv.para.pwd[i] = '\0';

                public_para_manager_store_by_id(WIFI_PARA_, (INT8U*)&s_priv.para, sizeof(Wifi_para_t));        
ERR:
                wait = 0;
                s_priv.smartcfg = 0;
                public_para_manager_read_by_id(WIFI_PARA_, (INT8U*)&s_priv.para, sizeof(Wifi_para_t));        
                s_priv.step = STEP_SMART_OFF;
                os_timer_start(s_priv.link_tmr, 5*SECOND);
            } else {
                if (wait++ > 20) {
                    wait = 0;
                    s_priv.smartcfg = 0;
                    s_priv.step = STEP_SMART_OFF;
                    os_timer_start(s_priv.link_tmr, 5*SECOND);
                }
            }
            break;
        }        
        case STEP_SMART_OFF:                                                   /* �ر�smartcfg */
        {
            SYS_DEBUG("<_link_tmr: SMART_OFF>\n");

            _at_write_and_read(&c_at_cmd_sets[ATSMART_OFF], NULL, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }    
        case STEP_SET_STA:                                                     /* ����ΪSTAģʽ */
        {
            SYS_DEBUG("<_link_tmr: SET_STA>\n");
            sprintf(para[0], "%d", 1);
            _at_write_and_read(&c_at_cmd_sets[ATCWMODE], para, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }   
        case STEP_RST:                                                         /* ���� */
        {
            SYS_DEBUG("<_link_tmr: RST>\n");
            _at_write_and_read(&c_at_cmd_sets[ATRST], NULL, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }
        case STEP_CLOSE_ATE:                                                   /* �رջ��� */
        {
            SYS_DEBUG("<_link_tmr: CLOSE_ATE>\n");
            _at_write_and_read(&c_at_cmd_sets[ATE], NULL, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }  
        case STEP_JOIN_AP:                                                     /* ����AP */
        {
            SYS_DEBUG("<_link_tmr: JOIN_AP>\n");
            snprintf(para[0], AT_PARA_SIZE, "%s", s_priv.para.ssid);
            snprintf(para[1], AT_PARA_SIZE, "%s", s_priv.para.pwd);
            
            _at_write_and_read(&c_at_cmd_sets[ATCWJAP], para, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }
        case STEP_QUERY_AP:                                                    /* ��ѯAP״̬ */
        {
            SYS_DEBUG("<_link_tmr: QUERY_AP>\n");

            _at_write_and_read(&c_at_cmd_sets[ATCWJAP_Q], NULL, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }
        case STEP_TCP_CONNECT:                                                 /* TCP ���� */
        {
            SYS_DEBUG("<_link_tmr: TCP_CONNECT>\n");
            strncpy(para[0], modetbl[0], AT_PARA_SIZE); 
            strncpy(para[1], s_priv.para.server, AT_PARA_SIZE);
            sprintf(para[2], "%d", s_priv.para.port);
            _at_write_and_read(&c_at_cmd_sets[ATCIPSTART], para, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }
        case STEP_OPEN_TRANS:                                                  /* ��͸��ģʽ */
        {
            SYS_DEBUG("<_link_tmr: OPEN_TRANS>\n");

            sprintf(para[0], "%d", 1);
            _at_write_and_read(&c_at_cmd_sets[ATCIPMODE], para, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }
        case STEP_TRANS:                                                       /* ��ʼ͸�� */
        {
            SYS_DEBUG("<_link_tmr: TRANS>\n");

            _at_write_and_read(&c_at_cmd_sets[ATCIPSEND], NULL, _link_callback);

            os_timer_stop(s_priv.link_tmr);
            break;
        }        
        case STEP_OK:                                                          /* �ɹ� */
        {
            SYS_DEBUG("<_link_tmr: OK>\n");


            os_timer_stop(s_priv.link_tmr);
            break;
        }      
        default:
            break;
    }
}

/*******************************************************************
** ������:      drv_esp8266_init
** ��������:    
** ����:        none
** ����:        none
********************************************************************/
void drv_esp8266_init(void)
{
    memset(&s_priv, 0, sizeof(Priv_t));

    s_at.at_tmr = os_timer_create(0, _at_handle_tmr);
    
    s_priv.link_tmr  = os_timer_create(0, _link_tmr);
    os_timer_start(s_priv.link_tmr, 1*SECOND);

    roundbuf_init(&s_roundbuf, s_recv_buf, sizeof(s_recv_buf), NULL); 
    /* bug:(���ܴ���bug)�����ѭ����������ʼ�����ڿ���usart2�ж� */
    usart_rxit_enable(COM2);
    
    public_para_manager_read_by_id(WIFI_PARA_, (INT8U*)&s_priv.para, sizeof(Wifi_para_t));        
}

INT32S drv_esp8266_write(INT8U* pdata, INT16U datalen)
{
    if (s_priv.step != STEP_OK) {
        return -1;
    }
    
    return _conn_write(pdata, datalen);
}

INT32S drv_esp8266_read(INT8U* pdata, INT16U datalen)
{
    if (s_priv.step != STEP_OK) {
        return -1;
    }

    return _conn_read(pdata, datalen);
}

BOOLEAN drv_esp8266_is_connect(void)
{
    if (s_priv.step == STEP_OK) {
        return true;
    } else {
        return false;
    }
}

BOOLEAN drv_esp8266_reset(INT8U delay)
{
    s_priv.step = STEP_POWER_OFF;
    s_priv.failcnt = 0;

    os_timer_stop(s_at.at_tmr);

    os_timer_start(s_priv.link_tmr, 1*delay);
    return true;
}

BOOLEAN drv_esp8266_smartconfig(void)
{
    s_priv.smartcfg = TRUE;

    drv_esp8266_reset(1);

    return true;
}

BOOLEAN drv_esp8266_server(char *server)
{
    snprintf(s_priv.para.server, sizeof(s_priv.para.server), "%s", server);
    public_para_manager_store_by_id(WIFI_PARA_, (INT8U*)&s_priv.para, sizeof(Wifi_para_t));        

    drv_esp8266_reset(30);
    return true;
}

BOOLEAN drv_esp8266_port(INT16U port)
{
    s_priv.para.port = port;
    public_para_manager_store_by_id(WIFI_PARA_, (INT8U*)&s_priv.para, sizeof(Wifi_para_t));        

    drv_esp8266_reset(30);
    return true;
}

#endif /* end of EN_WIFI */

