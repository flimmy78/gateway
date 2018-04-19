/********************************************************************************
**
** �ļ���:     public_para_reg.h
** ��Ȩ����:   (c) 2013-2015 ����������ͨ�������Ƽ����޹�˾
** �ļ�����:   ʵ�ֹ��������ļ��洢����ע����Ϣ�����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/26 | ���ѽ� |  �������ļ�
********************************************************************************/
#ifndef PUBLIC_PARA_REG_H
#define PUBLIC_PARA_REG_H

#include "public_para_typedef.h"

/*
********************************************************************************
* ����ģ�����ݽṹ
********************************************************************************
*/
typedef enum {
    PP_BK,    
    PP_NBK,
    PP_BK_MAX
} PP_BK_E;

/* ����������Ϣ�ṹ�� */
typedef struct {
    INT8U        type;                    /* ���������ļ����� */
    INT8U        id;                      /* ��������ͳһ��� */
    INT16U       rec_size;                /* �������� */
    INT8U        bk;                      /* �������� */
    void const  *i_ptr;                   /* Ĭ�ϲ���ָ�� */
} PP_REG_T;

/* ����������ṹ�� */
typedef struct {
    INT8U           type;                 /* ���������ļ����� */
    INT16U          dly;                  /* ��ʱ�洢���ļ�ʱ�䣬��λ���� */
    char const     *filename;             /* ���������ļ��ļ���,��'\0'Ϊ������ */
    INT32U          memlen;               /* ӳ���ڴ��С */
    INT8U          *memptr;               /* ӳ���ڴ� */
    PP_REG_T const *preg;                 /* ��������������ע����Ϣ�� */
    INT8U           nreg;                 /* ��������������Ĳ����ĸ��� */
} PP_CLASS_T;

typedef struct {
    INT8U chksum[2];
} PP_HEAD_T;

/*
********************************************************************************
* �����������ݿ��ļ���ͳһ���
********************************************************************************
*/
#ifdef BEGIN_PP_DEF
#undef BEGIN_PP_DEF
#endif

#ifdef PP_DEF
#undef PP_DEF
#endif

#ifdef END_PP_DEF
#undef END_PP_DEF
#endif

#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)  \
               _PP_TYPE_##_ENUM,
               
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_)
#define END_PP_DEF(_PP_TYPE_)

typedef enum {
    #include "public_para_reg.def"
    PP_CLASS_ID_MAX
} PP_CLASS_ID_E;

/* ������ͳһ��� */
#ifdef BEGIN_PP_DEF
#undef BEGIN_PP_DEF
#endif

#ifdef PP_DEF
#undef PP_DEF
#endif

#ifdef END_PP_DEF
#undef END_PP_DEF
#endif

#ifdef GLOBALS_PP_REG
#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)  \
               INT8U const _PP_TYPE_ = _PP_TYPE_##_ENUM;
               
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_)
#define END_PP_DEF(_PP_TYPE_)

#include "public_para_reg.def"
#else
#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)  \
               extern INT8U const _PP_TYPE_;
               
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_)
#define END_PP_DEF(_PP_TYPE_)

#include "public_para_reg.def"
#endif

/*
********************************************************************************
* �����������ݿ��ļ�ͳһ���
********************************************************************************
*/
#ifdef BEGIN_PP_DEF
#undef BEGIN_PP_DEF
#endif

#ifdef PP_DEF
#undef PP_DEF
#endif

#ifdef END_PP_DEF
#undef END_PP_DEF
#endif

#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_) \
               _PP_ID_##_ENUM,
                  
#define END_PP_DEF(_PP_TYPE_)

typedef enum {
    #include "public_para_reg.def"
    PP_ID_MAX
} PP_ID_E;

/* ����ͳһ��� */
#ifdef BEGIN_PP_DEF
#undef BEGIN_PP_DEF
#endif

#ifdef PP_DEF
#undef PP_DEF
#endif

#ifdef END_PP_DEF
#undef END_PP_DEF
#endif

#ifdef GLOBALS_PP_REG
#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_) \
               INT8U const _PP_ID_ = _PP_ID_##_ENUM;
                  
#define END_PP_DEF(_PP_TYPE_)

#include "public_para_reg.def"
#else

#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_) \
               extern INT8U const _PP_ID_;
                  
#define END_PP_DEF(_PP_TYPE_)

#include "public_para_reg.def"
#endif

/*******************************************************************
** ������:     public_para_reg_get_class_info
** ��������:   ��ȡ��Ӧ�����������ע����Ϣ
** ����:       [in] nclass:ͳһ��ŵ����ţ���PP_CLASS_ID_E
** ����:       �ɹ�����ע���ָ�룬ʧ�ܷ���0
********************************************************************/
PP_CLASS_T const *public_para_reg_get_class_info(INT8U nclass);

/*******************************************************************
*   ������:    public_para_reg_get_class_max
*   ��������:  ��ȡ��ע��Ĺ������������
*�� ����:      ��
*   ����ֵ:    �����
*******************************************************************/ 
INT8U public_para_reg_get_class_max(void);

/*******************************************************************
** ������:     public_para_reg_get_item_info
** ��������:   ��ȡ��Ӧ����������ע����Ϣ
** ����:       [in] id:ͳһ��ŵĲ�����ţ���PP_ID_E
** ����:       �ɹ�����ע���ָ�룬ʧ�ܷ���0
********************************************************************/
PP_REG_T const *public_para_reg_get_item_info(INT8U id);

/*******************************************************************
*   ������:    public_para_reg_get_item_max
*   ��������:  ��ȡ��ע��Ĺ�����������
*�� ����:      ��
*   ����ֵ:    ����
*******************************************************************/
INT8U public_para_reg_get_item_max(void);

#endif



