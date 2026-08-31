/****************************************************************************
 *
 * Copy right:   2025-, Copyrigths of EigenComm Ltd.
 * File name:    dev_comm.c
 * Description:  ec7xx openhal entry source file
 * History:      Rev1.0   2025-09-05
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h> /* memset */
#include <stdlib.h> /* atoi */

#include "Driver_Common.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "dev_comm.h"
#include "api_tp.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif
/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief        
  \return 
*/
api_ret_t open_dev_startup(devType_t type,void* para)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            api_tp_startup(para);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            api_scr_startup(para);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            api_cam_startup(para);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return 
*/
api_ret_t open_dev_query(devType_t type, uint32_t index)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_query(index);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_query(index);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_query(index);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_dev_create(devType_t type, uint32_t index, void *cfg, uint32_t *usrId)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_create(index, cfg, usrId);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_create(index, cfg, usrId);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_create(index, cfg, usrId);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/**
  \fn          
  \brief
  \return
*/
api_ret_t open_dev_delete(devType_t type, uint32_t usrId)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_delete(usrId);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_delete(usrId);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_delete(usrId);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_dev_open(devType_t type, uint32_t usrId, void *cfg, size_t timeout)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_open(usrId, cfg, timeout);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_open(usrId, cfg, timeout);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_open(usrId, cfg, timeout);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_dev_close(devType_t type, uint32_t usrId)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_close(usrId);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_close(usrId);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_close(usrId);
            break;
        #endif
        default:
            break;
    }
    return ret;
}


/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_dev_ioctl(devType_t type, uint32_t usrId, uint32_t cmd, void *para)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_ioctl(usrId, cmd, para);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_ioctl(usrId, cmd, para);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_ioctl(usrId, cmd, para);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
api_ret_t open_dev_pmctl(devType_t type, uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_pmctl(usrId, cfg, count);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_pmctl(usrId, cfg, count);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_pmctl(usrId, cfg, count);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

api_ret_t open_dev_write(devType_t type, uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_write(usrId, buf, count);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_write(usrId, buf, count);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_write(usrId, buf, count);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

api_ret_t open_dev_read(devType_t type, uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            ret = api_tp_read(usrId, buf, count);
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            ret = api_scr_read(usrId, buf, count);
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            ret = api_cam_read(usrId, buf, count);
            break;
        #endif
        default:
            break;
    }
    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
/**
  \fn          
  \brief    
  \return
*/
api_ret_t open_dev_test(devType_t type)
{ 
    api_ret_t ret = OPEN_HAL_DONE;
    switch (type)
    {
        case DEV_TP:
            // ret = api_test_tp();
            break;
        #ifdef FEATURE_HAL_SCREEN_ENABLE
        case DEV_SCR:
            // api_test_scr();
            break;
        #endif
        #ifdef FEATURE_HAL_CAM_ENABLE
        case DEV_CAM:
            // api_test_cam();
            break;
        #endif
        default:
            break;
    }
    return ret;
}