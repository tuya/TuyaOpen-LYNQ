/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_gnss.c
 * Description:  EC718
 * History:      Rev1.0   2024-10-29
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "rtthread.h"
#include "gnss.h"

extern int skip_atoi(const char **s);

int cmd_gnss(int argc, char **argv)
{

    char *sub_cmd = argv[1];
    // gnss start
    if (strcmp(sub_cmd, "start") == 0)
    {
        gnssStart();
    }
    // gnss stop
    else if (strcmp(sub_cmd, "stop") == 0)
    {
        gnssStop();
    }
    // gnss isFix
    else if (strcmp(sub_cmd, "isFix") == 0)
    {
        printf("%d\r\n", gnssIsFix());
    }
    // gnss getLatitude
    else if (strcmp(sub_cmd, "getLatitude") == 0)
    {
        printf("%s\r\n", gnssGetLatitude());
    }
    // gnss getLongitude
    else if (strcmp(sub_cmd, "getLongitude") == 0)
    {
        printf("%s\r\n", gnssGetLongitude());
    }
    // gnss getAltitude
    else if (strcmp(sub_cmd, "getAltitude") == 0)
    {
        printf("%s\r\n", gnssGetAltitude());
    }
    else
    {
        rt_kprintlnf("Usage: gnss [options]");
        rt_kprintlnf("[options]:");
        rt_kprintlnf("    %-20s - start gnss", "start");
        rt_kprintlnf("    %-20s - stop gnss", "stop");
        rt_kprintlnf("    %-20s - gnss isFix", "isFix");
        rt_kprintlnf("    %-20s - gnss getLatitude", "getLatitude");
        rt_kprintlnf("    %-20s - gnss getLongitude", "getLongitude");
        rt_kprintlnf("    %-20s - gnss getAltitude", "getAltitude");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_gnss, gnss, gnss test);

#endif // FEATURE_SUBSYS_CONSOLE_ENABLE