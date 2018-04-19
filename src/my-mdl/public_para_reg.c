/********************************************************************************
**
** �ļ���:     public_para_reg.c
** ��Ȩ����:   (c) 2013-2015 
** �ļ�����:   ʵ�ֹ��������ļ��洢����ע����Ϣ�����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/08/26 | ���ѽ� |  �������ļ�
********************************************************************************/
#define  GLOBALS_PP_REG   1

#include "sys_swconfig.h"
#include "sys_typedef.h"
#include "public_para_reg.h"


/*
****************************************************************
*   ����ӳ���ڴ�
****************************************************************
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
        INT8U _PP_TYPE_##_MEM_BUF[0

#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_) \
        + _LEN_ + sizeof(PP_HEAD_T) + 1
#define END_PP_DEF(_PP_TYPE_) \
        ];

#include "public_para_reg.def"

/*
****************************************************************
*   ���幫�������ļ�ע���
****************************************************************
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
               {_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_},

#define END_PP_DEF(_PP_TYPE_)

static const PP_REG_T s_pp_regtbl[] = {
    #include "public_para_reg.def"
    {0}
};

/*
****************************************************************
*   �������ע���
****************************************************************
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

#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_) \
          _PP_TYPE_##BEGIN,

#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_)       \
          _PP_ID_##DEF,

#define END_PP_DEF(_PP_TYPE_) \
          _PP_TYPE_##END,

typedef enum {
    #include "public_para_reg.def"
    PP_DEF_MAX
} PP_DEF_E;

#ifdef BEGIN_PP_DEF
#undef BEGIN_PP_DEF
#endif

#ifdef PP_DEF
#undef PP_DEF
#endif

#ifdef END_PP_DEF
#undef END_PP_DEF
#endif

#define BEGIN_PP_DEF(_PP_TYPE_, _DLY_, _DESC_)                                                            \
                   {_PP_TYPE_, _DLY_, _DESC_, sizeof(_PP_TYPE_##_MEM_BUF),  (INT8U *)_PP_TYPE_##_MEM_BUF, \
                   (PP_REG_T const *)&s_pp_regtbl[_PP_TYPE_##BEGIN - 2 * _PP_TYPE_],                      \
                   _PP_TYPE_##END - _PP_TYPE_##BEGIN - 1                                                  \
                   },
                   
#define PP_DEF(_PP_TYPE_, _PP_ID_, _LEN_, _BAK_, _I_PTR_)
#define END_PP_DEF(_PP_TYPE_)

static const PP_CLASS_T s_class_tbl[] = {
    #include "public_para_reg.def"
    {0}
};

/*******************************************************************
** ������:     public_para_reg_get_class_info
** ��������:   ��ȡ��Ӧ�����������ע����Ϣ
** ����:       [in] nclass:ͳһ��ŵ����ţ���PP_CLASS_ID_E
** ����:       �ɹ�����ע���ָ�룬ʧ�ܷ���0
********************************************************************/
PP_CLASS_T const *public_para_reg_get_class_info(INT8U nclass)
{
    if (nclass >= PP_CLASS_ID_MAX) {
        return 0;
    }

    return (PP_CLASS_T const *)(&s_class_tbl[nclass]);
}

/*******************************************************************
*   ������:    public_para_reg_get_class_max
*   ��������:  ��ȡ��ע��Ĺ������������
*�� ����:      ��
*   ����ֵ:    �����
*******************************************************************/ 
INT8U public_para_reg_get_class_max(void)
{
    return PP_CLASS_ID_MAX;
}

/*******************************************************************
** ������:     public_para_reg_get_item_info
** ��������:   ��ȡ��Ӧ����������ע����Ϣ
** ����:       [in] id:ͳһ��ŵĲ�����ţ���PP_ID_E
** ����:       �ɹ�����ע���ָ�룬ʧ�ܷ���0
********************************************************************/
PP_REG_T const *public_para_reg_get_item_info(INT8U id)
{
    if (id >= PP_ID_MAX) {
        return 0;
    }
    return (PP_REG_T const *)&s_pp_regtbl[id];
}

/*******************************************************************
*   ������:    public_para_reg_get_item_max
*   ��������:  ��ȡ��ע��Ĺ�����������
*�� ����:      ��
*   ����ֵ:    ����
*******************************************************************/
INT8U public_para_reg_get_item_max(void)
{
    return PP_ID_MAX;
}

