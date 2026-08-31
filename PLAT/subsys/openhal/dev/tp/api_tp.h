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

#define EC_TP_INDEX_START (0)
#define EC_TP_INDEX_LIMIT (4)
#define U8TO16(x1, x2) ((((x1) & 0xFF) << 8) | ((x2) & 0xFF))
#define U8TO32(x1, x2, x3, x4)                                              \
    ((((x1) & 0xFF) << 24) | (((x2) & 0xFF) << 16) | (((x3) & 0xFF) << 8) | \
     ((x4) & 0xFF))

typedef enum
{
    OPEN_TP_USED_FT6336 = 0,
    OPEN_TP_USED_AXS15231,
    OPEN_TP_USED_CST816,
    OPEN_TP_SUPPORT_NUM,
} api_tp_ioctl_t;

typedef enum
{
    HAL_TP_FT6336 = 0,
    HAL_TP_AXS15231,
    HAL_TP_ST77922,
    HAL_TP_CST816,
    HAL_TP_TOTAL
} tpSupportList_e;

typedef struct TsFunc_
{
    void (*tp_reset)(void);
    int (*tp_report)(void);
    int (*tp_supend)(void);
    int (*tp_resum)(void);
    int (*tp_updata_fw)(uint8_t *bin_addr, uint16_t len);
    int (*tp_set_workmode)(int mode, uint8_t enable);
    uint32_t (*tp_check_esd)(void);
    int (*tp_prox_handle)(uint8_t cmd);
    int (*tp_get_dbg_data)(uint8_t *buf, uint16_t len);
    int (*tp_get_test_result)(uint8_t *buf, uint16_t len);
    int (*tp_dev_init)(uint8_t port_id, uint32_t speed,
                       void (*tp_bus_cb_func)(uint32_t event));
    int (*tp_scan)(uint32_t usrId, int16_t *pos);
} TsFunc_t;

typedef struct TpData_
{
    uint8_t pt_num;
    int16_t pox_x;
    int16_t pox_y;
} TpData_t;

typedef struct
{
    uint32_t i2cId;
    uint32_t type;
    uint32_t PadId;
    uint8_t PadMux;
    uint32_t GpioId;
    IsrFunc gpioISR;
    int (*scan)(uint32_t, int16_t *);
    const TsFunc_t *devFunc;
    uint8_t fw_id;
    uint8_t reverse_x;
    uint8_t reverse_y;
    uint16_t origin_width;
    uint16_t origin_height;
    uint16_t output_width;
    uint16_t output_height;
} api_tp_inf, *api_tp_infp;

int api_tp_startup(void *para);
api_tp_infp api_tp_default(tpSupportList_e type);
api_ret_t api_tp_create(uint32_t index, api_tp_infp cfg, uint32_t* usrId);
api_ret_t api_tp_delete(uint32_t usrId);
api_ret_t api_tp_open(uint32_t usrId, api_tp_infp cfg, uint32_t timeout);
api_ret_t api_tp_close(uint32_t usrId);
api_ret_t api_tp_ioctl(uint32_t usrId, api_tp_ioctl_t type, void *para);
api_ret_t api_tp_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count);
api_ret_t api_tp_read(uint32_t usrId, void *buf, size_t count);
api_ret_t api_tp_write(uint32_t usrId, void *buf, size_t count);
api_ret_t api_tp_query(uint32_t usrId);
#ifdef __cplusplus
}
#endif
#endif /* _API_TP_H_ */