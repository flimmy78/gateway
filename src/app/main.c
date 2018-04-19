/********************************************************************************
**
** �ļ���:     main.c
** ��Ȩ����:   
** �ļ�����:   main����,�������
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**|    ����    |  ����  |  �޸ļ�¼
**===============================================================================
**| 2015/06/12 | ���ѽ� |  �������ļ�
********************************************************************************/
#include "bsp.h"
#include "sys_includes.h"
#include <stdarg.h>

#if DBG_SYSTEM > 0
#define SYS_DEBUG          OS_DEBUG
#else
#define SYS_DEBUG(...)     do{}while(0)
#endif



/*
********************************************************************************
* ����ȫ�ֱ���
********************************************************************************
*/
Sys_base_info_t g_sys_base_info;
Sys_run_info_t g_sys_run_info;

/*
********************************************************************************
* ���徲̬����
********************************************************************************
*/
static INT8U s_systmr;
static volatile BOOLEAN s_systick_flag = 0;
BOOLEAN g_debug_sw = 1;

#if EN_CPU_USAGE > 0 
/*
********************************************************************************
*                          OSIdleCtr
* OSCPUUsage = 100 * (1 - ------------)     (units are in %)
*                         OSIdleCtrMax
********************************************************************************
*/

typedef struct {
    INT32U idle_ctr_max;
    INT8U  usage;
} Cpu_stats_t;

/* cpuʹ����ͳ����ر��� */
static volatile INT32U s_idle_ctr;
static volatile INT32U s_tick_ctr;
static Cpu_stats_t s_cpu_stats;
#endif

static void _run_led(void)
{
    static BOOLEAN flag = 0;
    
    if (flag) {
        flag = 0;
        GPIO_SetBits(GPIOB, GPIO_Pin_9);
    } else {
        flag = 1;
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    }
}

static void _system_tmr(void *index)
{
	bsp_watchdog_feed();
    _run_led();

	SYS_DEBUG("<time:%d,%d,%d,%d,[%d:%d:%d] run:%d>\n", g_systime.tm_year, g_systime.tm_mon, g_systime.tm_mday, g_systime.tm_wday, g_systime.tm_hour, g_systime.tm_min, g_systime.tm_sec, g_run_sec);
}

static void _system_init(void)
{
    s_systmr = os_timer_create(0, _system_tmr);
    os_timer_start(s_systmr, SECOND);

    /* ͳ��ϵͳ�����������Ϳ��Ź���λ���� */
    if (!public_para_manager_read_by_id(SYS_RUN_INFO_, (INT8U*)&g_sys_run_info, sizeof(Sys_run_info_t))) {        
        memset(&g_sys_run_info, 0, sizeof(Sys_run_info_t));
    }    

	/* Check if the system has resumed from IWDG reset */
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) {
		/* IWDGRST flag set */
		SYS_DEBUG("<Stm32 reset from iwdg!!!>\r\n");
		/* Clear reset flags */
		RCC_ClearFlag();
        g_sys_run_info.wdgcnt++;
	}
    g_sys_run_info.restartcnt++;

    public_para_manager_store_by_id(SYS_RUN_INFO_, (INT8U*)&g_sys_run_info, sizeof(Sys_run_info_t));
	SYS_DEBUG("<---restart cnt:%d, iwdg cnt:%d !!!>\r\n", g_sys_run_info.restartcnt, g_sys_run_info.wdgcnt);

#if EN_CPU_USAGE > 0 
    /* ����һ��systick��cpu�����ʵ�������ֵ */
    s_idle_ctr = 0;
    s_systick_flag = 0;
    while (!s_systick_flag);
    s_systick_flag = 0;
    while (!s_systick_flag) {
        s_idle_ctr++;
    }

    s_cpu_stats.idle_ctr_max = s_idle_ctr;
    s_tick_ctr = 0;
    s_idle_ctr = 0;
    SYS_DEBUG("idle_ctr_max:%x\r\n", s_cpu_stats.idle_ctr_max);

