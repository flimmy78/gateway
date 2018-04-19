/********************************************************************************
**
** �ļ���:     net_link_node.c
** ��Ȩ����:   (c) 2013-2014 
** �ļ�����:   ����ڵ����
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2014/9/16 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "sys_includes.h"
#include "net_typedef.h"
#include "net_link_node.h"

#if DBG_SYSTEM > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif

/*
********************************************************************************
* ��������
********************************************************************************
*/
#define REG_MAX     4



/*
********************************************************************************
* �ṹ����
********************************************************************************
*/
/* ����ڵ����� */
typedef struct {
    NODE            node;
    Net_link_node_t cell;
} NET_NODE_T;

typedef struct {
    LIST_T freelist;
    LIST_T onlinelist;
    LIST_T offlinelist;
    NET_NODE_T net_node[NET_NODE_MAX];
    Inform inform[REG_MAX];
    INT8U mantmr;
} LCB_T;

/*
********************************************************************************
* ��̬����
********************************************************************************
*/
static LCB_T s_lcb;

/*******************************************************************
** ������:     manager_tmr
** ��������:   �ڵ����ʱ��
** ����:       [in] index  : ��ʱ������
** ����:       ��
********************************************************************/
static void manager_tmr(void *index)
{
    INT8U i;
    Net_link_node_t *cell, *destcell;
    
    index = index;
    cell = (Net_link_node_t *)dlist_get_head(&s_lcb.onlinelist);
    for (;;) {
        if (cell == 0) {
            break;
        }
        
        if (cell->live == 0) {
            destcell = cell;
            cell = (Net_link_node_t *)dlist_next_ele((LISTMEM *)cell);
            
            destcell->status = OFFLINE;
            dlist_del_ele(&s_lcb.onlinelist, (LISTMEM *)destcell);
            dlist_append_ele(&s_lcb.offlinelist, (LISTMEM *)destcell);
            for (i = 0; i < REG_MAX; i++) {
                if (s_lcb.inform[i] == NULL) {
                    break;
                }
                s_lcb.inform[i](NODE_OFFLINE, destcell);
            }  
            continue;
        }
        cell->live--;    
        cell = (Net_link_node_t *)dlist_next_ele((LISTMEM *)cell);
    }
}

/*******************************************************************
** ������:      net_link_node_find_by_addr
** ��������:    ���ݵ�ַ��������ڵ�
** ����:        [in] plist: ָ������
**              [in] addr : ��ѯ��ַ       
** ����:        Net_link_node_t
********************************************************************/
Net_link_node_t* net_link_node_find_by_addr(LIST_T *plist, INT8U *addr)
{
    Net_link_node_t *cell;

    cell = (Net_link_node_t *)dlist_get_head(plist);
    for (;;) {
        if (cell == 0) {
            break;
        }
        if (strncmp((char*)cell->addr, (char*)addr, ADR_WIDTH) == 0) {
            return cell;
        } 
        cell = (Net_link_node_t *)dlist_next_ele((LISTMEM *)cell);
    }

    return NULL;
}

/*******************************************************************
** ������:      net_link_node_find_by_addr
** ��������:    ���ݵ�ַ�����Ѿ���������ڵ�
** ����:        [in] addr : ��ѯ��ַ
** ����:        Net_link_node_t
********************************************************************/
Net_link_node_t* net_link_node_find_exist(INT8U *addr)
{
    Net_link_node_t *cell;

    cell = net_link_node_find_by_addr(&s_lcb.onlinelist, addr);
    if (cell != NULL) {                                                        /* ���������д���Ҫ���µĽڵ� */
        return cell;
    }

    cell = net_link_node_find_by_addr(&s_lcb.offlinelist, addr);
    if (cell != NULL) {                                                        /* ���������д���Ҫ���µĽڵ� */
        return cell;
    }

    return NULL;
}

