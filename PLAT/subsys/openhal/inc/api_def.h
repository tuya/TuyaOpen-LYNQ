/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_btn.h
 * Description:  ec7xx openAPI  entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#ifndef  API_DEF_H
#define  API_DEF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "devicemanager.h"

#define OPEN_HAL_STAT_UNUSED    (0xFFFF0000)
#define OPEN_HAL_STAT_MUSK      (0x0000FF00)
#define OPEN_HAL_PORT_MUSK      (0x000000FF)

typedef void (*IsrFunc)(uint32_t);

typedef enum
{
    OPEN_HAL_DONE   = 0,    //执行成功
    OPEN_HAL_LOCK   ,       //已锁定不释放
    OPEN_HAL_NONE   ,       //没有查到对应设备
    OPEN_HAL_FREE   ,       //未初始化
    OPEN_HAL_IDLE   ,       //初始化，未使用
    OPEN_HAL_USED   ,       //初始化，已使用
    OPEN_HAL_INVALID_PARA,  //参数错误
    OPEN_HAL_TIMEOUT,       //超时
    OPEN_HAL_MAX            
} api_ret_t;

typedef struct
{
    PmMode_e mode;
    RuntimeMode_e runtime;
} open_hal_pm_t;

typedef struct {
    int32_t (*query)(uint32_t);
    int32_t (*open)(uint32_t,void*,size_t);
    int32_t (*close)(uint32_t,void*,size_t);
    int32_t (*pmctl)(uint32_t,void*,size_t);
    int32_t (*ioctl)(uint32_t,void*,size_t);
    int32_t (*read)(uint32_t,void*,size_t);
    int32_t (*write)(uint32_t,void*,size_t);
}api_register_inf, *api_register_fp;

#define GET_MUX_FROM_REG(addr)  (((*((volatile uint8_t*)(addr)) >> 4) & 0x7U))

#ifdef __cplusplus
}
#endif
#endif /* API_DEF_H */