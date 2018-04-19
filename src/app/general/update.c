/********************************************************************************
**
** �ļ���:     update.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ��������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/10/09 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"
#if EN_UPDATE > 0 

#if DBG_GENERAL > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif
/*
********************************************************************************
* ע: ����������δ��֤�������ƹ��ڼ򵥣������¼�������:
* 1.�ն�������û�б���֮ǰ����״̬��ֻ����������
* 2.flash�ǰ���ҳ����������д�����������ݵ�������:һ����ȡҳ���ݣ���������ҳ����
* �����޸Ļ����е�ҳ���ݣ��ġ����޸ĵĻ��水ҳд�룻��������д��ҳ�������⣬��
* �п��ܵ��������Ѿ�д�õ��������⵽�ƻ�������MAP��ʶλ����Ϊ�ð�����ʱ��Ч��
* ����������µĺ���ǣ��������а�����crcУ�������������crc��ȷ�ˣ�������
* ���³������в��ɿصĴ���
********************************************************************************
*/


/*
********************************************************************************
* �궨��
********************************************************************************
*/
#define APP_START_ADDR          (0x08002000)   /* app0��ʼ���λ��, 0x2000�Ǹ�bootʹ��8k */
#define APP_END_ADDR	        (0x08020000)   /* flash�ı�Ե */
#define APP_MAX_SIZE	        (0xf000)       /* ����APP�Ĵ�С,60k */
#define APP_PAGE_SIZE	        (0x100)        /* flash��ҳ��С,256bytes */

#define UPDATE_PKG_MAX_SIZE     (160)          /* ��������������ܳ���160bytes */
#define UPDATE_PKG_MAP_MAX_SIZE (128)          /* ������հ�ӳ���Ĵ�С,���ɽ��հ���:128*8 */

#define EEPROM_UPDATE_FLAG_ADDR (0x080807F0)   /* ��boot����˲�������˵�ַ�����bootһ�� */

/*
********************************************************************************
* �ṹ����
********************************************************************************
*/
typedef struct {
    INT16U version;          /* Ҫ��������İ汾�� */
    INT32U filesize;         /* �����ļ���С */
    INT16U filecrc;          /* �ļ�У�� */
    INT8U  pkgsize;          /* ÿ����С */
} Update_info_t;

typedef struct {
    INT16U total_pkg;        /* ����������ܰ��� */
    INT16U total_page;       /* ��������ռ����ҳ������Ϊflash��ҳ����д */
    Update_info_t info;      /* ��λ���·���������Ϣ */
                             /* ÿ��λ������ʶ�������Ľ������ */
    INT8U  pkgmap[UPDATE_PKG_MAP_MAX_SIZE]; 
} Priv_t;

/*
********************************************************************************
* ��̬����
********************************************************************************
*/
static Priv_t s_priv;
static INT8U  s_page_buf[APP_PAGE_SIZE] = {0}; 

