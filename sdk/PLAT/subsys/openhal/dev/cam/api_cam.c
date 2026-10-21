/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_cam.c
 * Description:  ec7xx openHAL Cam entry source file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_Common.h"
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "api_comm.h"
#include "api_cam.h"

//数量上限
#define EC_DEV_CAM_INDEX_LIMIT      (1)

// #ifdef BSP_MINIDKB_1V1
//此表后续会移至DDK FLASH存储区域
static uint8_t CamStartupList[EC_DEV_CAM_INDEX_LIMIT] = {
    0x0,
};
// #endif

static api_ret_t api_cam_query(uint32_t usrId)
{
    return 0;
}

static uint32_t usrId_to_index(uint32_t usrId)
{ 
    return 0;
}

int api_cam_startup(void* para)
{
    for(int i=0;i<EC_DEV_CAM_INDEX_LIMIT;i++)
    {

    }
}

uint32_t api_cam_default(char* str,void *cfg)
{
    return 0;
}
uint32_t api_cam_apply(char* str,void *cfg)
{
    return 0;
}

api_ret_t api_cam_delete(uint32_t usrId)
{
    return 0;
}
api_ret_t api_cam_open(uint32_t usrId,void *cfg,size_t timeout)
{
    return 0;
}
api_ret_t api_cam_close(uint32_t usrId)
{
    return 0;
}
api_ret_t api_cam_write(uint32_t usrId, void* buf, size_t count)
{
    return 0;
}
api_ret_t api_cam_read(uint32_t usrId, void* buf, size_t count)
{
    return 0;
}
api_ret_t api_cam_ioctl(uint32_t usrId,api_cam_ioctl_t type, void *para)
{
    api_ret_t ret = api_cam_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        switch (type) {
            case OPEN_CAM_IOCTL_START_PREVIEW:
                break;
            case OPEN_CAM_IOCTL_STOP_PREVIEW:
                break;
            case OPEN_CAM_IOCTL_REC_START:
                break;
            case OPEN_CAM_IOCTL_REC_STOP:
                break;
            case OPEN_CAM_IOCTL_SHOOT_START:
                break;
            case OPEN_CAM_IOCTL_SHOOT_STOP:
                break;
            case OPEN_CAM_IOCTL_SHOOT_FILE_FORMAT:
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
end:
    return ret;
}
api_ret_t api_cam_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count)
{
    return 0;
}