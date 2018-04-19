/********************************************************************************
**
** �ļ���:     stream.c
** ��Ȩ����:   (c) 2013-2014 ������������ͨ�������Ƽ����޹�˾
** �ļ�����:   ʵ��������������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2014/12/18 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "sys_typedef.h"
#include "stream.h"
#include <stdio.h>
#include <stdarg.h>
/*
********************************************************************************
* ��������
********************************************************************************
*/
#ifndef	 CR
#define  CR                      0x0D
#endif

#ifndef  LF
#define  LF                      0x0A
#endif

/*******************************************************************
** ������:     stream_init
** ��������:   ��ʼ��������
** ����:       [in] pstrm:  ������
**             [in] pbuf:   �������������ڴ��ַ
**             [in] buflen: �������������ڴ��ֽ���
** ����:       �ɹ���ʧ��
********************************************************************/
BOOLEAN stream_init(Stream_t *pstrm, INT8U *pbuf, INT16U buflen)
{
    if (pstrm == 0) return FALSE;
	
    pstrm->len      = 0;
    pstrm->maxlen   = buflen;
    pstrm->current  = pbuf;
    pstrm->start    = pbuf;
    return TRUE;
}

/*******************************************************************
** ������:     stream_get_len
** ��������:   ��ȡ�������������ֽ���
** ����:       [in] pstrm: ������
** ����:       �����������ֽ���
********************************************************************/
INT16U stream_get_len(Stream_t *pstrm)
{
    return (pstrm->len);
}

/*******************************************************************
** ������:     stream_get_left_len
** ��������:   ��ȡ��������ʣ��Ŀ����ֽ���
** ����:       [in] pstrm: ������
** ����:       ������ʣ��Ŀ����ֽ���
********************************************************************/
INT16U stream_get_left_len(Stream_t *pstrm)
{
    if (pstrm->maxlen >= pstrm->len) {
        return (pstrm->maxlen - pstrm->len);
    } else {
        return 0;
    }
}

/*******************************************************************
** ������:     stream_get_maxlen
** ��������:   ��ȡ�����������ܳ���
** ����:       [in] pstrm: ������
** ����:       ����������󳤶�
********************************************************************/
INT16U stream_get_maxlen(Stream_t *pstrm)
{
    return (pstrm->maxlen);
}

/*******************************************************************
** ������:     stream_get_pointer
** ��������:   ��ȡ��������ǰ��/дָ��
** ����:       [in] pstrm: ������
** ����:       ��ǰ��/дָ��
********************************************************************/
INT8U *stream_get_pointer(Stream_t *pstrm)
{
    return (pstrm->current);
}

/*******************************************************************
** ������:     stream_get_start_pointer
** ��������:   ��ȡ�������������ڴ�ĵ�ַ
** ����:       [in] pstrm: ������
** ����:       �������ڴ��ַ
********************************************************************/
INT8U *stream_get_start_pointer(Stream_t *pstrm)
{
    return (pstrm->start);
}

/*******************************************************************
** ������:     stream_move_pointer
** ��������:   �ƶ��������ж�/дָ��
** ����:       [in] pstrm: ������
**             [in] len:   �ƶ��ֽ���
** ����:       ��
********************************************************************/
void stream_move_pointer(Stream_t *pstrm, INT16U len)
{
    if (pstrm != 0) {
        if ((pstrm->len + len) <= pstrm->maxlen) {
            pstrm->len     += len;
            pstrm->current += len;
        } else {
            pstrm->len = pstrm->maxlen;
        }
    }
}

/*******************************************************************
** ������:     stream_write_byte
** ��������:   ����������д��һ���ֽ�����
** ����:       [in] pstrm:     ������
**             [in] writebyte: д�������
** ����:       ��
********************************************************************/
void stream_write_byte(Stream_t *pstrm, INT8U writebyte)
{
    if (pstrm != 0) {
        if (pstrm->len < pstrm->maxlen) {
            *pstrm->current++ = writebyte;
            pstrm->len++;
        }
    }
}