/*******************************************************************
** ������:      _save_update_flag
** ��������:    ����update��־��1��ʾ��Ҫ����
** ����:        [in] flag
** ����:        none
********************************************************************/
static void _save_update_flag(INT8U flag)
{
    INT8U cnt = 0;

    DATA_EEPROM_Unlock();	
Retry:    
    IWDG_ReloadCounter();

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
** ������:      _reset_update_para
** ��������:    ��λ��������
** ����:        none
** ����:        none
********************************************************************/
static void _reset_update_para(void)
{
    memset(&s_priv, 0, sizeof(s_priv));
}

/*******************************************************************
** ������:      _get_bit_in_array
** ��������:    ����������������package map���ж�Ӧ��λ
** ����:        [in] idx
** ����:        true or false
********************************************************************/
static BOOLEAN _get_bit_in_array(INT16U idx)
{
	INT8U ret = 0;
	INT8U arrayIndex = idx / 8;
	INT8U byteOffset = idx % 8;

	ret = s_priv.pkgmap[arrayIndex] & (0x1 << byteOffset);
	return (ret != 0)?(TRUE):(FALSE);
}

/*******************************************************************
** ������:      _set_bit_in_array
** ��������:    ������������λpackage map ��ָ����λ
** ����:        [in] idx
** ����:        true or false
********************************************************************/
static BOOLEAN _set_bit_in_array(INT16U idx)
{
	INT8U arrayIndex = idx / 8;
	INT8U byteOffset = idx % 8;

    if (arrayIndex < UPDATE_PKG_MAP_MAX_SIZE) {
    	s_priv.pkgmap[arrayIndex] |= (0x1 << byteOffset);
        return TRUE;
    } else {
        return FALSE;
    }
}

#if 0
/*******************************************************************
** ������:      _reset_bit_in_array
** ��������:    ���������Ÿ�λpackage map ��ָ����λ
** ����:        [in] idx
** ����:        true or false
********************************************************************/
static BOOLEAN _reset_bit_in_array(INT16U idx)
{
	INT8U arrayIndex = idx / 8;
	INT8U byteOffset = idx % 8;

    if (arrayIndex < UPDATE_PKG_MAP_MAX_SIZE) {
    	s_priv.pkgmap[arrayIndex] &= ~(0x1 << byteOffset);
        return TRUE;
    } else {
        return FALSE;
    }
}
#endif /* if 0. 2016-1-31 1:10:07 suyoujiang */

/*******************************************************************
** ������:      _write_page
** ��������:    ��flash��д��һ��ҳ
** ����:        [in] offset
**              [in] pdata
**              [in] len
** ����:        true or false
********************************************************************/
static BOOLEAN _write_page(INT32U offset, INT8U *pData, INT32U len)
{
    BOOLEAN ret = TRUE;
	INT32U statrAddr = 0;
	INT32U off = 0;
	INT32U data = 0;

    if (((offset + len) > APP_MAX_SIZE) || (len != APP_PAGE_SIZE)) {
        return FALSE;
    }

	statrAddr = APP_START_ADDR + offset;
	bsp_watchdog_feed();
	FLASH_Unlock();
  
    if (FLASH_ErasePage(statrAddr) != FLASH_COMPLETE) {
    	ret = FALSE;
    } else {
    	off = 0;

    	while (off + 3 < APP_PAGE_SIZE) {
    		data = *(INT32U *)(pData + off);                                   /* ע����뱣֤�����pData��4�ֽڶ��룬����д������ݽ������ */
    		if (FLASH_FastProgramWord(statrAddr + off, data) == FLASH_COMPLETE) {
    			off = off + sizeof(INT32U);
    		} else { 
    			ret = FALSE;
    			break;
    		}
    	}
    }

	FLASH_Lock();
	
	return ret;
}

/*******************************************************************
** ������:      _read_page
** ��������:    ��ȡflash����
** ����:        [in] offset
**              [in] pdata
**              [in] len
** ����:        true or false
********************************************************************/
static BOOLEAN _read_page(INT32U offset, INT8U *pdata, INT32U len)
{
	INT32U statrAddr;

    if ((offset + len) > APP_MAX_SIZE) {
        return FALSE;
    }

	statrAddr = APP_START_ADDR + offset;
	bsp_watchdog_feed();
	
	while (len-- > 0) {
        *pdata++ = *(__IO uint8_t *)(statrAddr++);
	}
	
	return TRUE;
}

static BOOLEAN _write_update_pgk(INT16U pkg, INT8U *pData, INT32U len)
{
    BOOLEAN ret = TRUE;
	INT16U targetPage = pkg * s_priv.info.pkgsize / APP_PAGE_SIZE;
	INT8U  pageOff    = pkg * s_priv.info.pkgsize - APP_PAGE_SIZE * targetPage;
	INT32U left = 0;
	
	if (_read_page(targetPage * APP_PAGE_SIZE, s_page_buf, APP_PAGE_SIZE) == FALSE) {
        ret = FALSE;
    }
		
	if (pageOff + len < APP_PAGE_SIZE) {
		memcpy(s_page_buf + pageOff, pData, len);
		if (_write_page(targetPage * APP_PAGE_SIZE, s_page_buf, APP_PAGE_SIZE) == FALSE) {
            ret = FALSE;
        }
	} else {
		memcpy(s_page_buf + pageOff, pData, APP_PAGE_SIZE - pageOff);
		if (_write_page(targetPage * APP_PAGE_SIZE, s_page_buf, APP_PAGE_SIZE) == FALSE) {
            ret = FALSE;
		}
		left = len + pageOff - APP_PAGE_SIZE;

		if (left > 0) {
            targetPage++;
			if (_read_page(targetPage * APP_PAGE_SIZE, s_page_buf, APP_PAGE_SIZE) == FALSE) {
                ret = FALSE;
    		}
			memcpy(s_page_buf, pData + APP_PAGE_SIZE - pageOff, left);
			if (_write_page(targetPage * APP_PAGE_SIZE, s_page_buf, APP_PAGE_SIZE) == FALSE) {
                ret = FALSE;
    		}
		}
	}
	return ret;
}

/*******************************************************************
** ������:      _check_update
** ��������:     ����Ƿ��������
** ����:        none
** ����:        none
********************************************************************/
static void _check_update(void)
{
	INT16U i, len;
    static INT16U crc16 = 0xffff;
    INT32U left;
	
	for (i = 0; i < s_priv.total_pkg; i++) {
		if (_get_bit_in_array(i) != TRUE) {                                    /* ������δ������ */
            return;
		}
	}

    left = s_priv.info.filesize;
    for (i = 0; i < s_priv.total_page; i++) {
    	_read_page(i * APP_PAGE_SIZE, s_page_buf, APP_PAGE_SIZE);

    	if (left >= APP_PAGE_SIZE) {
    		len = APP_PAGE_SIZE;
        } else {
    		len = left;
        }
    	
    	if (i == 0) {
    		ut_crc16((INT8U *)&crc16, s_page_buf, len);
        } else {
    		ut_crc16_separate((INT8U *)&crc16, s_page_buf, len);
        }

    	left -= len;
    }

    if (crc16 == s_priv.info.filecrc) {
		_save_update_flag(1);
    	__set_FAULTMASK(1);
    	NVIC_SystemReset();
    } else {
        SYS_DEBUG("<---update file crc fault!!!>\n");
        crc16 = 0xffff;
        _reset_update_para();
    }
}

/*******************************************************************
** ������:      _handle_0x8210
** ��������:    ������Ϣ�·�
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8210(Comm_pkt_recv_t *packet)
{
    Stream_t rstrm, wstrm;
    INT8U result = 0, sendbuf[3];
    INT16U cur_ver;
    Update_info_t info;
    Comm_pkt_send_t send_pkt;
    
    if (10 != packet->len) {
        result = 3;                                                            /* ���ݳ����������ݳ���Ϊ10byte���μ�Э�� */
        goto EXIT;
    }

    stream_init(&rstrm, packet->pdata, packet->len);
    info.version = stream_read_half_word(&rstrm);

    if (stream_read_byte(&rstrm) == 0) {                                       /* ����ǿ����������Ҫ�жϰ汾�� */
        cur_ver = atoi(SOFTWARE_VERSION_STR);
        if (info.version < cur_ver) {
            result = 2;                                                        /* �����汾��С�ڵ�ǰ�汾�� */
            goto EXIT;        
        } else if (info.version == cur_ver) {
            result = 1;                                                        /* �����汾�ŵ��ڵ�ǰ�汾�� */
            goto EXIT;   
        }  
    }

    info.filesize = stream_read_long(&rstrm);
    info.filecrc  = stream_read_half_word(&rstrm);

    info.pkgsize = stream_read_byte(&rstrm);
    if (info.pkgsize > UPDATE_PKG_MAX_SIZE) {
        result = 3;
        goto EXIT;
    } 

    if (memcmp(&s_priv.info, &info, sizeof(info)) != 0) {                      /* ����·���������Ϣ��֮ǰ�洢�Ĳ�һ�� */
        memcpy(&s_priv.info, &info, sizeof(info));
        memset(s_priv.pkgmap, 0, UPDATE_PKG_MAP_MAX_SIZE);                     /* �����pkgmap��ʶλ������֮ǰ���չ������� */

    	s_priv.total_pkg = (s_priv.info.filesize % s_priv.info.pkgsize == 0) ? 
    		(s_priv.info.filesize / s_priv.info.pkgsize) : (s_priv.info.filesize / s_priv.info.pkgsize + 1);
    	s_priv.total_page = (s_priv.info.filesize % APP_PAGE_SIZE == 0) ? 
    		(s_priv.info.filesize / APP_PAGE_SIZE) : (s_priv.info.filesize / APP_PAGE_SIZE + 1);
    }
    
