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
#ifdef FEATURE_SUBSYS_APP_MANAGER_ENABLE
#include "appmanager.h"
#endif
#ifdef FEATURE_SUBSYS_FATFS_ENABLE
#include "ff.h"
#endif

#ifdef FEATURE_SUBSYS_FINSH_ENABLE

#ifdef FEATURE_SUBSYS_APP_MANAGER_ENABLE
extern int32_t appRun_ram(char *name);
void console_app_load(int argc, char **argv)
{
    printf("console_app_load\r\n");
    appRun_ram(argv[1]);
    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_app_load, appload, app run);

extern int32_t appRun_ram_run();
void console_app_run(int argc, char **argv)
{
    printf("console_app_run\r\n");
    appRun_ram_run();
    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_app_run, apprun, app run);

void console_appManagerInstallApp(int argc, char **argv)
{
    bool autoStart = false;

    if ((argc == 3) && (argv[2][0] == '1'))
    {
        autoStart = true;
    }

    appManagerInstallApp(argv[1], autoStart);
    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerInstallApp, appManagerInstallApp, Install app);

void console_appManagerUninstallApp(int argc, char **argv)
{
    appManagerUninstallApp(argv[1]);
    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerUninstallApp, appManagerUninstallApp, Uninstall app);

void console_appManagerStartApp(int argc, char **argv)
{
    appManagerStartApp(argv[1]);
    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerStartApp, appManagerStartApp, Start app);

void console_appManagerStopApp(int argc, char **argv)
{
    appManagerStopApp(argv[1]);
    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerStopApp, appManagerStopApp, Stop app);

void console_appManagerSetAppAutoStart(int argc, char **argv)
{
    bool autoStart = false;

    if (argc == 3)
    {
        if (argv[2][0] == '1')
        {
            autoStart = true;
        }
        appManagerSetAppAutoStart(argv[1], autoStart);
    }

    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerSetAppAutoStart, appManagerSetAppAutoStart, Set app auto-start status);

void console_appManagerGetAppByName(int argc, char **argv)
{
    AppDetailT appDetail = {0};

    if (argc == 2)
    {
        appManagerGetAppByName(argv[1], &appDetail);
    }

    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerGetAppByName, appManagerGetAppByName, Get app);

void console_appManagerGetAppsByType(int argc, char **argv)
{
    uint8_t     type      = 0;
    AppDetailT *appDetail = NULL;

    if (argc == 2)
    {
        type = argv[1][0] - '0';
        appManagerGetAppsByType(type, &appDetail);
        if (appDetail != NULL)
        {
            free(appDetail);
            appDetail = NULL;
        }
    }

    rt_kprintf("\n\r%s >",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_appManagerGetAppsByType, appManagerGetAppsByType, Get apps);
#endif

#endif

#endif