/*******************************************************************
** ������:      net_link_node_update
** ��������:    ֻ����ԭ�е�����ڵ㣬������
** ����:        [in] addr     : Ŀ���ַ
** ����:        Net_link_node_t
********************************************************************/
Net_link_node_t* net_link_node_update(INT8U *addr)
{
    INT8U i;
    Net_link_node_t *cell;

    cell = net_link_node_find_by_addr(&s_lcb.onlinelist, addr);
    if (cell != NULL) {                                                        /* ���������д���Ҫ���µĽڵ� */
        cell->live = LIVE_TIME_MAX;
        return cell;
    }

    cell = net_link_node_find_by_addr(&s_lcb.offlinelist, addr);
    if (cell != NULL) {                                                        /* ���������д���Ҫ���µĽڵ� */
        dlist_del_ele(&s_lcb.offlinelist, (LISTMEM *)cell);
        dlist_append_ele(&s_lcb.onlinelist, (LISTMEM *)cell);
        
        cell->status = ONLINE;
        cell->live = LIVE_TIME_MAX;
        for (i = 0; i < REG_MAX; i++) {
            if (s_lcb.inform[i] == NULL) {
                break;
            }
            s_lcb.inform[i](NODE_ONLINE, cell);
        }    

        return cell;   
    }
    return NULL;
}

/*******************************************************************
** ������:      net_link_node_new
** ��������:    ����������ڵ�
** ����:        ��      
** ����:        Net_link_node_t
********************************************************************/
Net_link_node_t* net_link_node_new(void)
{
    INT8U i;
    Net_link_node_t *cell;

    cell = (Net_link_node_t *)dlist_del_head(&s_lcb.freelist);                 /* �������Ҳ���Ҫ���µĽڵ㣬�ӿ���������һ��������ڵ� */
    if (cell != NULL) {
        memset(cell, 0, sizeof(Net_link_node_t));
        return cell;
    }

    cell = (Net_link_node_t *)dlist_del_head(&s_lcb.offlinelist);
    if (cell != NULL) {
        for (i = 0; i < REG_MAX; i++) {                                        /* ɾ���ɽڵ�֪ͨ */
            if (s_lcb.inform[i] == NULL) {
                continue;
            }
            s_lcb.inform[i](NODE_DEL, cell);
        } 
        
        memset(cell, 0, sizeof(Net_link_node_t));
        return cell;
    }
    return NULL;   
}

/*******************************************************************
** ������:      net_link_node_add
** ��������:    �������ڵ�
** ����:        [in] node: Ҫ��ӵĽڵ�
** ����:        true or false
********************************************************************/
BOOLEAN net_link_node_add(Net_link_node_t* node)
{
    INT8U i;

    return_val_if_fail(node != NULL, FALSE);

    node->status = ONLINE;
    node->live   = LIVE_TIME_MAX;
    dlist_append_ele(&s_lcb.onlinelist, (LISTMEM *)node);
    
    for (i = 0; i < REG_MAX; i++) {
        if (s_lcb.inform[i] == NULL) {
            continue;
        }
        s_lcb.inform[i](NODE_ADD, node);
    }    
    return TRUE;  
}

/*******************************************************************
** ������:      net_link_node_init
** ��������:    ����ڵ�����ʼ��
** ����:        ��
** ����:        ��
********************************************************************/
void net_link_node_init(void)
{
    memset(&s_lcb, 0, sizeof(s_lcb));

    dlist_init(&s_lcb.onlinelist);
    dlist_init(&s_lcb.offlinelist);
    dlist_mem_init(&s_lcb.freelist, (LISTMEM *)s_lcb.net_node, NET_NODE_MAX, sizeof(NET_NODE_T));

    s_lcb.mantmr = os_timer_create(0, manager_tmr);
    os_timer_start(s_lcb.mantmr, 2*SECOND);
}

/*******************************************************************
** ������:      net_link_node_get_list
** ��������:    ��ȡ��������
** ����:        [in] type: ��������
** ����:        LIST_T
********************************************************************/
LIST_T* net_link_node_get_list(INT8U type)
{
    if (type == NODE_ONLINE) {
        return &s_lcb.onlinelist;
    } else {
        return &s_lcb.offlinelist;
    }
}

/*******************************************************************
** ������:      net_link_node_reg
** ��������:    ע��ڵ����/ɾ��֪ͨ�ص�����
** ����:        [in] inform: �ص�����
** ����:        true or false
********************************************************************/
BOOLEAN  net_link_node_reg(Inform inform)
{
    INT8U i;
    
    for (i = 0; i < REG_MAX; i++) {
        if (s_lcb.inform[i] == NULL) {
            break;
        }
    }

    return_val_if_fail((inform != NULL) && (i < REG_MAX), FALSE);

    s_lcb.inform[i] = inform;
	return TRUE;
}