EXIT:    
    if (!packet->ack) {                                                        /* ����Ӧ�� */
        return;
    }

    stream_init(&wstrm, sendbuf, sizeof(sendbuf));
    stream_write_half_word(&wstrm, packet->flowseq);
    stream_write_byte(&wstrm, result);

    send_pkt.len   = stream_get_len(&wstrm);
    send_pkt.pdata = sendbuf;
    send_pkt.msgid = 0x0210;
    comm_send_dirsend(&send_pkt);
}

/*******************************************************************
** ������:      _handle_0x8211
** ��������:    �������ݰ��·�
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8211(Comm_pkt_recv_t *packet)
{
    Stream_t rstrm, wstrm;
    INT8U result = 0, sendbuf[3];
    INT16U pkgidx, crc, calc_crc, calc_pkgsize = 0;
    Comm_pkt_send_t send_pkt;

    if (packet->len < 10) {
        result = 1;                                                            /* ���ݳ������� */
        goto EXIT;
    }

    stream_init(&rstrm, packet->pdata, packet->len);
    
    if (stream_read_long(&rstrm) != s_priv.info.filesize) {
        result = 1;
        goto EXIT;
    }

    if (stream_read_half_word(&rstrm) != s_priv.info.filecrc) {
        result = 1;
        goto EXIT;
    }

    pkgidx = stream_read_half_word(&rstrm);
    crc    = stream_read_half_word(&rstrm);

    if (pkgidx < s_priv.total_pkg - 1) {
    	calc_pkgsize = s_priv.info.pkgsize;
    } else if (pkgidx == s_priv.total_pkg - 1) {                               /* ���һ�� */
    	calc_pkgsize = s_priv.info.filesize - pkgidx * s_priv.info.pkgsize;
    }

    if ((pkgidx >= s_priv.total_pkg) || (stream_get_left_len(&rstrm) != calc_pkgsize)) {
        result = 1;
        goto EXIT;
    }

    calc_crc = 0;                                                              /* ������0������ᴫ�벻ȷ��ֵ��У�麯���� */
    ut_crc16((INT8U *)&calc_crc, stream_get_pointer(&rstrm), stream_get_left_len(&rstrm));
    if (calc_crc != crc) {
    	SYS_DEBUG("< update crc error!!!>\n");
        result = 1;
        goto EXIT;
    }

    if (_get_bit_in_array(pkgidx) == FALSE) {
        if (_write_update_pgk(pkgidx, stream_get_pointer(&rstrm), stream_get_left_len(&rstrm))) {
        	_set_bit_in_array(pkgidx);
        } else {
        	SYS_DEBUG("< _write_update_pgk fail>\n");
        }
    	_check_update();
    }

