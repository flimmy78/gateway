/********************************************************************************
**
** �ļ���:     os_msg.c
** ��Ȩ����:   
** �ļ�����:   ��Ϣ����ģ��
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����       | ����   |  �޸ļ�¼
**===============================================================================
**| 2012/08/14 | ���ѽ� |  �������ļ�
*********************************************************************************/
#include "sys_includes.h"

/*
********************************************************************************
* ���ݽṹ����
********************************************************************************
*/
/* ������Ϣ����,���ڴ��ע�����Ϣ */
typedef struct {
    INT16U  tskid;           /* ͳһ�������� */
    INT16U  msgid;           /* ͳһ����Ϣ��� */
    void    (*proc)(INT32U lpara, INT32U hpara, void* p); /* ��Ϣ������ */
} OS_MSG_TBL_T;

/* �������������ڴ��ע������� */
typedef struct {
    INT16U         tskid;    /* ͳһ�������� */
    OS_MSG_TBL_T  *pmsg;     /* �����������Ϣע��� */
    INT32U         nmsg;     /* ���������ע����Ϣ�ĸ��� */
} OS_TSK_TBL_T;

typedef struct {
    INT8U          tskid;
    INT32U         msgid;
    INT32U         lpara;
    INT32U         hpara;
    void*          p;
} OS_MSG_T;

typedef struct {
    INT8U          flag;
    INT16U         msgpos;
    INT16U         nummsg;
    INT16U   const msgmax;
    OS_MSG_T*const pmsgq;
} MSGCB_T;

/*
****************************************************************
*   ��������������Ϣ��ע���
****************************************************************
*/
#ifdef BEGIN_MSG_DEF
#undef BEGIN_MSG_DEF
#endif

#ifdef MSG_DEF
#undef MSG_DEF
#endif

#ifdef END_MSG_DEF
#undef END_MSG_DEF
#endif

#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE)          \
                   static const OS_MSG_TBL_T _TSK_ID_##MSG_TBL[] = {
                    
#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_)             \
                   {_TSK_ID_, _MSG_ID_, _PROC_},
                 
#define END_MSG_DEF(_TSK_ID_)                           \
                   {0}};

#include "os_msg_reg.def"

/*
****************************************************************
*   ���������ע���
****************************************************************
*/
#ifdef BEGIN_MSG_DEF
#undef BEGIN_MSG_DEF
#endif

#ifdef MSG_DEF
#undef MSG_DEF
#endif

#ifdef END_MSG_DEF
#undef END_MSG_DEF
#endif

