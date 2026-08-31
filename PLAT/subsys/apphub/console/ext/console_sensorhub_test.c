/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_gsensor_test.c
 * Description:  EC718
 * History:      Rev1.0   2024-11-29
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"

extern int skip_atoi(const char **s);

int cmd_sensorhub(int argc, char **argv)
{

    char *sub_cmd = argv[1];
    // gnss start
    if (strcmp(sub_cmd, "start") == 0)
    {
        // gnssStart();
    }
    // gnss stop
    else if (strcmp(sub_cmd, "stop") == 0)
    {
        // gnssStop();
    }
    else
    {
        rt_kprintlnf("Usage: sensorhub [options]");
        rt_kprintlnf("[options]:");

    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_sensorhub, sensorhub, sensorhub test);

#endif // FEATURE_SUBSYS_CONSOLE_ENABLE