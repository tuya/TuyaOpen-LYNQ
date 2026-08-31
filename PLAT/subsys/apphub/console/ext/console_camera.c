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

#include "camera.h"

#include "open_image.h"

osSemaphoreId_t semGetFrame;

#ifdef FEATURE_SUBSYS_FINSH_ENABLE

#ifdef FEATURE_SUBSYS_CAMERA_ENABLE
void console_cam_start(int argc, char **argv)
{
    printf("console_cam_start\r\n");
    cameraStart();
}
MSH_CMD_EXPORT_ALIAS(console_cam_start, camstart, camera start);

void console_cam_stop(int argc, char **argv)
{
    printf("console_cam_stop \r\n");
    cameraStop();
}
MSH_CMD_EXPORT_ALIAS(console_cam_stop, camstop, camera stop);
extern uint32_t scr_dev_UsrId;
void console_cam_preview_start(int argc, char **argv)
{
    printf("console_cam_preview_start\r\n");
    cameraStartPreview(scr_dev_UsrId);
    printf("cmd: camstartpre, ret: %d, value: %d\r\n", 0, 0);
}
MSH_CMD_EXPORT_ALIAS(console_cam_preview_start, camstartpre,
                     camera preview start);

void console_cam_preview_stop(int argc, char **argv)
{
    printf("console_cam_preview_stop \r\n");
    cameraStopPreview(scr_dev_UsrId);
    printf("cmd: camstoppre, ret: %d, value: %d\r\n", 0, 0);
}
MSH_CMD_EXPORT_ALIAS(console_cam_preview_stop, camstoppre, camera preview stop);

void console_cam_preview_flip(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_MIRROR_FLIP, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_MIRROR_FLIP, &value);
    }
    printf("cmd: camFlip, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_preview_flip, camFlip, camera preview flip);

void console_cam_wr_reg(int argc, char **argv)
{
    char *endptr;
    long reg_addr = strtol(argv[1], &endptr, 16);
    long reg_value = strtol(argv[2], &endptr, 16);
    cameraWrReg((uint32_t)reg_addr, (uint32_t)reg_value);
    printf("cmd: camWrReg, ret: 0, value: 0x%02X \r\n", reg_value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_wr_reg, camWrReg, camera write register);

void console_cam_rd_reg(int argc, char **argv)
{
    char *endptr;
    long reg_addr = strtol(argv[1], &endptr, 16);
    uint32_t reg_value = cameraRdReg((uint32_t)reg_addr);
    printf("cmd: camRdReg, ret: 0, value: 0x%02X \r\n", reg_value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_rd_reg, camRdReg, camera read register);

void console_cam_ae(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_AE, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_AE, &value);
    }
    printf("cmd: camAe, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_ae, camAe, camera set or get ae mode);

void console_cam_awb(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_AWB, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_AWB, &value);
    }
    printf("cmd: camAwb, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_awb, camAwb, camera set or get awb mode);

void console_cam_sharp(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_SHARP, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_SHARP, &value);
    }
    printf("cmd: camSharp, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_sharp, camSharp,
                     camera set or get sharpness value);

void console_cam_saturation(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_SATURATION, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_SATURATION, &value);
    }
    printf("cmd: camSaturation, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_saturation, camSaturation,
                     camera set or get saturation value);

void console_cam_contrast(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_CONTRAST, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_CONTRAST, &value);
    }
    printf("cmd: camContrast, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_contrast, camContrast,
                     camera set or get contrast value);

