/********************************************************************************
**
** �ļ���:     roundbuf.c
** ��Ȩ����:   (c) 2007-2008 ������Ѹ����ɷ����޹�˾
** �ļ�����:   ʵ��ѭ��������������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����       | ����   |  �޸ļ�¼
**===============================================================================
**| 2007/04/11 | �´ӻ� |  �������ļ�
**| 2008/10/13 | �θ�   |  ʹ��ԭ��̨����ʵ�ֺͽӿ�,���㳵̨������ֲ
*********************************************************************************/
#include "sys_typedef.h"
#include "roundbuf.h"
/*******************************************************************
** ������:     roundbuf_init
** ��������:   ��ʼ��ѭ��������
** ����:       [in]  round:           ѭ��������
**             [in]  mem:             ѭ����������������ڴ��ַ
**             [in]  memsize:         ѭ����������������ڴ��ֽ���
**             [in]  rule:            ѭ�����������ݶ�ȡ�ǵĽ������
** ����:       ��
********************************************************************/
void roundbuf_init(RoundBuf_t *round, INT8U *mem, INT32U memsize, AsmRule_t *rule)
{
    round->bufsize = memsize;
    round->used    = 0;
    round->bptr    = mem;
    round->eptr    = mem + memsize;
    round->wptr = round->rptr = round->bptr;
    round->rule = rule;
}

/*******************************************************************
** ������:     roundbuf_reset
** ��������:   ��λѭ��������, ����ѭ������������ʹ���ֽ�����0
** ����:       [in]  round:           ѭ��������
** ����:       ��
********************************************************************/
void roundbuf_reset(RoundBuf_t *round)
{
    round->used = 0;
    round->rptr = round->wptr = round->bptr;
}

/*******************************************************************
** ������:     roundbuf_get_start_pos
** ��������:   ��ȡѭ���������������ڴ����ʼָ��
** ����:       [in]  round:           ѭ��������
** ����:       �������ڴ����ʼָ��
********************************************************************/
INT8U *roundbuf_get_start_pos(RoundBuf_t *round)
{
    return round->bptr;
}

/*******************************************************************
** ������:     roundbuf_write_byte
** ��������:   ��ѭ����������д��һ���ֽڵ�����
** ����:       [in]  round:           ѭ��������
**             [in]  data:              ��д�������
** ����:       ��д֮ǰѭ���������е�ʹ���ֽ����ﵽ�������ڴ���ֽ���, 
**             �򷵻�ʧ��; ����, ���سɹ�
********************************************************************/
BOOLEAN roundbuf_write_byte(RoundBuf_t *round, INT8U data)
{
    if(NULL == round) return FALSE;
    if (round->used >= round->bufsize) return FALSE;
    *round->wptr++ = data;
    if (round->wptr >= round->eptr) {
        round->wptr = round->bptr;
    }
    round->used++;
    return TRUE;
}

/*******************************************************************
** ������:     roundbuf_read_byte
** ��������:   ��ѭ���������ж�ȡһ���ֽڵ�����
** ����:       [in]  round:           ѭ��������
** ����:       ���֮ǰѭ���������е�ʹ���ֽ���Ϊ0, �򷵻�-1;
**             ����, ���ض�ȡ�����ֽ�
********************************************************************/
INT32S roundbuf_read_byte(RoundBuf_t *round)
{
    INT32S ret;
    
    if (round->used == 0) return -1;
    ret = *round->rptr++;
    if (round->rptr >= round->eptr) {
        round->rptr = round->bptr;
    }
    round->used--;
    return ret;
}

/*******************************************************************
** ������:     roundbuf_read_data
** ��������:   ��ѭ��������ָ������
** ����:       [in]  round:           ѭ��������
**             [out] pdata:           Ҫ�����ݿ��ָ��
**             [in]  datalen:         Ҫ�����ݿ���ֽ���
** ����:       ʵ�ʶ������ݳ���
********************************************************************/
INT32S roundbuf_read_data(RoundBuf_t *round, INT8U *pdata, INT32U datalen)
{
    INT32U i, readlen;

    if (round->used <= datalen) {
        readlen = round->used;
    } else {
        readlen = datalen;
    }

    for (i = 0; i < readlen; i++) {

        pdata[i] = *round->rptr++;
        if (round->rptr >= round->eptr) {
            round->rptr = round->bptr;
        }
        round->used--;
    }
    
    return readlen;
}

