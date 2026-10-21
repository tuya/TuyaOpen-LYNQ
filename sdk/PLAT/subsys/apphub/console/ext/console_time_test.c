/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    console_time.c
 * Description:  EC718
 * History:      Rev1.0   2024-03-27
 *
 ****************************************************************************/

#ifdef FEATURE_SUBSYS_CONSOLE_ENABLE

#include "string.h"
#include "rtthread.h"
#include "systime.h"

extern int skip_atoi(const char **s);

int cmd_time(int argc, char **argv)
{
    char *sub_cmd = argv[1];
    // time sync
    if (strcmp(sub_cmd, "sync") == 0)
    {
        rt_kprintf("%d\r\n", timeSync());
    }
    // time set_time_zone 8
    else if (strcmp(sub_cmd, "set_time_zone") == 0)
    {
        timeZoneSet(skip_atoi(&argv[2]));
    }
    // time get_time_zone
    else if (strcmp(sub_cmd, "get_time_zone") == 0)
    {
        rt_kprintlnf("%d", timeZoneGet());
    }
    // time mktime 2024 9 25 14 30 00
    else if (strcmp(sub_cmd, "mktime") == 0)
    {
        struct tm tmTime;
        memset(&tmTime, 0, sizeof(tmTime));
        tmTime.tm_year = skip_atoi(&argv[2]) - 1900;
        tmTime.tm_mon = skip_atoi(&argv[3]) - 1;
        tmTime.tm_mday = skip_atoi(&argv[4]);
        tmTime.tm_hour = skip_atoi(&argv[5]) - timeZoneGet();
        tmTime.tm_min = skip_atoi(&argv[6]);
        tmTime.tm_sec = skip_atoi(&argv[7]);

        time_t unix_time = time_mktime(&tmTime);
        rt_kprintlnf("%d", (uint32_t)unix_time);
    }
    // time get_current_unix_time
    else if (strcmp(sub_cmd, "get_current_unix_time") == 0)
    {
        time_t time1 = time_time(NULL);
        rt_kprintf("%d\r\n", (uint32_t)time1);
    }
    // time get_current_ctime
    else if (strcmp(sub_cmd, "get_current_ctime") == 0)
    {
        time_t time1 = time_time(NULL);
        char *ctime = time_ctime(&time1);
        rt_kprintf("%s\r\n", ctime);
    }
    // time get_current_gmtime
    else if (strcmp(sub_cmd, "get_current_gmtime") == 0)
    {
        time_t time1 = time_time(NULL);
        struct tm *tmTime = time_gmtime(&time1);
        rt_kprintf("%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);
    }
    // time get_current_local_time
    else if (strcmp(sub_cmd, "get_current_local_time") == 0)
    {
        time_t time1 = time_time(NULL);
        struct tm *tmTime = time_localtime(&time1);
        rt_kprintf("%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);
    }
    // time get_gmtime_from_unix_time 1727248438
    else if (strcmp(sub_cmd, "get_gmtime_from_unix_time") == 0)
    {
        time_t unix_time = (time_t)skip_atoi(&argv[2]);
        struct tm *tmTime = time_gmtime(&unix_time);
        rt_kprintf("%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);
    }
    // time get_local_time_from_unix_time 1727248438
    else if (strcmp(sub_cmd, "get_local_time_from_unix_time") == 0)
    {
        time_t unix_time = (time_t)skip_atoi(&argv[2]);
        struct tm *tmTime = time_localtime(&unix_time);
        rt_kprintf("%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);
    }
    // time get_diff_time_between_unix_time 1727248438 1727248439
    else if (strcmp(sub_cmd, "get_diff_time_between_unix_time") == 0)
    {
        time_t time1 = (time_t)skip_atoi(&argv[2]);
        time_t time2 = (time_t)skip_atoi(&argv[3]);
        uint32_t diff_time = (uint32_t)time_difftime(time2, time1);
        rt_kprintf("%d\r\n", diff_time);
    }
    else
    {
        rt_kprintf("Usage: time [options]\n\r");
        rt_kprintf("[options]:\n\r");
        rt_kprintf("    %-31s - sync time\n\r", "sync");
        rt_kprintf("    %-31s - set time zone\n\r", "set_time_zone");
        rt_kprintf("    %-31s - get time zone\n\r", "get_time_zone");
        rt_kprintf("    %-31s - mktime\n\r", "mktime");
        rt_kprintf("    %-31s - get current unix time\n\r", "get_current_unix_time");
        rt_kprintf("    %-31s - get current ctime\n\r", "get_current_ctime");
        rt_kprintf("    %-31s - get current gmtime\n\r", "get_current_gmtime");
        rt_kprintf("    %-31s - get current local time\n\r", "get_current_local_time");
        rt_kprintf("    %-31s - get gmtime from unix time\n\r", "get_gmtime_from_unix_time");
        rt_kprintf("    %-31s - get local time from unix time\n\r", "get_local_time_from_unix_time");
        rt_kprintf("    %-31s - get diff time between unix time\n\r", "get_diff_time_between_unix_time");
    }
    rt_kprintf("\n\r%s", getConsolePrompt());
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_time, time, systime test);

#endif
