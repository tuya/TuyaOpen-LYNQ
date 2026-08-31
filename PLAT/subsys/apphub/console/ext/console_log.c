/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_log.c
 * Description:  EC718
 * History:      Rev1.0   2024-05-23
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "ps_lib_api.h"
#include "cmisim.h"
#include "cmips.h"
#include "networkmgr.h"
#include "slpman.h"
#include "time.h"
#include "storage.h"
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#include "status.h"
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
#endif
#include "bsp.h"
#include "packet.h"

#include "console.h"
#include "console_ex.h"
#include "console_hal.h"
#include "console_file.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE
#include "rtthread.h"
#include "shell.h"
#endif
#ifdef FEATURE_SUBSYS_PIKAPYTHON_ENABLE
#include "pikaScript.h"
#endif
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_CMDPARSE_ENABLE
#include "cmdparse.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTEST_ENABLE
#include "systest.h"
#endif
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "ff.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
#endif
// #if defined(FEATURE_SUBSYS_AUDIO_ENABLE) || defined(FEATURE_SUBSYS_MEDIA_ENABLE)
// #ifdef FEATURE_SUBSYS_AUDIO_ENABLE
// #include "audio.h"
// #else
// #include "media.h"
// #endif
// #ifdef FEATURE_SUBSYS_MEDIA_AUDIO_PCM_ENABLE
// #include "pcm.h"
// #endif
// #endif

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#ifdef FEATURE_SUBSYS_FINSH_ENABLE
int cmd_log(int argc, char **argv)
{
    if (argc > 4)
    {
        rt_kprintf("argc > 4");
        return 0;
    }
    else
    {
        char *sub_cmd = argv[1];
        if (strcmp(sub_cmd, "set_level") == 0)
        {
            syslogSetLevel(skip_atoi(&argv[2]));
            
        }
        else if (strcmp(sub_cmd, "get_level") == 0)
        {
            rt_kprintf("%d\r\n", syslogGetLevel());
            
        }
        else if (strcmp(sub_cmd, "set_channel") == 0)
        {
            // SC_UART0 = (1 << 0), 1
            // SC_UART1 = (1 << 1), 2
            // SC_UART2 = (1 << 2), 4
            // SC_UART3 = (1 << 3), 8
            // SC_USB   = (1 << 4), 16
            // SC_LCD   = (1 << 5), 32
            // SC_EPAT  = (1 << 6), 64
            // SC_FILE  = (1 << 7), 128
            syslogSetChannel(skip_atoi(&argv[2]));
            
        }
        else if (strcmp(sub_cmd, "get_channel") == 0)
        {
            rt_kprintf("%d\r\n", syslogGetChannel());
            
        }
        else
        {
            goto _usage;
        }
        return 0;
    }
_usage:
    rt_kprintf("Usage: log [options]\n\r");
    rt_kprintf("[options]:\n\r");
    rt_kprintf("    %-31s - set level\n\r", "set_level");
    rt_kprintf("    %-31s - get level\n\r", "get_level");
    rt_kprintf("    %-31s - set channel\n\r", "set_channel");
    rt_kprintf("    %-31s - get channel\n\r", "get_channel");
    
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_log, log, log ctrl);

#endif
#endif
#endif