/*******************************************************************
** ������:     roundbuf_read_byte_no_move_ptr
** ��������:   �ٲ��ƶ�������ָ��������,��ѭ���������ж�ȡһ���ֽڵ�����
** ����:       [in]  round:           ѭ��������
** ����:       ʣ��Ŀ����ֽ���
********************************************************************/
INT32S roundbuf_read_byte_no_move_ptr(RoundBuf_t *round)
{
    if (round->used == 0) return -1;
    else return *round->rptr; 
}

/*******************************************************************
** ������:     roundbuf_get_left
** ��������:   ��ȡѭ����������ʣ��Ŀ����ֽ���
** ����:       [in]  round:           ѭ��������
** ����:       ʣ��Ŀ����ֽ���
********************************************************************/
INT32U roundbuf_get_left(RoundBuf_t *round)
{
    return (round->bufsize - round->used);
}

/*******************************************************************
** ������:     roundbuf_get_used
** ��������:   ��ȡѭ������������ʹ���ֽ���
** ����:       [in]  round:           ѭ��������
** ����:       ��ʹ�õ��ֽ���
********************************************************************/
INT32U roundbuf_get_used(RoundBuf_t *round)
{
       return round->used;
}

/*******************************************************************
** ������:     roundbuf_write_block
** ��������:   ��ѭ����������д��һ�����ݵ�Ԫ
** ����:       [in]  round:           ѭ��������
**             [in]  bptr:            ��д�����ݿ��ָ��
**             [in]  blksize:         ��д�����ݿ���ֽ���
** ����:       �ɹ�д��ѭ�����������ֽ���
********************************************************************/
BOOLEAN roundbuf_write_block(RoundBuf_t *round, INT8U *bptr, INT32U blksize)
{
    if (blksize > roundbuf_get_left(round)) return FALSE;
    for (; blksize > 0; blksize--) { 
       *round->wptr++ = *bptr++;
       if (round->wptr >= round->eptr) round->wptr = round->bptr;
       round->used++;
    }    
    return TRUE;
}

/*******************************************************************
** ������:     roundbuf_deassemble
** ��������:   ��ȡ�������е����ݲ����չ�����н���
** ����:       [in]  round:           ѭ��������
**             [in]  bptr:            ���������ݿ��ָ��
**             [in]  blksize:         ����ȡ���ݿ������ֽ���
** ����:       ��ȡ�Ľ������ֽ���
********************************************************************/
INT32U roundbuf_deassemble(RoundBuf_t *round, INT8U *dptr, INT32U maxlen)
{
    INT8U  *rptr, *tptr;
    INT8U  cur, pre = 0;
    INT32U used, ct;
    INT32S rlen;
    
    if (round->rule == NULL) return 0;
    
    rlen = -1;
    ct   = 0;
    tptr = dptr;
    rptr = round->rptr;
    used = round->used;
    while (used > 0) {
        cur = *rptr++;
        used--;
        ct++;
        if (rptr >= round->eptr) rptr = round->bptr;
        if (rlen == -1) {                                   /* search flags */
            if (cur == round->rule->c_flags) {
                rlen = 0;
                pre  = 0;
            }
            continue;
        } else if (rlen == 0) {
            if (cur == round->rule->c_flags) continue;
        } else {
            if (cur == round->rule->c_flags) {
                round->rptr  = rptr;
                round->used -= ct;
                return rlen;
            }
        }
        if (pre == round->rule->c_convert0) {               /* search convert character */
            if (cur == round->rule->c_convert1)             /* c_flags    = c_convert0 + c_convert1 */
                *tptr++ = round->rule->c_flags;
            else if (cur == round->rule->c_convert2)        /* c_convert0 = c_convert0 + c_convert2 */
                *tptr++ = round->rule->c_convert0;
            else {
                round->rptr  = rptr;                        /* detect error frame! */
                round->used -= ct;
                ct = 0;
                tptr = dptr;
                rlen = -1;
                continue;
            }
        } else {
            if (cur != round->rule->c_convert0)             /* search convert character */
                *tptr++ = cur;
            rlen++;
        }
        pre = cur;
        if (rlen >= maxlen) {
            round->rptr  = rptr;
            round->used -= ct;
            ct = 0;
            tptr = dptr;
            rlen = -1;
            continue;
        }
    }
    return 0;
}

