#ifndef _DEV_COMM_H_
#define _DEV_COMM_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
	DEV_TP			= 19,
	DEV_SCR			= 20,
	DEV_CAM			= 21,
	DEV_MAX ,
} devType_t;

#include <stdint.h>
#include "api_def.h"
#include "api_btn.h"
#ifdef FEATURE_HAL_SCREEN_ENABLE
#include "api_scr.h"
#endif
#ifdef FEATURE_HAL_CAM_ENABLE
#include "api_cam.h"
#endif

api_ret_t open_dev_startup(devType_t type,void* para);
api_ret_t open_dev_query(devType_t type, uint32_t index);
api_ret_t open_dev_create(devType_t type, uint32_t index,void *cfg, uint32_t *usrId);
api_ret_t open_dev_delete(devType_t type, uint32_t usrId);
api_ret_t open_dev_open(devType_t type, uint32_t usrId, void *cfg, size_t timeout);
api_ret_t open_dev_close(devType_t type, uint32_t usrId);
api_ret_t open_dev_ioctl(devType_t type, uint32_t usrId, uint32_t cmd, void *para);
api_ret_t open_dev_pmctl(devType_t type, uint32_t usrId, open_hal_pm_t *cfg, size_t count);
api_ret_t open_dev_write(devType_t type, uint32_t usrId, void* buf, size_t count);
api_ret_t open_dev_read(devType_t type, uint32_t usrId, void* buf, size_t count);
api_ret_t open_dev_test(devType_t type);

#ifdef __cplusplus
}
#endif
#endif /* _DEV_COMM_H_ */