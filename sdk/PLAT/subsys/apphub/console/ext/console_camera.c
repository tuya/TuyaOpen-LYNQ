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
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_SYSTIME_ENABLE
#include "systime.h"
#endif

#include "open_image.h"

#ifdef FEATURE_SUBSYS_FINSH_ENABLE

#ifdef FEATURE_SUBSYS_CAMERA_ENABLE
void console_cam_preview_start(int argc, char **argv)
{
    printf("console_cam_preview_start\r\n");
    cameraStart();
    rt_kprintf("\n\r%s",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_cam_preview_start, camstart, camera preview start);

void console_cam_preview_stop(int argc, char **argv)
{
    printf("console_cam_preview_stop \r\n");
    cameraStop();
    rt_kprintf("\n\r%s",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_cam_preview_stop, camstop, camera preview stop);

char dcim_name_buf[128] = {0};
void console_cam_shoot(int argc, char **argv)
{
    OPEN_IMAGE_INFO info={0};
    time_t  curtime  = 0;
    struct tm       *tmTime = NULL;
    struct timeval   tv     = {0};
    struct timezone  tz     = {0};

    info.width = 320;
    info.height = 240;

    printf("console_cam_shoot \r\n");
    curtime = time_time(NULL);
    // SYSLOG_INFO("time:%d\r\n", (uint32_t)curtime);
    // SYSLOG_INFO("ctime:%s\r\n", time_ctime(&curtime));
    tmTime = time_localtime(&curtime);
    // SYSLOG_INFO("localtime:%d-%d-%d %d:%d:%d %d\r\n", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec, tmTime->tm_wday);
    time_gettimeofday(&tv, &tz);
    // SYSLOG_INFO("gettimeofday:tv_sec=%d,tv_usec=%d,tz_minuteswest=%d\r\n", (uint32_t)tv.tv_sec, (uint32_t)tv.tv_usec, (uint32_t)tz.tz_minuteswest);
    snprintf(dcim_name_buf, sizeof(dcim_name_buf), "D:/DCIM_%d%02d%02d_%02d%02d%02d_%d.jpg", 1900 + tmTime->tm_year, tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec,tv.tv_usec/10000);
    SYSLOG_INFO("dcim_name_buf:%s\r\n",dcim_name_buf);
    cameraStartShoot();
    extern uint8_t photo_buf[];
    extern int cam_data_rdy;
    while (cam_data_rdy != 2)
    {
        SYSLOG_INFO("cam_data_rdy:%d\n",cam_data_rdy);
        osDelay(50);
    }
    cam_data_rdy = 0;
    
    //save
    SYSLOG_INFO("cam_data_rdy:%d\r\n",cam_data_rdy);
#define SAVE_CAM_YUV 0
#if SAVE_CAM_YUV
    printf("save yuv start \r\n");
    FILE        *file         = NULL;
    file = file_fopen("d:/raw1.yuv", "wb");
    file_fwrite(photo_buf,info.width*info.height*2 , 1, file);
    file_fclose(file);
    printf("save file success\r\n");
#endif
    open_image_save(dcim_name_buf,&info,photo_buf,0);
    rt_kprintf("\n\r%s",getConsolePrompt());
}
MSH_CMD_EXPORT_ALIAS(console_cam_shoot, camshoot, camera take a phote);
#endif

#endif

#endif