#endif    
}

/*******************************************************************
** ������:      main
** ��������:    ���������
** ����:        [in] argc :  �ļ�����ʱ �Կո�Ϊ��������в�������
**              [in] argv :  �ַ�������,�������ָ������ַ���������ָ������,ÿһ��Ԫ��ָ��һ������ 
** ����:        
********************************************************************/
int main(int argc, char* argv[])
{
    /* �弶֧�ְ���ʼ�� */
    bsp_init();

    /* Ӳ���쳣׷�ٿ��ʼ�� */
    cm_backtrace_init("enddevice", HARDWARE_VERSION_STR, SOFTWARE_VERSION_STR);/* �̼����ƣ�������������ɵĹ̼����ƶ�Ӧ */

#if 0 /* hard fault test */
    fault_test_by_div0();
#endif /* if 0. 2017-4-21 22:30:36 syj */
    /* my os��ʼ�� */
    os_timer_init();                                                           /* ��ʱ����ʼ�� */
    mem_init();                                                                /* ��̬�ڴ��ʼ�� */
    os_systime_init();

    /* my mdl��ʼ�� */
    public_para_manager_init();
    
    if (!public_para_manager_read_by_id(UID_, (INT8U*)&g_sys_base_info.uid, sizeof(g_sys_base_info.uid))) {        
        g_sys_base_info.uid = 0xffffffff;
    }

    if (!public_para_manager_read_by_id(GROUP_, (INT8U*)g_sys_base_info.group, GROUP_NUM)) {        
        memset(g_sys_base_info.group, 0, GROUP_NUM);
    }    
    
    /* my drv��ʼ�� */
#if EN_ZIGBEE > 0
    drv_zigbee_init();
#endif

#if EN_WIFI > 0
    drv_esp8266_init();
#endif

#if EN_ETHERNET > 0
    drv_ethernet_init();
#endif
#if EN_DALI > 0
    drv_dali_init();
#endif

#if EN_NRF24L01 > 0
    net_link_init();
#endif

    tts_init();

    /* app��ʼ�� */
    /* ע:���������Щʹ�ý���ע�ắ��֮ǰ */
    debug_cmd_init();

    comm_manager_init();

    para_manager_init();
    general_manager_init();
    lamp_manager_init();
    energy_measure_init();
    task_manager_init();
    alarm_manager_init();    

#if EN_UPDATE > 0 
    update_init();
#endif

#if EN_YEELINK > 0
    yeelink_init();
#endif
    wsn_sensor_init();
    
    _system_init();
    
	for (;;) {
        while (!s_systick_flag) {
            #if EN_CPU_USAGE > 0             
            s_idle_ctr++;
            #endif            
        }
        s_systick_flag = 0;

        #if EN_CPU_USAGE > 0         
        if (s_tick_ctr >= 200uL) {                                             /* 1sͳ��һ��cpuʹ���ʣ�200��systickʱ�Ӿ����� */
            s_idle_ctr *= 100uL;
            s_cpu_stats.usage = (INT8U)(100uL - s_idle_ctr / (s_cpu_stats.idle_ctr_max * s_tick_ctr));
            s_idle_ctr = 0;
            s_tick_ctr = 0;
            SYS_DEBUG("cpu usage:%d%%\r\n", s_cpu_stats.usage);
        }
        #endif        
        os_timer_scan();
        os_msg_sched();
	}
    
    //return 0;
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    s_systick_flag = 1;
#if EN_CPU_USAGE > 0
    s_tick_ctr++;
#endif
}

INT8U cpu_usage_get(void)
{
#if EN_CPU_USAGE > 0
    return s_cpu_stats.usage;
#else
    return 0;
#endif
}

int OS_DEBUG(const char *format,...)
{
	va_list arg; 
	int done = 0; 

    if (g_debug_sw == 0) {
        return done;
    }

	va_start(arg, format); 
	done += vprintf(format, arg); 
	va_end(arg);

	return done;
}

