/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_tp.h
 * Description:  ec7xx openHAL TP entry header file
 * History:      Rev1.0   2024-08-29
 *
 ****************************************************************************/
#ifndef _API_TP_H_
#define _API_TP_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"

#define EC_TP_INDEX_START      (0)
#define EC_TP_INDEX_LIMIT      (2)

typedef enum {
    OPEN_TP_USED_FT6336 = 0, 
    OPEN_TP_USED_AXS15231 , 
    OPEN_TP_SUPPORT_NUM ,
} api_tp_ioctl_t;

typedef enum {
    HAL_TP_FT6336 = 0,
    HAL_TP_AXS15231 ,
    HAL_TP_ST77922 ,
    HAL_TP_TOTAL
} tpSupportList_e;

typedef struct {
    uint32_t i2cId;
    uint32_t PadId;
    uint8_t PadMux;
    uint32_t GpioId;
    IsrFunc gpioISR;
    int (*scan)(uint32_t,int16_t*);
}api_tp_inf, *api_tp_infp;

typedef struct {
    uint32_t tpUsrId;
    uint32_t i2cUsrId;
    uint32_t PadUsrId;
    uint32_t GpioUsrId;
}api_tp_UsrId_t;

int api_tp_startup(void* para);
api_tp_infp api_tp_default(tpSupportList_e type);
uint32_t api_tp_create(uint32_t index,void *cfg);
api_ret_t api_tp_delete(uint32_t usrId);
api_ret_t api_tp_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_tp_close(uint32_t usrId);
api_ret_t api_tp_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_tp_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_tp_ioctl(uint32_t usrId,api_tp_ioctl_t type, void *para);
api_ret_t api_tp_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
int api_test_tp(void);

#ifdef __cplusplus
}
#endif
#endif /* _API_TP_H_ */