void console_cam_ev(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_EV, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_EV, &value);
    }
    printf("cmd: camEv, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_ev, camEv, camera set or get ev value);

void console_cam_scene(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_SCENE, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_SCENE, &value);
    }
    printf("cmd: camScene, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_scene, camScene, camera set or get scene mode);

void console_cam_fps(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "set") == 0)
    {
        value = atoi(argv[2]);
        ret = cameraSetSettings(CAMERA_FPS, value);
    }
    else
    {
        ret = cameraGetSettings(CAMERA_FPS, &value);
    }
    printf("cmd: camFps, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_fps, camFps, camera set or get fps);

void console_cam_max_fps(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "get") != 0)
    {
        printf("only support get method\r\n");
        ret = -1;
    }
    else
    {
        ret = cameraGetSettings(CAMERA_CMOS_MAX_FPS, &value);
    }
    printf("cmd: camMaxFps, ret: %d, value: %d\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_max_fps, camMaxFps, camera get maxim fps);

void console_cam_resolution(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "get") != 0)
    {
        printf("only support get method\r\n");
        ret = -1;
    }
    else
    {
        ret = cameraGetSettings(CAMERA_RESOLUTION, &value);
    }
    printf("cmd: camResolution, ret: %d, value: %X\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_resolution, camResolution,
                     camera get resolution);

void console_cam_cmos_model(int argc, char **argv)
{
    int ret = 0;
    uint32_t value = 0;
    if(strcmp(argv[1], "get") != 0)
    {
        printf("only support get method\r\n");
        ret = -1;
    }
    else
    {
        ret = cameraGetSettings(CAMERA_CMOS_MODEL, &value);
    }
    printf("cmd: camCmosModel, ret: %d, value: %X\r\n", ret, value);
}
MSH_CMD_EXPORT_ALIAS(console_cam_cmos_model, camCmosModel, camera get cmos id);

char dcim_name_buf[128] = {0};

#ifdef FEATURE_CAMERA_CAPVIEW_ENABLE
extern int cameraCapViewStart(void);
extern int cameraCapViewStop(void);
void console_cam_capview_start(int argc, char **argv) { cameraCapViewStart(); }
MSH_CMD_EXPORT_ALIAS(console_cam_capview_start, camcapviewstart,
                     camera get data and show on LCD);
void console_cam_capview_stop(int argc, char **argv) { cameraCapViewStop(); }
MSH_CMD_EXPORT_ALIAS(console_cam_capview_stop, camcapviewstop,
                     camera stop show on LCD);
#endif

void console_cam_shoot(int argc, char **argv)
{
    time_t curtime = 0;
    struct tm *tmTime = NULL;
    struct timeval tv = {0};
    struct timezone tz = {0};
    int ret = 0;
    curtime = time_time(NULL);
    tmTime = time_localtime(&curtime);
    time_gettimeofday(&tv, &tz);
    snprintf(dcim_name_buf, sizeof(dcim_name_buf),
             "D:/DCIM_%d%02d%02d_%02d%02d%02d_%d.jpg", 1900 + tmTime->tm_year,
             tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour,
             tmTime->tm_min, tmTime->tm_sec, tv.tv_usec / 10000);
    CamImg_t img = {0};
    cameraClearBuf();
    ret = cameraGetBuf(&img, 1000);
    if(ret != 0)
    {
        printf("get image frame data failed\r\n");
        return;
    }
    uint32_t img_width = img.width;
    uint32_t img_height = img.height;
    printf("save yuv start %dx%d \r\n", img_width, img_height);
    OpImg_t *image = NULL;
    if(img.fmt == CAM_IMG_FMT_YUV422)
    {
        image = open_image_create(img_width, img_height, IMG_FMT_YUV422S,
                                  img.addr, img_width * img_height * 2);
        ret = open_image_save("d:/callback.yuv", image, NULL);
    }
    else if (img.fmt == CAM_IMG_FMT_MONO)
    {
        image = open_image_create(img_width, img_height, IMG_FMT_Y,
                                  img.addr, img_width * img_height);
        ret = open_image_save("d:/callback.y", image, NULL);
    }
    
    if(ret == 0)
    {
        SYSLOG_INFO("save yuv success\r\n");
    }
    else
    {
        SYSLOG_INFO("save yuv failed, ret: %d\r\n", ret);
    }
    ret = open_image_save((char *)dcim_name_buf, image, NULL);
    if(ret == 0)
    {
        SYSLOG_INFO("save jpeg file: %s\r\n", dcim_name_buf);
    }
    else
    {
        SYSLOG_INFO("save jpeg failed\r\n");
    }
    cameraReleaseBuf(&img);
    open_image_destroy(image);
    osSemaphoreDelete(semGetFrame);
}
MSH_CMD_EXPORT_ALIAS(console_cam_shoot, camshoot, camera take a phote);

