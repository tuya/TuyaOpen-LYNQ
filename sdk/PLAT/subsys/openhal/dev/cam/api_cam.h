/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_cam.h
 * Description:  ec7xx openHAL CAM entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#ifndef _API_CAM_H_
#define _API_CAM_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"

typedef void ( *cam_isr_t)(void);

typedef enum {
    OPEN_CAM_IOCTL_DUMMY = 0,       /* dummy */
    OPEN_CAM_IOCTL_START,         /* start */
    OPEN_CAM_IOCTL_STOP,           /* stop */
    OPEN_CAM_IOCTL_GET_MODE,        /* get mode */
    OPEN_CAM_IOCTL_GET_BUSY,        /* get busy */
    OPEN_CAM_IOCTL_SET_RESOLUTION,  /* set resolution */
    OPEN_CAM_IOCTL_GET_RESOLUTION,  /* get resolution */
    OPEN_CAM_IOCTL_SET_FRAMERATE,    /* set framerate */
    OPEN_CAM_IOCTL_GET_FRAMERATE,    /* get framerate */
    OPEN_CAM_IOCTL_SET_BRIGHTNESS,   /* set brightness */
    OPEN_CAM_IOCTL_SET_CONTRAST,     /* set contrast */
    OPEN_CAM_IOCTL_SET_SATURATION,   /* set saturation */
    OPEN_CAM_IOCTL_SET_SHARPNESS,    /* set sharpness */
    OPEN_CAM_IOCTL_SET_WB, /* set white balance */
    OPEN_CAM_IOCTL_SET_EXPOSURE,     /* set exposure */
    OPEN_CAM_IOCTL_SET_AE,           /* set ae */
    OPEN_CAM_IOCTL_SET_AWB,          /* set awb */
    OPEN_CAM_IOCTL_SET_MIRROR,       /* set mirror */
    OPEN_CAM_IOCTL_GET_MIRROR,      /* get mirror */
    OPEN_CAM_IOCTL_SET_FLIP,         /* set flip */
    OPEN_CAM_IOCTL_GET_FLIP,        /* get flip */
    OPEN_CAM_IOCTL_SET_ROTATE,       /* set rotate */
    OPEN_CAM_IOCTL_GET_ROTATE,      /* get rotate */
    OPEN_CAM_IOCTL_SET_COLOR_MODE,   /* set color mode*/
    OPEN_CAM_IOCTL_GET_COLOR_MODE,   /* get color mode*/
    OPEN_CAM_IOCTL_START_PREVIEW,   /* start preview */
    OPEN_CAM_IOCTL_STOP_PREVIEW,    /* stop preview */
    OPEN_CAM_IOCTL_REC_START,        /* start rec*/
    OPEN_CAM_IOCTL_REC_STOP,         /* stop rec */
    OPEN_CAM_IOCTL_REC_FILE_FORMAT,  /* rec file format */
    OPEN_CAM_IOCTL_SHOOT_START,      /* start shoot */
    OPEN_CAM_IOCTL_SHOOT_STOP,       /* stop shoot */
    OPEN_CAM_IOCTL_SHOOT_FILE_FORMAT,/* shoot file format */
    OPEN_CAM_IOCTL_MAX,
} api_cam_ioctl_t;

int api_cam_startup(void* para);
uint32_t api_cam_default(char* str,void *cfg);
uint32_t api_cam_apply(char* str,void *cfg);
api_ret_t api_cam_delete(uint32_t usrId);
api_ret_t api_cam_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_cam_close(uint32_t usrId);
api_ret_t api_cam_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_cam_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_cam_ioctl(uint32_t usrId,api_cam_ioctl_t type, void *para);
api_ret_t api_cam_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);

#ifdef __cplusplus
}
#endif
#endif /* _API_CAM_H_ */