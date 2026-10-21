/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of EigenComm Ltd.
 * File name:    test.h
 * Description:  EC618 mqtt demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef __CAMERA_H__
#define __CAMERA_H__


#include "cmsis_os2.h"
#include "subsys.h"

typedef enum {
    CAMERA_PIC_MODE,
    CAMERA_REC_MODE,
} camera_mode_t;

typedef enum {
    CAMERA_STATUS_IDLE,
    CAMERA_STATUS_STARTING,
    CAMERA_STATUS_STARTED,
    CAMERA_STATUS_STOPPING,
    CAMERA_STATUS_STOPPED,
    CAMERA_STATUS_BUSY,
} camera_status_t;

typedef enum {
    CAMERA_ERR_NONE,
} camera_err_t;

typedef enum {
    CAMERA_FPS,
    CAMERA_RESOLUTION,
    CAMERA_MIRROR,
    CAMERA_FLIP,
    CAMERA_ROTATE,
} camera_setting_t;

typedef enum {
    CAMERA_NAME,
    CAMERA_MAX_RESOLUTION,
} camera_hw_info_t;

void subCameraInit(void);
void cameraStart(void);
void cameraStop(void);

#endif
