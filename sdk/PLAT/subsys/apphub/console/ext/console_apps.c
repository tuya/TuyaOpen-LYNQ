/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console.c
 * Description:  EC718 
 * History:      Rev1.0   2023-03-03
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

#ifdef FEATURE_SUBSYS_FINSH_ENABLE

#ifdef FEATURE_SUBSYS_APP_MANAGER_ENABLE
void console_app_run(int argc, char **argv)
{
    printf("console_app_run\r\n");
    appRun(argv[1]);
    rt_kprintf("\n\r%s",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_app_run, apprun, app run);


#endif

#endif

#endif