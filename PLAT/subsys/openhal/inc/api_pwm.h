/****************************************************************************
 *
 * Copy right:   2025-, Copyrigths of EigenComm Ltd.
 * File name:    api_pwm.h
 * Description:  openhal pwm entry header file
 * History:      Rev1.0   2025-09-15
 *
 ****************************************************************************/
#ifndef _API_PWM_H_
#define _API_PWM_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#define EC_PWM_INDEX_START      (0)
#define EC_PWM_INDEX_LIMIT      (5)

typedef enum {
    OPEN_PWM_IOCTL_ISR = 0,     // 配置硬件中断回调
    OPEN_PWM_IOCTL_FREQ ,       // 配置硬件周期频率
    OPEN_PWM_IOCTL_ALL
} api_pwm_ioctl_t;

typedef struct {
    uint32_t pin        : 6;
    uint32_t act        : 1;
    uint32_t duty       : 7;
    uint32_t mode       : 4;
    uint32_t freq       : 4;
    uint32_t port       : 3;    // 指定TIMER index校验
    uint32_t gpio       : 6;    // 外部输入的gpio编号
} pwm_config_t;


int8_t *api_pwm_startup(void* para, int8_t *pad);
api_ret_t api_pwm_create(int8_t index, pwm_config_t *para, void *out);  // 创建PWM，设备可发现
api_ret_t api_pwm_setup(int8_t index, pwm_config_t* para);  // 实际执行硬件配置
api_ret_t api_pwm_delete(uint32_t usrId);
api_ret_t api_pwm_open(uint32_t usrId, pwm_config_t *cfg, size_t timeout);
api_ret_t api_pwm_close(uint32_t usrId);
api_ret_t api_pwm_ioctl(uint32_t usrId,api_pwm_ioctl_t type, void *para);
api_ret_t api_pwm_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
api_ret_t api_pwm_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_pwm_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_pwm_query(uint32_t usrId);

int api_test_pwm(void);

#define CSV_CFG_PWM_ITEMS   (6)
int32_t api_pwm_parse(char* str, pwm_config_t *cfg);
#ifdef __cplusplus
}
#endif
#endif /* _API_PWM_H_ */