#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE)                              \
                   {_TSK_ID_,                                               \
                   (OS_MSG_TBL_T *)_TSK_ID_##MSG_TBL,                       \
                   sizeof(_TSK_ID_##MSG_TBL) / sizeof(OS_MSG_TBL_T) - 1     \
                   },
                   
#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_)
#define END_MSG_DEF(_TSK_ID_)

static const OS_TSK_TBL_T s_tsk_tbl[] = {
    #include "os_msg_reg.def"
    {0}
};

/*
****************************************************************
*   
****************************************************************
*/

#ifdef BEGIN_MSG_DEF
#undef BEGIN_MSG_DEF
#endif

#ifdef MSG_DEF
#undef MSG_DEF
#endif

#ifdef END_MSG_DEF
#undef END_MSG_DEF
#endif

#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE)  \
                   static OS_MSG_T _TSK_ID_##MSG_QUEUE[MAX_MSG_QUEUE];
                    
#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_)
                 
#define END_MSG_DEF(_TSK_ID_)

#include "os_msg_reg.def"

/*
********************************************************************************
* ����ģ��ֲ�����
********************************************************************************
*/
#ifdef BEGIN_MSG_DEF
#undef BEGIN_MSG_DEF
#endif

#ifdef MSG_DEF
#undef MSG_DEF
#endif

#ifdef END_MSG_DEF
#undef END_MSG_DEF
#endif

#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE)  \
                   {0,                          \
                    0,                          \
                    0,                          \
                    MAX_MSG_QUEUE,              \
                    (OS_MSG_T*)_TSK_ID_##MSG_QUEUE \
                   },
#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_)
                 
#define END_MSG_DEF(_TSK_ID_)

static MSGCB_T s_msgcb_tab[] = {
    #include "os_msg_reg.def"
    {0}
};

/*******************************************************************
** ������:     _get_reg_msg_info
** ��������:   ��ȡ��Ӧ�����ע����Ϣ
** ����:       [in] tskid:ͳһ��ŵ������
** ����:       �ɹ�����ע���ָ�룬ʧ�ܷ���0
********************************************************************/
static OS_TSK_TBL_T* _get_reg_msg_info(INT16U tskid)
{
    if (tskid >= TSK_ID_MAX) {
        return 0;
    }

    return (OS_TSK_TBL_T *)(&s_tsk_tbl[tskid]);
}

/*******************************************************************
** ������:     os_msg_post
** ��������:   ����ϵͳ��Ϣ
** ����:       [in]  msgid:             ϵͳ��ϢID
**             [in]  lpara:          ��32λ��Ϣ����
**             [in]  hpara:          ��32λ��Ϣ����
** ����:       ��
********************************************************************/
BOOLEAN os_msg_post(INT8U tskid, INT32U msgid, INT32U lpara, INT32U hpara, void*p)
{
    INT16U pos;
    MSGCB_T *ptr;
    OS_MSG_T *pmsg;
    return_val_if_fail((tskid < TSK_ID_MAX), FALSE);
    
    ptr = &s_msgcb_tab[tskid];
    if (ptr->nummsg >= ptr->msgmax) {
        return false;
    }

    pos = ptr->msgpos + ptr->nummsg;
    if (pos >= ptr->msgmax) {
        pos -= ptr->msgmax;
    }
    pmsg = ptr->pmsgq;
    pmsg[pos].tskid = tskid;
    pmsg[pos].msgid = msgid;
    pmsg[pos].lpara = lpara;
    pmsg[pos].hpara = hpara;
    pmsg[pos].p     = p;
    if (ptr->flag == 0) {
        ptr->flag = 1;
        //PORT_PostCommonMsg();
    }
    ptr->nummsg++;
    return true;
}

/*******************************************************************
** ������:     MsgIsExist
** ��������:   ��������Ϣ�������Ƿ����ָ������ϢID
** ����:       [in]  msgid:             �����Ե�ϵͳ��ϢID
** ����:       true:  ����Ϣ�����д���
**             false: ������Ϣ�����д���
********************************************************************/
/*
static BOOLEAN MsgIsExist(INT8U tskid, INT32U msgid)
{
    INT16U pos, i;

    pos = s_mcb.msgpos;
    for (i = 0; i < s_mcb.nummsg; i++, pos++) {
        if (pos >= OS_MAX_MSG) {
            pos -= OS_MAX_MSG;
        }
        if ((s_mcb.msgq[pos].msgid == msgid) && (s_mcb.msgq[pos].tskid == tskid)) {
            return true;
        }
    }
    return false;
}
*/
/*******************************************************************
** ������:     OS_APostMsg
** ��������:   ����ϵͳ��Ϣ
** ����:       [in]  overlay:           true:  ������Ϣ�����д�����ͬ����ϢID, �򲻽��µ���Ϣ�������Ϣ������
**                                      false: �µ���Ϣ����������Ϣ������
**             [in]  tskid:            ��Ϣ��������ID
**             [in]  msgid:             ϵͳ��ϢID
**             [in]  lpara:          ��32λ��Ϣ����
**             [in]  hpara:          ��32λ��Ϣ����
** ����:       ��
********************************************************************/
/*
void OS_APostMsg(BOOLEAN overlay, INT8U tskid, INT32U msgid, INT32U lpara, INT32U hpara)
{
    if (overlay) {
        if (!MsgIsExist(tskid, msgid)) {
            os_msg_post(tskid, msgid, lpara, hpara);
        }
    } else {
        os_msg_post(tskid, msgid, lpara, hpara);
    }
}
*/
/*******************************************************************
** ������:     os_msg_sched
** ��������:   ����ϵͳ��Ϣ
** ����:       ��
** ����:       ��
********************************************************************/
void os_msg_sched(void)
{
    INT8U tskid;
    INT32U msgid, lpara, hpara, num;//, nmsg;
    OS_TSK_TBL_T *ptsk;
    OS_MSG_TBL_T *pmsg;
    MSGCB_T *ptr;
    OS_MSG_T *p;
    void* msg;

    for (tskid = 0; tskid < TSK_ID_MAX; tskid++) {
        ptr = &s_msgcb_tab[tskid];
        if (ptr->flag == 0) {
            continue;
        }
        
        p = ptr->pmsgq;
        for (;;) {
            num  = ptr->nummsg;
            if (num == 0) {
                ptr->flag = 0;
                return;
            }
            //tskid = p[ptr->msgpos].tskid;
            msgid = p[ptr->msgpos].msgid;
            lpara = p[ptr->msgpos].lpara;
            hpara = p[ptr->msgpos].hpara;
            msg   = p[ptr->msgpos].p;
            if (++ptr->msgpos >= ptr->msgmax) {
                ptr->msgpos = 0;
            }
            ptr->nummsg--;

            if (tskid < TSK_ID_MAX) {
                ptsk = _get_reg_msg_info(tskid);
                pmsg = ptsk->pmsg;
                //nmsg = ptsk->nmsg;

                if (pmsg[msgid].proc != 0) {
                    pmsg[msgid].proc(lpara, hpara, msg);
                }
            }
        }
    }
}

/*******************************************************************
** ������:     OS_MsgNULL
** ��������:   ����Ϣ�������������滻��δʵ�ֵ���Ϣ������
** ����:       [in]  taskid:   ����id��
** ����:       ��
********************************************************************/
void OS_MsgNULL(INT32U lpara, INT32U hpara, void *p)
{
}

