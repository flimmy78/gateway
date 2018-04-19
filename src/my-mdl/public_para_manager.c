/********************************************************************************
**
** �ļ���:     public_para_manager.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ʵ�ֹ��������ļ��洢��������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/25 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"
#include "public_para_manager.h"

#if DBG_PBULIC_PARA > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif

/*
********************************************************************************
* ����ģ�����ò���
********************************************************************************
*/
// para operation attrib
#define WF_                  0x01        /* need write to file_xx */
#define WS_                  0x02        /* need save to simbak */
#define RS_                  0x04        /* need read from simbak */
#define CL_                  0x08        /* need clear para in file_xx */


#define VALID_               'V'
#define BOUND_FLAG           'F'
#define PP_LOCK              'L'
#define PERIOD_UPDATE        1*SECOND

/*
********************************************************************************
* ����ģ�����ݽṹ
********************************************************************************
*/
typedef struct {
    INT8U   id;
    void    (*informer)(INT8U reason);
} INFORM_REG_T;

typedef struct {
    INT8U           valid;             /* ������Ч */
    INT8U           attrib;            /* �������� */
    INT16U          space;             /* һ����¼ռ�õĿռ� */
    INT32U          offset;            /* ����λ�� */
    PP_REG_T const *preg;
} PP_T;

typedef struct {
    INT8U             fp;
    INT16U            ct_delay;
    INT32U            filesize;
    PP_CLASS_T const *pclass;
} CLASS_T;

typedef struct {
    INT8U        lock;
    PP_T         pp[MAX_PP_NUM];
    CLASS_T      cls[MAX_PP_CLASS_NUM];
    INFORM_REG_T inform[MAX_PP_INFORM];
} DCB_T;

/*
********************************************************************************
* ����ģ�龲̬����
********************************************************************************
*/
static DCB_T s_dcb;
static INT8U s_updatetmr;

