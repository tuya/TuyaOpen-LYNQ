#ifndef _API_WAKEUP_H_
#define _API_WAKEUP_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "api_def.h"
#define EC_WAKEUP_INDEX_START (0)
#define EC_WAKEUP_INDEX_LIMIT (6)

enum
{
    WAKEUP_PIN_0 = 0,
    WAKEUP_PIN_1,
    WAKEUP_PIN_2,
    WAKEUP_PIN_3,
    WAKEUP_PIN_4,
    WAKEUP_PIN_5,
};

typedef struct {
    uint32_t pin        : 6;
    uint32_t pull       : 2;
    uint32_t mode       : 4;
    uint32_t gpio       : 6;    // gpio 编号
} wakeup_config_t;

typedef enum api_wakeup_ioctrl_
{
    OPEN_WAKEUP_IOCTL_CFG = 0,    // 中断配置
    OPEN_WAKEUP_IOCTL_INTERRUPT,  // WAKEUP中断使能
    OPEN_WAKEUP_IOCTL_ISR_CB,     // 输入中断回调
    OPEN_WAKEUP_IOCTL_GET_LEVEL,  // 获取电平
} api_wakeup_ioctl_e;


int api_wakeup_startup(void *para);
uint32_t api_wakeup_create(uint32_t pin, void *config);
api_ret_t api_wakeup_delete(uint32_t usrId);
api_ret_t api_wakeup_open(uint32_t usrId, void *cfg, size_t timeout);
api_ret_t api_wakeup_close(uint32_t usrId);
api_ret_t api_wakeup_ioctl(uint32_t usrId, api_wakeup_ioctl_e type, void *para);
api_ret_t api_wakeup_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count);
api_ret_t api_wakeup_setup(int8_t index, wakeup_config_t* para);
#define CSV_WAKEUP_ITEM_MAX     (4)
int32_t api_wakeup_parse(char* str,wakeup_config_t *cfg);
#ifdef __cplusplus
}
#endif
#endif /* _API_WAKEUP_H_ */