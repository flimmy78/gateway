/********************************************************************************
**
** �ļ���:     os_msg.h
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
#ifndef H_OS_MSG
#define H_OS_MSG

/*
********************************************************************************
* ���ݽṹ����
********************************************************************************
*/


/*
****************************************************************
*   ���������������
****************************************************************
*/
/* ��Ϣ���������� */
#ifdef BEGIN_MSG_DEF
#undef BEGIN_MSG_DEF
#endif

#ifdef MSG_DEF
#undef MSG_DEF
#endif

#ifdef END_MSG_DEF
#undef END_MSG_DEF
#endif

#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE)

#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_)                    \
                   void _PROC_(INT32U lpara, INT32U hpara, void* p);
                   
#define END_MSG_DEF(_TSK_ID_)

#include "os_msg_reg.def"

/*
********************************************************************************
* ����ͳһ��ŵ�����ID
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


#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE)    \
                   _TSK_ID_,
                    

#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_)
#define END_MSG_DEF(_TSK_ID_)

typedef enum {
    #include "os_msg_reg.def"
    TSK_ID_MAX
} OS_TSK_ID_E;

/*
********************************************************************************
* ����ͳһ��ŵ���ϢID
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


#define BEGIN_MSG_DEF(_TSK_ID_, MAX_MSG_QUEUE) \
        typedef enum {

#define MSG_DEF(_TSK_ID_, _MSG_ID_, _PROC_) \
                    _MSG_ID_,
                    
#define END_MSG_DEF(_TSK_ID_)   \
          _TSK_ID_##_MSG_ID_MAX \
        } _TSK_ID_##_MSG_ID_E;

#include "os_msg_reg.def"


/*******************************************************************
** ������:     os_msg_post
** ��������:   ����ϵͳ��Ϣ
** ����:       [in]  msgid:             ϵͳ��ϢID
**             [in]  lpara:          ��32λ��Ϣ����
**             [in]  hpara:          ��32λ��Ϣ����
** ����:       ��
********************************************************************/
BOOLEAN os_msg_post(INT8U tskid, INT32U msgid, INT32U lpara, INT32U hpara, void*p);

/*******************************************************************
** ������:     os_msg_sched
** ��������:   ����ϵͳ��Ϣ
** ����:       ��
** ����:       ��
********************************************************************/
void os_msg_sched(void);

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
//void OS_APostMsg(BOOLEAN overlay, INT8U tskid, INT32U msgid, INT32U lpara, INT32U hpara);




#endif          /* end of H_GPS_MSGMAN */