/*******************************************************************
** ������:      _check_para_valid
** ��������:    �жϲ����Ƿ���ȷ
** ����:        [in] ptr������ָ��
**              [in] len����������
** ����:        ��Ч����true����Ч����false
********************************************************************/
static BOOLEAN _check_para_valid(INT8U *ptr, INT16U len)
{
    PP_HEAD_T *phead;
    HWORD_UNION chksum = {0};
	
    phead = (PP_HEAD_T *)ptr;
    chksum.bytes.high = phead->chksum[0];
    chksum.bytes.low  = phead->chksum[1];
    
    if (u_chksum_2(ptr + sizeof(PP_HEAD_T), len - sizeof(PP_HEAD_T) - 1) != chksum.hword) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*******************************************************************
** ������:      _copy_file_to_ram
** ��������:    ����������RAM��
** ����:        [in] cls��������,��PP_CLASS_ID_E
** ����:        �ɹ�����true��ʧ�ܷ���false
********************************************************************/
static BOOLEAN _copy_file_to_ram(INT8U cls)
{
    INT8U i, id;
    PP_REG_T const *preg;
    PP_CLASS_T const *pclass;

    return_val_if_fail((cls < public_para_reg_get_class_max()), RET_FAIL);
    
    pclass = s_dcb.cls[cls].pclass;
    return_val_if_fail((s_dcb.cls[cls].filesize == pclass->memlen), RET_FAIL);

    if (eeprom_read_byte(s_dcb.cls[cls].fp, 0, pclass->memptr, s_dcb.cls[cls].filesize) == 1) {
        return false;
    }
    
    /* �жϸ������� */
    preg   = pclass->preg;
    for (i = 0; i < pclass->nreg; i++) {
        id = preg->id;
        return_val_if_fail((s_dcb.pp[id].space == sizeof(PP_HEAD_T) + preg->rec_size + 1), RET_FAIL);
        
        *(pclass->memptr + s_dcb.pp[id].offset + s_dcb.pp[id].space - 1) = BOUND_FLAG;
        if (_check_para_valid(pclass->memptr + s_dcb.pp[id].offset, s_dcb.pp[id].space)) {/* ������ */
            s_dcb.pp[id].valid = VALID_;
        }
        
        preg++;
    }
    return true;
}

/*******************************************************************
** ������:      _init_public_para
** ��������:    ��ʼ��������������
** ����:        ��
** ����:        ��
********************************************************************/
static BOOLEAN _init_public_para(void)
{
    INT8U i, j, nclass, id, maxid;
    INT32U offset;
    PP_REG_T const *preg;
    PP_CLASS_T const *pclass;
    
    maxid = public_para_reg_get_item_max();
    return_val_if_fail((maxid <= MAX_PP_NUM), RET_FAIL);
    nclass = public_para_reg_get_class_max();
    return_val_if_fail((nclass <= MAX_PP_CLASS_NUM), RET_FAIL);
    
    for (i = 0; i < nclass; i++) {
        pclass = public_para_reg_get_class_info(i);
        preg   = pclass->preg;
        offset = 0;
        for (j = 0; j < pclass->nreg; j++) {
            /* ��ȡע�����Ϣ */
            return_val_if_fail((preg != 0), RET_FAIL);
            return_val_if_fail((preg->id < maxid), RET_FAIL);
            return_val_if_fail((preg->type == pclass->type), RET_FAIL);
            return_val_if_fail((offset + sizeof(PP_HEAD_T) + preg->rec_size + 1 <= pclass->memlen), RET_FAIL);
            
            id = preg->id;
            
            s_dcb.pp[id].preg    = preg;
            s_dcb.pp[id].valid   = 0;
            s_dcb.pp[id].attrib  = 0;
            s_dcb.pp[id].offset  = offset;
            s_dcb.pp[id].space   = sizeof(PP_HEAD_T) + preg->rec_size + 1;
            
            offset += s_dcb.pp[id].space;
            preg++;
        }
        s_dcb.cls[i].pclass   = pclass;
        s_dcb.cls[i].filesize = offset;
        return_val_if_fail((s_dcb.cls[i].filesize == pclass->memlen), RET_FAIL);
        /* ע���ļ� */
        s_dcb.cls[i].fp = eeprom_blank_get(s_dcb.cls[i].filesize);
        return_val_if_fail(s_dcb.cls[i].fp != 0xff, RET_FAIL);
        
        SYS_DEBUG("<_init_public_para, cls:(%d), filesize:(%d)>\r\n", i, offset);
    }
    
    for (i = nclass; i > 0; i--) {
        if (!_copy_file_to_ram(i - 1)) {
            return false;
        }
        SYS_DEBUG("<_copy_file_to_ram, cls:(%d)>\r\n", i - 1);
    }
    return true;
}

/*******************************************************************
** ������:      _flush_ram_to_file
** ��������:    �洢�������ļ�
** ����:        [in] cls��������,��PP_CLASS_ID_E
** ����:        �ɹ�����true��ʧ�ܷ���false
********************************************************************/
static BOOLEAN _flush_ram_to_file(INT8U cls)
{
    INT8U j, id;
    INT32U offset;
    PP_HEAD_T *phead;
    PP_REG_T const *preg;
    PP_CLASS_T const *pclass;
    
    return_val_if_fail((cls == s_dcb.cls[cls].pclass->type), RET_FAIL);
    
    pclass = s_dcb.cls[cls].pclass;
    preg   = pclass->preg;
    for (j = 0; j < pclass->nreg; j++) {
        return_val_if_fail((preg != 0), RET_FAIL);
            
        id = preg->id;
        offset = s_dcb.pp[id].offset;
        phead  = (PP_HEAD_T *)(pclass->memptr + offset);
        return_val_if_fail((*(pclass->memptr + offset + s_dcb.pp[id].space - 1) == BOUND_FLAG), RET_FAIL);
        
        if ((s_dcb.pp[id].attrib & CL_) == CL_) {
            return_val_if_fail((s_dcb.pp[id].valid != VALID_), RET_FAIL);
            return_val_if_fail((phead->chksum[0] == 0 && phead->chksum[1] == 0), RET_FAIL);
            s_dcb.pp[id].attrib &= ~CL_;
                
            if (eeprom_write_byte(s_dcb.cls[cls].fp, s_dcb.pp[id].offset, s_dcb.cls[cls].pclass->memptr + s_dcb.pp[id].offset, s_dcb.pp[id].space)) {
                return false;
            }
        } else if ((s_dcb.pp[id].attrib & WF_) == WF_) {
            return_val_if_fail((s_dcb.pp[id].valid == VALID_), RET_FAIL);
            return_val_if_fail(_check_para_valid((INT8U *)phead, s_dcb.pp[id].space), RET_FAIL);
            s_dcb.pp[id].attrib &= ~WF_;
                
            if (eeprom_write_byte(s_dcb.cls[cls].fp, s_dcb.pp[id].offset, s_dcb.cls[cls].pclass->memptr + s_dcb.pp[id].offset, s_dcb.pp[id].space)) {
                return false;
            }
        }
        preg++;
    }

    return true;
} 

/*******************************************************************
** ������:     _update_tmr_proc
** ��������:   ��ʱ���¶�ʱ������
** ����:       [in]  pdata:  ����δ��
** ����:       ��
********************************************************************/
static void _update_tmr_proc(void *pdata)
{
    INT8U i, nclass;
    PP_CLASS_T const *pclass;
            
    pdata = pdata;    
    os_timer_start(s_updatetmr, PERIOD_UPDATE);
    
    if (s_dcb.lock == PP_LOCK) {
        return;
    }
    
    nclass = public_para_reg_get_class_max();
    return_if_fail((nclass <= MAX_PP_CLASS_NUM));
    for (i = 0; i < nclass; i++) {
        pclass = public_para_reg_get_class_info(i);
        return_if_fail((pclass->dly != 0));
        if (s_dcb.cls[i].ct_delay < pclass->dly) {
            if (++s_dcb.cls[i].ct_delay == pclass->dly) {
                if (!_flush_ram_to_file(i)) {
                    return;
                }
            }
        } else {
            if (i == PP_TYPE_DELAY) {
                s_dcb.cls[i].ct_delay = 0;
            }
        }
    }
}

/*******************************************************************
** ������:      public_para_manager_init
** ��������:    ���������洢������ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void public_para_manager_init(void)
{
    SYS_DEBUG("<public_para_manager_init>\r\n");
    
    memset(&s_dcb, 0, sizeof(s_dcb));
    
    s_dcb.lock = PP_LOCK;
    if (!_init_public_para()) {
        return;
    }
    s_dcb.lock = 0;
        
    s_updatetmr = os_timer_create( 0, _update_tmr_proc);
    os_timer_start(s_updatetmr, PERIOD_UPDATE);
    //OS_RegResetHooker(RESET_HDL_PRIO_LOW, _reset_inform);
}

/*******************************************************************
** ������:     public_para_manager_del_by_class
** ��������:   ɾ��ָ���������Ĳ���
** ����:       [in] cls��������,��PP_CLASS_ID_E
** ����:       �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN public_para_manager_del_by_class(INT8U cls)
{
    INT8U i, id;
    PP_HEAD_T *phead;
    PP_REG_T const *preg;
    PP_CLASS_T const *pclass;
    
    return_val_if_fail((cls < public_para_reg_get_class_max()), RET_FAIL);
    
    pclass = s_dcb.cls[cls].pclass;
    return_val_if_fail((s_dcb.cls[cls].filesize == pclass->memlen), RET_FAIL);
    
    preg   = pclass->preg;
    for (i = 0; i < pclass->nreg; i++) {
        id = preg->id;
        return_val_if_fail((s_dcb.pp[id].space == sizeof(PP_HEAD_T) + preg->rec_size + 1), RET_FAIL);
        
        phead  = (PP_HEAD_T *)(pclass->memptr + s_dcb.pp[id].offset);
        phead->chksum[0] = 0;
        phead->chksum[1] = 0;
    
        s_dcb.pp[id].valid  = 0;
        s_dcb.pp[id].attrib = 0;
        
        preg++;
    }

    if (eeprom_write_byte(s_dcb.cls[cls].fp, 0, pclass->memptr, s_dcb.cls[cls].filesize) == 1) {
        return false;
    }

    os_msg_post(TSK_SYS_MSG, MSG_PP_CHANGE, cls | 0xff00, PP_REASON_RESET, NULL);
    return true;
}

/*******************************************************************
** ������:     public_para_manager_del_all
** ��������:   ɾ�����в������Ĳ���,������Լ�����������
** ����:       ��
** ����:       �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN public_para_manager_del_all(void)
{
    INT8U i, nclass;
    
    nclass = public_para_reg_get_class_max();
    return_val_if_fail((nclass <= MAX_PP_CLASS_NUM), RET_FAIL);
    
    for (i = 0; i < nclass; i++) {
        if (!public_para_manager_del_by_class(i)) {
            return false;
        }
    }
    return true;
}

/*******************************************************************
** ������:     public_para_manager_del_all_and_rst
** ��������:   ɾ�����в������Ĳ���,���븴λ�����豸����ܼ����洢
** ����:       ��
** ����:       ��
********************************************************************/
void public_para_manager_del_all_and_rst(void)
{
    public_para_manager_del_all();
    s_dcb.lock = PP_LOCK;
}

/*******************************************************************
** ������:      public_para_manager_clear_by_id
** ��������:    ���PP����
** ����:        [in] id:  ������ţ���PP_ID_E
** ����:        ��Ч����true����Ч����false
********************************************************************/
BOOLEAN public_para_manager_clear_by_id(INT8U id)
{
    PP_HEAD_T *phead;
    PP_CLASS_T const *pclass;
       
    if (s_dcb.lock == PP_LOCK) {
        return FALSE;
    }
    
    return_val_if_fail((id < public_para_reg_get_item_max()), RET_FAIL);
    return_val_if_fail((id == s_dcb.pp[id].preg->id), RET_FAIL);
    
    pclass = public_para_reg_get_class_info(s_dcb.pp[id].preg->type);
    phead  = (PP_HEAD_T *)(pclass->memptr + s_dcb.pp[id].offset);
    
    return_val_if_fail((*(pclass->memptr + s_dcb.pp[id].offset + s_dcb.pp[id].space - 1) == BOUND_FLAG), RET_FAIL);
    
    memset(((INT8U *)phead), 0, s_dcb.pp[id].space - 1);
    s_dcb.pp[id].valid   = 0;
    s_dcb.pp[id].attrib |= CL_;
    
    s_dcb.cls[pclass->type].ct_delay = 0;
    os_msg_post(TSK_SYS_MSG, MSG_PP_CHANGE, id, PP_REASON_STORE, NULL);
    return TRUE;
}

/*******************************************************************
** ������:      public_para_manager_read_by_id
** ��������:    ��ȡPP�������ж�PP��Ч��
** ����:        [in] id:  ������ţ���PP_ID_E
**              [in] dptr:�������
**              [in] rlen:���泤��
** ����:        ��Ч����true����Ч����false
********************************************************************/
BOOLEAN public_para_manager_read_by_id(INT8U id, INT8U *dptr, INT16U rlen)
{
    PP_HEAD_T *phead;
    PP_CLASS_T const *pclass;
    
    return_val_if_fail((id < public_para_reg_get_item_max()), RET_FAIL);
    return_val_if_fail((rlen >= s_dcb.pp[id].preg->rec_size), RET_FAIL);
    return_val_if_fail((id == s_dcb.pp[id].preg->id), RET_FAIL);
    
    pclass = public_para_reg_get_class_info(s_dcb.pp[id].preg->type);
    phead  = (PP_HEAD_T *)(pclass->memptr + s_dcb.pp[id].offset);
    
    return_val_if_fail((*(pclass->memptr + s_dcb.pp[id].offset + s_dcb.pp[id].space - 1) == BOUND_FLAG), RET_FAIL);
   
    if (s_dcb.pp[id].valid == VALID_) {
        if (_check_para_valid((INT8U *)phead, s_dcb.pp[id].space)) {
            memcpy(dptr, ((INT8U *)phead) + sizeof(PP_HEAD_T), s_dcb.pp[id].preg->rec_size);
            return TRUE;
        } else {
            return_val_if_fail(0, RET_FAIL);
        }
    }
    if (s_dcb.pp[id].preg->i_ptr != 0) {
        memcpy(dptr, s_dcb.pp[id].preg->i_ptr, s_dcb.pp[id].preg->rec_size);
        return TRUE;
    }
    memset(dptr, 0, s_dcb.pp[id].preg->rec_size);
    return FALSE;
}

/*******************************************************************
** ������:      public_para_manager_store_by_id
** ��������:    �洢PP��������ʱ2��洢��flash
** ����:        [in] id:  ������ţ���PP_ID_E
**              [in] sptr:���뻺��
**              [in] slen:���泤��
** ����:        �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN public_para_manager_store_by_id(INT8U id, INT8U *sptr, INT16U slen)
{
    HWORD_UNION chksum;
    PP_HEAD_T *phead;
    PP_CLASS_T const *pclass;
       
    if (s_dcb.lock == PP_LOCK) {
        return FALSE;
    }
    
    return_val_if_fail((id < public_para_reg_get_item_max()), RET_FAIL);
    return_val_if_fail((slen == s_dcb.pp[id].preg->rec_size), RET_FAIL);
    return_val_if_fail((id == s_dcb.pp[id].preg->id), RET_FAIL);
    
    pclass = public_para_reg_get_class_info(s_dcb.pp[id].preg->type);
    phead  = (PP_HEAD_T *)(pclass->memptr + s_dcb.pp[id].offset);
    
    return_val_if_fail((pclass->type == s_dcb.pp[id].preg->type), RET_FAIL);
    return_val_if_fail((*(pclass->memptr + s_dcb.pp[id].offset + s_dcb.pp[id].space - 1) == BOUND_FLAG), RET_FAIL);
    
    memcpy(((INT8U *)phead) + sizeof(PP_HEAD_T), sptr, slen);
    
    chksum.hword = u_chksum_2(((INT8U *)phead) + sizeof(PP_HEAD_T), slen);
    phead->chksum[0] = chksum.bytes.high;
    phead->chksum[1] = chksum.bytes.low;
    
    s_dcb.pp[id].valid   = VALID_;
    s_dcb.pp[id].attrib |= WF_;
    
    if (pclass->type != PP_TYPE_DELAY) {
        s_dcb.cls[pclass->type].ct_delay = 0;
        os_msg_post(TSK_SYS_MSG, MSG_PP_CHANGE, id, PP_REASON_STORE, NULL);
    }
    
    return TRUE;
}

/*******************************************************************
** ������:      public_para_manager_store_instant_by_id
** ��������:    �洢PP����,�����洢��flash��
** ����:        [in] id:  ������ţ���PP_ID_E
**              [in] sptr:���뻺��
**              [in] slen:���泤��
** ����:        �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN public_para_manager_store_instant_by_id(INT8U id, INT8U *sptr, INT16U slen)
{
    BOOLEAN result;
    
    result = public_para_manager_store_by_id(id, sptr, slen);
    if (result) {
        if (_flush_ram_to_file(s_dcb.pp[id].preg->type)) {
            s_dcb.cls[s_dcb.pp[id].preg->type].ct_delay = 0;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/*******************************************************************
** ������:      public_para_manager_check_valid_by_id
** ��������:    �ж�PP������Ч��
** ����:        [in] id:  ������ţ���PP_ID_E
** ����:        ��Ч����true����Ч����false
********************************************************************/
BOOLEAN public_para_manager_check_valid_by_id(INT8U id)
{
    PP_HEAD_T *phead;
    PP_CLASS_T const *pclass;
    
    return_val_if_fail((id < public_para_reg_get_item_max()), RET_FAIL);
    return_val_if_fail((id == s_dcb.pp[id].preg->id), RET_FAIL);
    
    pclass = public_para_reg_get_class_info(s_dcb.pp[id].preg->type);
    phead  = (PP_HEAD_T *)(pclass->memptr + s_dcb.pp[id].offset);
    
    return_val_if_fail((*(pclass->memptr + s_dcb.pp[id].offset + s_dcb.pp[id].space - 1) == BOUND_FLAG), RET_FAIL);
    
    if (s_dcb.pp[id].valid == VALID_) {
        if (_check_para_valid((INT8U *)phead, s_dcb.pp[id].space)) {
            return TRUE;
        } else {
            return_val_if_fail(0, RET_FAIL);
        }
    }
    if (s_dcb.pp[id].preg->i_ptr != 0) {
        return TRUE;
    }
    return FALSE;      
}

/*******************************************************************
** ������:      public_para_manager_get_by_id
** ��������:    ��ȡPP���������ж�PP��������Ч�ԣ�ֱ�ӻ�ȡ
** ����:        [in] id:  ������ţ���PP_ID_E
**              [in] dptr:�������
**              [in] rlen:���泤��
** ����:        �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN public_para_manager_get_by_id(INT8U id, INT8U *dptr, INT16U rlen)
{
    PP_HEAD_T *phead;
    PP_CLASS_T const *pclass;
    
    return_val_if_fail((id < public_para_reg_get_item_max()), RET_FAIL);
    return_val_if_fail((rlen >= s_dcb.pp[id].preg->rec_size), RET_FAIL);
    return_val_if_fail((id == s_dcb.pp[id].preg->id), RET_FAIL);
    
    pclass = public_para_reg_get_class_info(s_dcb.pp[id].preg->type);
    phead  = (PP_HEAD_T *)(pclass->memptr + s_dcb.pp[id].offset);
    
    return_val_if_fail((*(pclass->memptr + s_dcb.pp[id].offset + s_dcb.pp[id].space - 1) == BOUND_FLAG), RET_FAIL);
    
    memcpy(dptr, ((INT8U *)phead) + sizeof(PP_HEAD_T), s_dcb.pp[id].preg->rec_size);
    return TRUE;        
}

/*******************************************************************
** ������:      public_para_manager_reg_change_informer
** ��������:    ע������仯֪ͨ�����ĺ���
** ����:        [in] id:  ������ţ���PP_ID_E
**              [in] fp:  �ص�����
** ����:        �ɹ�����true��ʧ�ܷ���false
********************************************************************/
BOOLEAN public_para_manager_reg_change_informer(INT8U id, void (*fp)(INT8U reason))
{
    INT8U i;
    
    return_val_if_fail((id < public_para_reg_get_item_max()), RET_FAIL);
    return_val_if_fail((id == s_dcb.pp[id].preg->id), RET_FAIL);
       
    for (i = 0; i < MAX_PP_INFORM; i++) {
        if (s_dcb.inform[i].informer == 0) {
            break;
        }
    }
    return_val_if_fail((i < MAX_PP_INFORM), RET_FAIL);
    
    s_dcb.inform[i].id       = id;
    s_dcb.inform[i].informer = fp;
    
    return TRUE;
}

/*******************************************************************
** ������:      public_para_manager_inform_change_by_id
** ��������:    �����仯֪ͨ����
** ����:        [in] id:      ������ţ���PP_ID_E
**              [in] reason:  �仯ԭ��
** ����:        ��
********************************************************************/
void public_para_manager_inform_change_by_id(INT16U id, INT8U reason)
{
    INT8U i, j, cls, maxid;//, offset;
    PP_REG_T const *preg;
    PP_CLASS_T const *pclass;
    
    maxid = public_para_reg_get_item_max();
    if (id > 0xff) {
        cls = id & 0xff;
        return_if_fail((cls < public_para_reg_get_class_max()));
        
        pclass = s_dcb.cls[cls].pclass;
        preg   = pclass->preg;
        for (j = 0; j < pclass->nreg; j++) {
            id = preg->id;
            for (i = 0; i < MAX_PP_INFORM; i++) {
                if (s_dcb.inform[i].informer != 0) {
                    return_if_fail((s_dcb.inform[i].id < maxid));
                    if (id == s_dcb.inform[i].id) {
                         (*s_dcb.inform[i].informer)(reason);
                    }
                }
            }
            //offset = id - s_dcb.cls[cls].pclass->preg->id;
            preg++;
            
            //SYS_DEBUG("<public_para_manager_inform_change_by_id,cls(%d),id(%d),offset id:(%d),reason:(%d)>\r\n", cls, id, offset, reason);
        }
    } else {
        return_if_fail((id < maxid));
        return_if_fail((id == s_dcb.pp[id].preg->id));
        cls = s_dcb.pp[id].preg->type;
        
        for (i = 0; i < MAX_PP_INFORM; i++) {
            if (s_dcb.inform[i].informer != 0) {
                return_if_fail((s_dcb.inform[i].id < maxid));
                if (id == s_dcb.inform[i].id) {
                    (*s_dcb.inform[i].informer)(reason);
                }
            }
        }
        //offset = id - s_dcb.cls[cls].pclass->preg->id;
        
        //SYS_DEBUG("<public_para_manager_inform_change_by_id,cls(%d),id(%d),offset id:(%d),reason:(%d)>\r\n", cls, id, offset, reason);
    }
}

/*******************************************************************
** ������:     public_para_manager_reset_inform
** ��������:   ��λ�ص��ӿ�
** ����:       [in] resettype����λ����
**             [in] filename�� �ļ���
**             [in] line��     �����к�
**             [in] errid��    ����ID
** ����:       ��
********************************************************************/
void public_para_manager_reset_inform(INT8U resettype, INT8U *filename, INT32U line, INT32U errid)
{
    INT8U i, nclass;
    
    resettype = resettype;
    filename = filename;
    line = line;
    errid = errid;
    
    if (s_dcb.lock == PP_LOCK) {
        return;
    }
        
    nclass = public_para_reg_get_class_max();
    return_if_fail((nclass <= MAX_PP_CLASS_NUM));
    for (i = 0; i < nclass; i++) {
        if (!_flush_ram_to_file(i)) {
            return;
        }
    }
}

/*******************************************************************
** ������:     OS_MsgReset
** ��������:   PP�����仯��Ϣ����
** ����:       [in]  lpara:
** ����:       ��
********************************************************************/
void OS_MsgPPChange(INT32U lpara, INT32U hpara, void *p)
{
    public_para_manager_inform_change_by_id(lpara, hpara);
}