/*******************************************************************
** ������:     stream_write_half_word
** ��������:   ����������д��һ������(16λ)����, ���ģʽ(���ֽ���д�����ֽں�д)
** ����:       [in] pstrm:     ������
**             [in] writeword: д�������
** ����:       ��
********************************************************************/
void stream_write_half_word(Stream_t *pstrm, INT16U writeword)
{
    HWORD_UNION temp;
    
    temp.hword = writeword;
    stream_write_byte(pstrm, temp.bytes.high);
    stream_write_byte(pstrm, temp.bytes.low);
}

/*******************************************************************
** ������:     stream_write_little_half_word
** ��������:   ����������д��һ������(16λ)����, С��ģʽ(���ֽ���д�����ֽں�д)
** ����:       [in] pstrm:     ������
**             [in] writeword: д�������
** ����:       ��
********************************************************************/
void stream_write_little_half_word(Stream_t *pstrm, INT16U writeword)
{
    HWORD_UNION temp;
    
    temp.hword = writeword;
    stream_write_byte(pstrm, temp.bytes.low);     
    stream_write_byte(pstrm, temp.bytes.high);
}

/*******************************************************************
** ������:     stream_write_long
** ��������:   ����������д��һ������(32λ)����, ���ģʽ(���ֽ���д�����ֽں�д)
** ����:       [in] pstrm:     ������
**             [in] writeword: д�������
** ����:       ��
********************************************************************/
void stream_write_long(Stream_t *pstrm, INT32U writelong)
{
    stream_write_half_word(pstrm, writelong >> 16);
    stream_write_half_word(pstrm, writelong);
}

/*******************************************************************
** ������:     stream_write_little_long
** ��������:   ����������д��һ������(32λ)����, С��ģʽ(���ֽ���д�����ֽں�д)
** ����:       [in] pstrm:     ������
**             [in] writeword: д�������
** ����:       ��
********************************************************************/
void stream_write_little_long(Stream_t *pstrm, INT32U writelong)
{
    stream_write_little_half_word(pstrm, writelong);
    stream_write_little_half_word(pstrm, writelong >> 16);                     /* ��16λ */
}

/*******************************************************************
** ������:     stream_write_linefeed
** ��������:   ����������д�뻻�з�, ��д��'\r'��'\n'
** ����:       [in] pstrm: ������
** ����:       ��
********************************************************************/
void stream_write_linefeed(Stream_t *pstrm)
{
    stream_write_byte(pstrm, CR);
    stream_write_byte(pstrm, LF);
}

/*******************************************************************
** ������:     stream_write_enter
** ��������:   ����������д��س���, ��д��'\r''
** ����:       [in] pstrm: ������
** ����:       ��
********************************************************************/
void stream_write_enter(Stream_t *pstrm)
{
    stream_write_byte(pstrm, CR);
}

/*******************************************************************
** ������:     stream_write_string
** ��������:   ����������д���ַ���
** ����:       [in] pstrm: ������
**             [in] ptr:   д����ַ���ָ��
** ����:       ��
********************************************************************/
void stream_write_string(Stream_t *pstrm, char *ptr)
{
    while(*ptr)
    {
        stream_write_byte(pstrm, *ptr++);
    }
}

/*******************************************************************
** ������:      stream_write_sprintf
** ��������:    ������ʵ��sprintf����
** ����:        [in] pstrm  : ������
**              [in] pFormat: ��ʽ���ַ���
** ����:        �ַ������ȣ�strlen��
********************************************************************/
INT32S stream_write_sprintf(Stream_t *pstrm, const char* pFormat,...)
{
	va_list arg; 
	INT32S done = 0; 

    if (pstrm == NULL) {
    	return done;
    }

	va_start(arg, pFormat); 
	done += vsprintf((char*)pstrm->current, pFormat, arg); 
	va_end(arg);

    stream_move_pointer(pstrm, done);
	return done;
}

/*******************************************************************
** ������:     stream_write_data
** ��������:   ����������д��һ���ڴ�����
** ����:       [in] pstrm: ������
**             [in] ptr:   д������ݿ��ַ
**             [in] len:   д������ݿ��ֽ���
** ����:       ��
********************************************************************/
void stream_write_data(Stream_t *pstrm, INT8U *ptr, INT16U len)
{
    while(len--)
    {
        stream_write_byte(pstrm, *ptr++);
    }
}

