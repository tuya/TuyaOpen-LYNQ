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
#include "stopWatch.h"
#include "sysAlarm.h"
#include "systime.h"

extern int skip_atoi(const char **s);

static void stopWatchCallback(uint32_t *timeList, uint32_t timeCount)
{
    for(uint32_t i = 0; i < timeCount; i++)
    {
        printf("%d:%02d:%02d.%03d\r\n", timeList[i] / 1000 / 60 / 60,
               timeList[i] / 1000 / 60 % 60, timeList[i] / 1000 % 60,
               timeList[i] % 1000);
    }
}

int32_t alarmCallback(SysAlarmInfoT *sysAlarmInfo)
{
    printf("%s: %02d:%02d\r\n", sysAlarmInfo->name, sysAlarmInfo->hour,
           sysAlarmInfo->minute);
}

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
    // time set_time 2024 9 25 14 30 00
    else if (strcmp(sub_cmd, "set_time") == 0)
    {
        struct tm tmTime;
        memset(&tmTime, 0, sizeof(tmTime));
        tmTime.tm_year = skip_atoi(&argv[2]) - 1900;
        tmTime.tm_mon = skip_atoi(&argv[3]) - 1;
        tmTime.tm_mday = skip_atoi(&argv[4]);
        tmTime.tm_hour = skip_atoi(&argv[5]) - timeZoneGet();
        tmTime.tm_min = skip_atoi(&argv[6]);
        tmTime.tm_sec = skip_atoi(&argv[7]);

        time_t ret = timeSet(&tmTime);
        rt_kprintlnf("set = %d", (uint32_t)ret);
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
    // time stop_watch_start 100 100
    else if(strcmp(sub_cmd, "stop_watch_start") == 0)
    {
        uint32_t interval = skip_atoi(&argv[2]);
        uint32_t countMax = skip_atoi(&argv[3]);
        stopWatchInit(stopWatchCallback, interval, countMax);
        stopWatchStart();
    }
    // time stop_watch_stop
    else if(strcmp(sub_cmd, "stop_watch_stop") == 0)
    {
        stopWatchStop();
        stopWatchDeinit();
    }
    // time stop_watch_count
    else if(strcmp(sub_cmd, "stop_watch_count") == 0)
    {
        printf("count = %d\r\n", stopWatchCount());
    }
    // time stop_watch_reset
    else if(strcmp(sub_cmd, "stop_watch_reset") == 0)
    {
        printf("reset = %d\r\n", stopWatchReset());
    }
    // time alarm_init
    else if(strcmp(sub_cmd, "alarm_init") == 0)
    {
        printf("init = %d\r\n", sysAlarmInit(alarmCallback, 100));
    }
    // time alarm_add test1 0 1 1
    else if(strcmp(sub_cmd, "alarm_add") == 0)
    {
        SysAlarmInfoT sysAlarmInfo = {0};
        memset(&sysAlarmInfo, 0, sizeof(sysAlarmInfo));
        memcpy(sysAlarmInfo.name, argv[2], strlen(argv[2]));
        // memcpy(sysAlarmInfo.tone, "D:/s1imy03_imy.mp3",
        //        strlen("D:/s1imy03_imy.mp3"));
        sysAlarmInfo.vibrate = true;
        sysAlarmInfo.on = true;
        sysAlarmInfo.hour = skip_atoi(&argv[3]);
        sysAlarmInfo.minute = skip_atoi(&argv[4]);
        sysAlarmInfo.repeat = 1 << (skip_atoi(&argv[5]) - 1);
        printf("add = %d\r\n", sysAlarmAdd(&sysAlarmInfo));
    }
    // time alarm_delete 1
    else if(strcmp(sub_cmd, "alarm_delete") == 0)
    {
        printf("delete = %d\r\n", sysAlarmDelete(skip_atoi(&argv[2])));
    }
    // time alarm_edit 1 test1 0 1 1
    else if(strcmp(sub_cmd, "alarm_edit") == 0)
    {
        SysAlarmInfoT sysAlarmInfo = {0};
        memset(&sysAlarmInfo, 0, sizeof(sysAlarmInfo));
        memcpy(sysAlarmInfo.name, argv[3], strlen(argv[3]));
        sysAlarmInfo.vibrate = true;
        sysAlarmInfo.on = true;
        sysAlarmInfo.hour = skip_atoi(&argv[4]);
        sysAlarmInfo.minute = skip_atoi(&argv[5]);
        sysAlarmInfo.repeat = 1 << (skip_atoi(&argv[6]) - 1);
        printf("edit = %d\r\n",
               sysAlarmEdit(skip_atoi(&argv[2]), &sysAlarmInfo));
    }
    // time alarm_count
    else if(strcmp(sub_cmd, "alarm_count") == 0)
    {
        printf("count = %d\r\n", sysAlarmGetCount());
    }
    // time alarm_list
    else if(strcmp(sub_cmd, "alarm_list") == 0)
    {
        SysAlarmInfoT *list = sysAlarmGetList();
        for(int i = 0; i < sysAlarmGetCount(); i++)
        {
            printf("alarm[%d] name = %s\r\n", i, list[i].name);
            printf("alarm[%d] tone = %s\r\n", i, list[i].tone);
            printf("alarm[%d] vibrate = %d\r\n", i, list[i].vibrate);
            printf("alarm[%d] on = %d\r\n", i, list[i].on);
            printf("alarm[%d] hour = %d\r\n", i, list[i].hour);
            printf("alarm[%d] minute = %d\r\n", i, list[i].minute);
            char string[33];
            itoa(list[i].repeat, string, 2);
            printf("alarm[%d] repeat = %s\r\n", i, string);
            printf("\r\n");
        }
    }
    else
    {
        printf("Usage: time [options]\r\n");
        printf("[options]:\r\n");
        printf("    %-31s - sync time\r\n", "sync");
        printf("    %-31s - set time zone\r\n", "set_time_zone");
        printf("    %-31s - get time zone\r\n", "get_time_zone");
        printf("    %-31s - mktime\r\n", "mktime");
        printf("    %-31s - set time\r\n", "set_time");
        printf("    %-31s - get current unix time\r\n", "get_current_unix_time");
        printf("    %-31s - get current ctime\r\n", "get_current_ctime");
        printf("    %-31s - get current gmtime\r\n", "get_current_gmtime");
        printf("    %-31s - get current local time\r\n", "get_current_local_time");
        printf("    %-31s - get gmtime from unix time\r\n", "get_gmtime_from_unix_time");
        printf("    %-31s - get local time from unix time\r\n", "get_local_time_from_unix_time");
        printf("    %-31s - get diff time between unix time\r\n", "get_diff_time_between_unix_time");
        printf("    %-31s - stop watch start\r\n", "stop_watch_start");
        printf("    %-31s - stop watch stop\r\n", "stop_watch_stop");
        printf("    %-31s - stop watch count\r\n", "stop_watch_count");
        printf("    %-31s - stop watch reset\r\n", "stop_watch_reset");
        printf("    %-31s - alarm init\r\n", "alarm_init");
        printf("    %-31s - alarm add\r\n", "alarm_add");
        printf("    %-31s - alarm delete\r\n", "alarm_delete");
        printf("    %-31s - alarm edit\r\n", "alarm_edit");
        printf("    %-31s - alarm count\r\n", "alarm_count");
        printf("    %-31s - alarm list\r\n", "alarm_list");
    }
    return 0;
}
MSH_CMD_EXPORT_ALIAS(cmd_time, time, systime test);

#endif
