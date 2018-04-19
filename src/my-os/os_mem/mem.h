/********************************************************************************
**
** �ļ���:     mem.h
** ��Ȩ����:   
** �ļ�����:   ʵ�ֶ�ʱ������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����       | ����   |  �޸ļ�¼
**===============================================================================
**| 2012/08/14 | ���ѽ� |  �������ļ�
*********************************************************************************/
#ifndef __MEM_H__
#define __MEM_H__


#ifdef __cplusplus
extern "C" {
#endif

#define OS_NOASSERT 0
#if     OS_NOASSERT
#define OS_ASSERT(x,y) do { if(!(y)) OS_PLATFORM_ASSERT(x); } while(0)
#else  /* OS_NOASSERT */
#define OS_ASSERT(x,y) 
#endif /* OS_NOASSERT */



#ifndef MEM_ALIGNMENT                   /* ------------------ �ֶ������ø���������� ------------------ */
#define MEM_ALIGNMENT             4     /* ���β��õ���ARM7TDMI�������������4�ֽڶ���� -------------- */
#endif

#ifndef MEM_SIZE
#define MEM_SIZE                  2*1024/* ��̬�ڴ��(HEAP)�������,Ϊ��̬�ڴ�����С ---------------- */
#endif

#ifndef MEM_MOVE_UP
#define MEM_MOVE_UP               1
#endif

#ifndef MEM_MOVE_DOWN
#define MEM_MOVE_DOWN             0
#endif

typedef INT32U               mem_ptr_t;
/* MEM_SIZE would have to be aligned, but using 64000 here instead of
 * 65535 leaves some room for alignment...
 */
#if MEM_SIZE > 64000l
typedef INT32U mem_size_t;
#else
typedef INT16U mem_size_t;
#endif /* MEM_SIZE > 64000 */


#if 0
typedef struct packet_buf_t {
    /** next pbuf in singly linked pbuf chain */
    struct packet_buf_t *next;
    void  *curptr;
    INT16U free_len;
    INT16U used_len;  
    INT16U tot_len;
    INT8U  state;
}PACKET_BUF_T;
#endif /* if 0. 2014-10-18 10:55:36 suyoujiang */

/* lwIP alternative malloc */
void  mem_init(void);
void *mem_malloc(mem_size_t size);
void *mem_realloc(void *rmem, mem_size_t newsize);
void *mem_calloc(mem_size_t count, mem_size_t size);
void  mem_free(void *mem);
char *mem_strdup(const char *s);

#if 0
/*******************************************************************
** ������:     pbuf_alloc
** ��������:   ��������涯̬�ڴ�
** ����:       [in] size:       ĿǰҪ�õĴ�С 
** ����:       [in] freesize:   Ԥ�����ֵĴ�С
** ����:       ��̬���ڴ��׵�ַ
** ע��:       
********************************************************************/
PACKET_BUF_T *pbuf_alloc(mem_size_t size, mem_size_t freesize);

/*******************************************************************
** ������:     pbuf_move
** ��������:   �ƶ���̬�ڴ��������ָ��
** ����:       [in] pbuf:   Ҫ�����Ķ�̬�ڴ����ָ�� 
** ����:       [in] size:   �ƶ��ĳ���
** ����:       [in] dir:    �ƶ��ķ���
** ����:       true or false
** ע��:       
********************************************************************/
BOOLEAN pbuf_move(PACKET_BUF_T *pbuf, mem_size_t size, BOOLEAN dir);
#endif /* if 0. 2014-10-18 10:55:26 suyoujiang */

#ifndef OS_MEM_ALIGN_SIZE
#define OS_MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))
#endif

#ifndef OS_MEM_ALIGN
#define OS_MEM_ALIGN(addr) ((void *)(((mem_ptr_t)(addr) + MEM_ALIGNMENT - 1) & ~(mem_ptr_t)(MEM_ALIGNMENT-1)))
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MEM_H__ */
