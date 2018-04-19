/********************************************************************************
**
** �ļ���:     dlist.c
** ��Ȩ����:   (c) 2007-2008 ������Ѹ����ɷ����޹�˾
** �ļ�����:   ʵ���������ݽṹ
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����       | ����   |  �޸ļ�¼
**===============================================================================
**| 2007/04/17 | �´ӻ� |  �������ļ�
**| 2008/10/13 | �θ�   |  ��ֲ��̨���򵽸��ļ�,�ӿ�ʹ��ԭ��̨�ӿ�
*********************************************************************************/
#include "sys_typedef.h"
#include "dlist.h"
/*******************************************************************
** ������:     dlist_check
** ��������:   ����������Ч��
** ����:       [in]  Lp:����
** ����:       true:    ������Ч
**             false:   ������Ч
********************************************************************/
BOOLEAN dlist_check(LIST_T *Lp, void *bptr, void *eptr, BOOLEAN needchecklp)
{
    INT32U    count;
    LISTNODE *curnode;
	
    if (Lp == 0) return FALSE;
    if (needchecklp == TRUE) {
        if (((void *)Lp < bptr) || ((void *)Lp > eptr)) return FALSE;//art090209
    }
    
    count = 0;
    curnode = Lp->Head;
    while(curnode != 0) {
        if (((void *)curnode < bptr) || ((void *)curnode > eptr)) return FALSE;//art090209
        if (++count > Lp->Item) return FALSE;
        curnode = curnode->next;
    }
    if (count != Lp->Item) return FALSE;
	
    count = 0;
    curnode = Lp->Tail;
    while(curnode != 0) {
        if (((void *)curnode < bptr) || ((void *)curnode > eptr)) return FALSE;//art090209
        if (++count > Lp->Item) return FALSE;
        curnode = curnode->prv;
    }
    if (count != Lp->Item) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*******************************************************************
** ������:     dlist_init
** ��������:   ��ʼ������
** ����:       [in]  Lp:����
** ����:       true:    �ɹ�
**             false:   ʧ��
********************************************************************/
BOOLEAN dlist_init(LIST_T *Lp)
{
    if (Lp == 0) return FALSE;
	
    Lp->Head = 0;
    Lp->Tail = 0;
    Lp->Item = 0;
    return TRUE;
}

/*******************************************************************
** ������:     dlist_item
** ��������:   ��ȡ����ڵ����
** ����:       [in]  Lp:        ����
** ����:       ����ڵ����
********************************************************************/
INT32U dlist_item(LIST_T *Lp)
{
    if (Lp == 0) {
        return 0;
    } else {
        return (Lp->Item);
    }
}

/*******************************************************************
** ������:      dlist_is_exist
** ��������:    �ж������Ƿ���ڸýڵ�
** ����:        [in]  Lp:        ����
**              [in]  Bp:        ָ���ڵ�
** ����:        true or false
********************************************************************/
BOOLEAN dlist_is_exist(LIST_T *Lp, LISTMEM *Bp)
{
    INT16U i;
    LISTNODE *nextnode, *curnode;

    if (Lp == 0 || Bp == 0) return FALSE;

    nextnode = Lp->Head;
    curnode  = (LISTNODE *)(Bp - sizeof(NODE));
    
    for (i = 0; i < Lp->Item; i++) {
        if (nextnode == curnode) {
            return TRUE;
        }
        nextnode = nextnode->next;
    }

    return FALSE;
}

/*******************************************************************
** ������:     dlist_get_head
** ��������:   ��ȡ����ͷ�ڵ�
** ����:       [in]  Lp: ����
** ����:       ����ͷ�ڵ�; �������޽ڵ�, �򷵻�0
********************************************************************/
LISTMEM *dlist_get_head(LIST_T *Lp)
{
    if (Lp == 0 || Lp->Item == 0) {
        return 0;
    } else {
        return ((LISTMEM *)Lp->Head + sizeof(NODE));
    }
}

/*******************************************************************
** ������:     dlist_get_tail
** ��������:   ��ȡ����β�ڵ�
** ����:       [in]  Lp:        ����
** ����:       ����β�ڵ�; �������޽ڵ�, �򷵻�0
********************************************************************/
LISTMEM *dlist_get_tail(LIST_T *Lp)
{
    if (Lp == 0 || Lp->Item == 0) {
        return 0;
    } else {
        return ((LISTMEM *)Lp->Tail + sizeof(NODE));
    }
}

/*******************************************************************
** ������:     dlist_next_ele
** ��������:   ��ȡָ���ڵ�ĺ�һ�ڵ�
** ����:       [in]  Bp: ����ǰ�ڵ�
** ����:       Bp����һ���ڵ�; �緵��0, ���ʾ�ڵ㲻����
********************************************************************/
LISTMEM *dlist_next_ele(LISTMEM *Bp)
{
    LISTNODE *curnode;
	
    if (Bp == 0) return 0;
    curnode = (LISTNODE *)(Bp - sizeof(NODE));
    if ((curnode = curnode->next) == 0) {
        return 0;
    } else {
        return ((LISTMEM *)curnode + sizeof(NODE));
    }
}

/*******************************************************************
** ������:     dlist_prv_ele
** ��������:   ��ȡָ���ڵ��ǰһ�ڵ�
** ����:       [in]  Bp: ָ���ڵ�
** ����:       ����ָ���ڵ�Bp��ǰһ�ڵ�; �緵��0, ���ʾ������ǰһ�ڵ�
********************************************************************/
LISTMEM *dlist_prv_ele(LISTMEM *Bp)
{
    LISTNODE *curnode;

    if (Bp == 0) return 0;
    curnode = (LISTNODE *)(Bp - sizeof(NODE));
    if ((curnode = curnode->prv) == 0) {
        return 0;
    } else {
        return ((LISTMEM *)curnode + sizeof(NODE));
    }
}

/*******************************************************************
** ������:     dlist_del_ele
** ��������:   ɾ��ָ���ڵ�
** ����:       [in]  Lp:        ����
**             [in]  Bp:        ָ���ڵ�
** ����:       ����ָ���ڵ�Bp���¸��ڵ�; �緵��0, ���ʾBp��������һ�ڵ�
********************************************************************/
LISTMEM *dlist_del_ele(LIST_T *Lp, LISTMEM *Bp)
{
    LISTNODE *curnode, *prvnode, *nextnode;

    if (Lp == 0 || Bp == 0) return 0;
    if (Lp->Item == 0) return 0;

    Lp->Item--;
    curnode  = (LISTNODE *)(Bp - sizeof(NODE));
    prvnode  = curnode->prv;
    nextnode = curnode->next;
    if (prvnode == 0) {
        Lp->Head = nextnode;
    } else {
        prvnode->next = nextnode;
    }
    if (nextnode == 0) {
        Lp->Tail = prvnode;
        return 0;
    } else {
        nextnode->prv = prvnode;
        return ((LISTMEM *)nextnode + sizeof(NODE));
    }
}

/*******************************************************************
** ������:     dlist_del_head
** ��������:   ɾ������ͷ�ڵ�
** ����:       [in]  Lp:        ����
** ����:       ����ͷ�ڵ�; �緵��0, ���ʾ����������ͷ�ڵ�
********************************************************************/
LISTMEM *dlist_del_head(LIST_T *Lp)
{
    LISTMEM *Bp;

    if (Lp == 0 || Lp->Item == 0) return 0;

    Bp = (LISTMEM *)Lp->Head + sizeof(NODE);
    dlist_del_ele(Lp, Bp);
    return Bp;
}

/*******************************************************************
** ������:     dlist_del_tail
** ��������:   ɾ������β�ڵ�
** ����:       [in]  Lp:        ����
** ����:       ����β�ڵ�; �緵��0, ���ʾ����������β�ڵ�
********************************************************************/
LISTMEM *dlist_del_tail(LIST_T *Lp)
{
    LISTMEM *Bp;

    if (Lp == 0 || Lp->Item == 0) return 0;

    Bp = (LISTMEM *)Lp->Tail + sizeof(NODE);
    dlist_del_ele(Lp, Bp);
    return Bp;
}

/*******************************************************************
** ������:     dlist_append_ele
** ��������:   ������β��׷��һ���ڵ�
** ����:       [in]  Lp:        ����
**             [in]  Bp:        ��׷�ӽڵ�
** ����:       ׷�ӳɹ���ʧ��
********************************************************************/
BOOLEAN dlist_append_ele(LIST_T *Lp, LISTMEM *Bp)
{
    LISTNODE *curnode;

    if (Lp == 0 || Bp == 0) return FALSE;

    curnode = (LISTNODE *)(Bp - sizeof(NODE));
    curnode->prv = Lp->Tail;
    if (Lp->Item == 0) {
        Lp->Head = curnode;
    } else {
        Lp->Tail->next = curnode;
    }
    curnode->next = 0;
    Lp->Tail = curnode;
    Lp->Item++;
    return TRUE;
}

/*******************************************************************
** ������:     dlist_insert_head
** ��������:   ������ͷ����һ���ڵ�
** ����:       [in]  Lp:        ����
**             [in]  Bp:        ������Ľڵ�
** ����:       ����ɹ���ʧ��
********************************************************************/
BOOLEAN dlist_insert_head(LIST_T *Lp, LISTMEM *Bp)
{
    LISTNODE *curnode;

    if (Lp == 0 || Bp == 0) return FALSE;

    curnode = (LISTNODE *)(Bp - sizeof(NODE));
    curnode->next = Lp->Head;
    if (Lp->Item == 0) {
        Lp->Tail = curnode;
    } else {
        Lp->Head->prv = curnode;
    }
    curnode->prv = 0;
    Lp->Head = curnode;
    Lp->Item++;
    return TRUE;
}

/*******************************************************************
** ������:     dlist_connect_head_tail
** ��������:   ��list��β����,�γɻ���
** ����:       [in]  Lp:        ����
** ����:       ���ӳɹ���ʧ��
********************************************************************/
BOOLEAN dlist_connect_head_tail(LIST_T *Lp)
{
    if (Lp == 0) return FALSE;
    Lp->Head->prv  = Lp->Tail;
    Lp->Tail->next = Lp->Head;
    return true;
}

// ���뵽CurBp��ǰ��,�� InsBp->CurBp
/*******************************************************************
** ������:     dlist_insert_prv_ele
** ��������:   ��ָ���ڵ�ǰ����һ���½ڵ�
** ����:       [in]  Lp:        ����
**             [in]  CurBp:     ָ���ڵ�
**             [in]  InsBp:     ������ڵ�
** ����:       ����ɹ���ʧ��
********************************************************************/
BOOLEAN dlist_insert_prv_ele(LIST_T *Lp, LISTMEM *CurBp, LISTMEM *InsBp)
{
    LISTNODE *curnode, *insnode;
	
    if (Lp == 0 || CurBp == 0 || InsBp == 0) return FALSE;
    if (Lp->Item == 0) return FALSE;

    curnode  = (LISTNODE *)(CurBp - sizeof(NODE));
    insnode  = (LISTNODE *)(InsBp - sizeof(NODE));

    insnode->next = curnode;
    insnode->prv  = curnode->prv;
    if (curnode->prv == 0){
        Lp->Head = insnode;
    } else {
        curnode->prv->next = insnode;
    }
    curnode->prv = insnode;
    Lp->Item++;
    return TRUE;
}

/*******************************************************************
** ������:     dlist_insert_next_ele
** ��������:   ��ָ���ڵ�����һ���½ڵ�
** ����:       [in]  Lp:        ����
**             [in]  CurBp:     ָ���ڵ�
**             [in]  InsBp:     ������ڵ�
** ����:       ����ɹ���ʧ��
********************************************************************/
BOOLEAN dlist_insert_next_ele(LIST_T *Lp, LISTMEM *CurBp, LISTMEM *InsBp)
{
    LISTNODE *curnode, *insnode;

    if (Lp == 0 || CurBp == 0 || InsBp == 0) return FALSE;
    if (Lp->Item == 0) return FALSE;
    
    curnode = (LISTNODE *)(CurBp - sizeof(NODE));
    insnode = (LISTNODE *)(InsBp - sizeof(NODE));
    
    insnode->next = curnode->next;
    insnode->prv  = curnode;
    if(curnode->next == 0) {
        Lp->Tail = insnode;
    } else {
        curnode->next->prv = insnode;
    }
    curnode->next = insnode;
    Lp->Item++;
    return TRUE;
}

/*******************************************************************
** ������:     dlist_mem_init
** ��������:   ��һ���ڴ��ʼ������������
** ����:       [in]  memLp:     ����
**             [in]  addr:      �ڴ���ʼ��ַ
**             [in]  nblks:     �ڴ�����
**             [in]  blksize:   �ڴ���С
** ����:       �ɹ���ʧ��
********************************************************************/
BOOLEAN dlist_mem_init(LIST_T *memLp, LISTMEM *addr, INT32U nblks, INT32U blksize)
{
    if (!dlist_init(memLp)) return FALSE;

    addr += sizeof(NODE);
    for(; nblks > 0; nblks--){
        if (!dlist_append_ele(memLp, addr)) return FALSE;
        addr += blksize;
    }
    return TRUE;
}