EXIT:    
    if (!packet->ack) {                                                        /* ����Ӧ�� */
        return;
    }

    stream_init(&wstrm, sendbuf, sizeof(sendbuf));
    stream_write_half_word(&wstrm, packet->flowseq);
    stream_write_byte(&wstrm, result);

    send_pkt.len   = stream_get_len(&wstrm);
    send_pkt.pdata = sendbuf;
    send_pkt.msgid = 0x0211;
    comm_send_dirsend(&send_pkt);
}

/*******************************************************************
** ������:      _handle_0x8212
** ��������:    ����״̬��ѯ
** ����:        [in] packet:
** ����:        none
********************************************************************/
static void _handle_0x8212(Comm_pkt_recv_t *packet)
{
    #define SENDBUF_SIZE (2+4+2+UPDATE_PKG_MAP_MAX_SIZE)
    Stream_t wstrm;
    INT8U *psendbuf;
    Comm_pkt_send_t send_pkt;

    if (!packet->ack) {                                                        /* ����Ӧ�� */
        return;
    }

    psendbuf = (INT8U*)mem_malloc(SENDBUF_SIZE);
    if (psendbuf == NULL) {
        return;
    }
    stream_init(&wstrm, psendbuf, SENDBUF_SIZE);
    stream_write_half_word(&wstrm, s_priv.info.version);
    stream_write_long(&wstrm, s_priv.info.filesize);
    stream_write_half_word(&wstrm, s_priv.info.filecrc);
    stream_write_data(&wstrm, s_priv.pkgmap, UPDATE_PKG_MAP_MAX_SIZE);

    send_pkt.len   = stream_get_len(&wstrm);
    send_pkt.pdata = psendbuf;
    send_pkt.msgid = 0x0212;
    comm_send_dirsend(&send_pkt);
    mem_free(psendbuf);
}

/*
********************************************************************************
* ע��ص�����
********************************************************************************
*/
static const FUNCENTRY_COMM_T s_functionentry[] = {
        0x8210, _handle_0x8210       /* ������Ϣ�·� */
       ,0x8211, _handle_0x8211       /* ���������·� */
       ,0x8212, _handle_0x8212       /* ����״̬��ѯ */
};
static const INT8U s_funnum = sizeof(s_functionentry) / sizeof(s_functionentry[0]);

/*******************************************************************
** ������:      update_init
** ��������:    Զ��������ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void update_init(void)
{
	INT8U i;

    _reset_update_para();

    for (i = 0; i < s_funnum; i++) {
        comm_recv_register(s_functionentry[i].index, s_functionentry[i].entryproc);
    }
}
#endif /* end of EN_UPDATE */