void scan_data_frame_callback(ImageData_t *img, void *param)
{
    // memcpy(snapshot_img_buf, img->data, img->size);
    // snapshot_width = img->width;
    // snapshot_height = img->height;
    osSemaphoreRelease(semGetFrame);
}
#if USE_DECODE_LIB
void console_cam_scan(int argc, char **argv)
{
    time_t curtime = 0;
    struct tm *tmTime = NULL;
    struct timeval tv = {0};
    struct timezone tz = {0};
    int ret = 0;
    semGetFrame = osSemaphoreNew(1, 0, NULL);
    cameraGetFrame(scan_data_frame_callback, NULL);
    if(osSemaphoreAcquire(semGetFrame, 1000 * 1000) != osOK)
    {
        printf("get image frame data failed\r\n");
        osSemaphoreDelete(semGetFrame);
        return;
    }
    else
        printf("get image frame data OK\r\n");

    osSemaphoreDelete(semGetFrame);
}
MSH_CMD_EXPORT_ALIAS(console_cam_scan, camscan, camera take a phote);

void console_cam_scanmode(int argc, char **argv)
{
    time_t curtime = 0;
    struct tm *tmTime = NULL;
    struct timeval tv = {0};
    struct timezone tz = {0};
    int ret = 0;

    for(;;)
    {
        semGetFrame = osSemaphoreNew(1, 0, NULL);
        cameraGetFrame(scan_data_frame_callback, NULL);
        if(osSemaphoreAcquire(semGetFrame, 1000 * 1000) != osOK)
        {
            printf("get image frame data failed\r\n");
            osSemaphoreDelete(semGetFrame);
            return;
        }
        else
            printf("get image frame data OK\r\n");

        osSemaphoreDelete(semGetFrame);
        osDelay(3);
    }
}
MSH_CMD_EXPORT_ALIAS(console_cam_scanmode, scanmode, camera take a phote);

static osSemaphoreId_t gSemaphoreScanDecode = NULL;

static void camDataIrqFunc(uint32_t stats, void *param)
{
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, camDataIrqFunc, P_DEBUG, "stats=%d", stats);
    osSemaphoreRelease(gSemaphoreScanDecode);
}

void console_cam_scanDecode(int argc, char **argv)
{
    int32_t resVal = -1;
    uint8_t buffer[256] = {0};
    uint32_t size = sizeof(buffer);
    CamCbCfg_t camCbCfg = {.cb_data_irq = camDataIrqFunc};

    extern int32_t sub_cam_register_dma_callback(CamCbCfg_t * camCbCfg);
    sub_cam_register_dma_callback(&camCbCfg);
    gSemaphoreScanDecode = osSemaphoreNew(1, 0, NULL);

    while(1)
    {
        size = sizeof(buffer);
        memset(buffer, 0, sizeof(buffer));
        extern int32_t scanAndDecode(uint8_t * buffer, uint32_t * size,
                                     uint32_t timeout);
        scanAndDecode(buffer, &size, osWaitForever);
        resVal = osSemaphoreAcquire(gSemaphoreScanDecode, osWaitForever);
        if(resVal != osOK)
        {
            ECPLAT_PRINTF(UNILOG_OPEN_HAL, console_cam_scanDecode, P_DEBUG,
                          "resVal=%d", resVal);
        }
    }
}
MSH_CMD_EXPORT_ALIAS(console_cam_scanDecode, scandecode,
                     camera take a phote and decode);
#endif
#endif

#endif

#endif