/*******************************************************************
** ������:     stream_write_data_back
** ��������:   ����������д��һ���ڴ�����,�Ӹߵ�ַ~�͵�ַ�����δ��
** ����:       [in] pstrm: ������
**             [in] ptr:   д������ݿ��ַ
**             [in] len:   д������ݿ��ֽ���
** ����:       ��
********************************************************************/
void stream_write_data_back(Stream_t *pstrm, INT8U *ptr, INT16U len)
{
    ptr += len - 1;
    while(len--)
    {
        stream_write_byte(pstrm, *ptr--);
    }
}

/*******************************************************************
** ������:     stream_read_byte
** ��������:   ���������ж�ȡһ���ֽ�
** ����:       [in] pstrm: ������
** ����:       ��ȡ�����ֽ�
********************************************************************/
INT8U stream_read_byte(Stream_t *pstrm)
{
    pstrm->len++;
    return (*pstrm->current++);
}

/*******************************************************************
** ������:     stream_read_half_word
** ��������:   ���������ж�ȡһ������(16λ)����,���ģʽ(�ȶ�Ϊ���ֽڣ����Ϊ���ֽ�)
** ����:       [in] pstrm: ������
** ����:       ��ȡ������
********************************************************************/
INT16U stream_read_half_word(Stream_t *pstrm)
{
    HWORD_UNION temp = {0};
	
    temp.bytes.high = stream_read_byte(pstrm);
    temp.bytes.low  = stream_read_byte(pstrm);
    return temp.hword;
}

/*******************************************************************
** ������:     stream_read_little_half_word
** ��������:   ���������ж�ȡһ������(16λ)����,С��ģʽ(�ȶ�Ϊ���ֽڣ����Ϊ���ֽ�)
** ����:       [in] pstrm: ������
** ����:       ��ȡ������
********************************************************************/
INT16U stream_read_little_half_word(Stream_t *pstrm)
{
    HWORD_UNION temp = {0};
	
    temp.bytes.low   = stream_read_byte(pstrm);
    temp.bytes.high  = stream_read_byte(pstrm);
    return temp.hword;
}

/*******************************************************************
** ������:     stream_read_long
** ��������:   ���������ж�ȡһ������(32λ)����,���ģʽ(�ȶ�Ϊ���ֽڣ����Ϊ���ֽ�)
** ����:       [in] pstrm: ������
** ����:       ��ȡ������
********************************************************************/
INT32U stream_read_long(Stream_t *pstrm)
{
    INT32U temp;
	
	temp = (stream_read_half_word(pstrm) << 16);
	temp += stream_read_half_word(pstrm);
    
    return temp;
}

/*******************************************************************
** ������:     stream_read_little_long
** ��������:   ���������ж�ȡһ������(32λ)����,С��ģʽ(�ȶ�Ϊ���ֽڣ����Ϊ���ֽ�)
** ����:       [in] pstrm: ������
** ����:       ��ȡ������
********************************************************************/
INT32U stream_read_little_long(Stream_t *pstrm)
{
    INT32U temp;
	
	temp = stream_read_little_half_word(pstrm);
	temp += (stream_read_little_half_word(pstrm) << 16);
    
    return temp;
}

/*******************************************************************
** ������:     stream_read_data
** ��������:   ���������ж�ȡָ�����ȵ���������
** ����:       [in]  pstrm: ������
**             [out] ptr:   ��ȡ�������ݴ�ŵ��ڴ��ַ
**             [in]  len:   ��ȡ�����ݳ���
** ����:       ��
********************************************************************/
void stream_read_data(Stream_t *pstrm, INT8U *ptr, INT16U len)
{
    while(len--)
    {
        *ptr++ = stream_read_byte(pstrm);
    }
}

/*******************************************************************
** ������:     stream_read_data_back
** ��������:   ���������ж�ȡָ�����ȵ���������,�Ӹߵ�ַ~�͵�ַ�����δ��
** ����:       [in]  pstrm: ������
**             [out] ptr:   ��ȡ�������ݴ�ŵ��ڴ��ַ
**             [in]  len:   ��ȡ�����ݳ���
** ����:       ��
********************************************************************/
void stream_read_data_back(Stream_t *pstrm, INT8U *ptr, INT16U len)
{
    ptr += len - 1;
    while(len--)
    {
        *ptr-- = stream_read_byte(pstrm);
    }
}

