/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_servicemanager_test.c
 * Description:  EC718
 * History:      Rev1.0   2025-09-28
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "string.h"
#include "servicemanager.h"
#include "clock.h"
#include <stdbool.h>

extern int skip_atoi(const char **s);

int cmd_servicemanager(int argc, char **argv)
{
    char *sub_cmd = argv[1];
    // servicemanager list
    if(strcmp(sub_cmd, "list") == 0)
    {
        Services_list();
    }
    // servicemanager start service:/halexpt
    else if(strcmp(sub_cmd, "start") == 0)
    {
        printf("ret = %d\r\n", Service_start(&argv[2]));
    }
    // servicemanager start service:/halexpt
    else if(strcmp(sub_cmd, "stop") == 0)
    {
        printf("ret = %d\r\n", Service_stop(&argv[2]));
    }
    else
    {
        rt_kprintlnf("Usage: servicemanager [options]");
        rt_kprintlnf("[options]:");
        rt_kprintlnf("    %-10s - get service list", "list");
        rt_kprintlnf("    %-10s - start service", "start");
        rt_kprintlnf("    %-10s - stop service", "stop");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_servicemanager, servicemanager, servicemanager test);

#endif  // FEATURE_SUBSYS_CONSOLE_ENABLE