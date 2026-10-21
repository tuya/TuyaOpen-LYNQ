/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_i2c.h
 * Description:  ec7xx openhal i2c entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#ifndef  API_I2C_H
#define  API_I2C_H
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#include "sctdef.h"
#define EC_I2C_INDEX_START      (0)
#define EC_I2C_INDEX_LIMIT      (2)

typedef enum {
    OPEN_I2C_IOCTL_ISR_CB = 0,     //输入中断回调
    OPEN_I2C_IOCTL_SPEED , 
    OPEN_I2C_IOCTL_SLAVE_ADDR ,            
} api_i2c_ioctl_t;

typedef enum {
    EVT_I2C_COMPLETED = 0,
    EVT_I2C_INCOMPLETE ,
    EVT_I2C_SLAVE_NACK ,
    EVT_I2C_BUS_ERROR ,
    EVT_I2C_ARBI_LOST ,
    EVT_I2C_TOTAL ,
} api_i2c_flags_e;

typedef struct {
    uint8_t   addr;         // Address
    uint8_t   reg;          // register
    uint8_t *data;          // data to transfer
    uint32_t  num;          // Number of bytes to transfer
} api_i2c_master_t;

typedef struct
{
    uint8_t (*i2c_init)(uint8_t);
    uint8_t (*i2c_scan)(uint8_t*,uint8_t);
    uint8_t (*i2c_wait)(uint8_t,uint8_t*,uint16_t,uint32_t);
    uint8_t (*i2c_read)(uint8_t,uint8_t,uint16_t,uint8_t*);
    uint8_t (*i2c_send)(uint8_t,uint8_t,uint16_t,uint8_t*);
}i2c_func;

int api_i2c_startup(void* para);
uint32_t api_i2c_create(uint32_t index,void *cfg);
api_ret_t api_i2c_delete(uint32_t usrId);
api_ret_t api_i2c_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_i2c_close(uint32_t usrId);
api_ret_t api_i2c_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_i2c_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_i2c_ioctl(uint32_t usrId,api_i2c_ioctl_t type, void *para);
api_ret_t api_i2c_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
uint32_t i2c_fault_wait(uint32_t usrId,size_t timeout);
int api_test_i2c(void);
#ifdef __cplusplus
}
#endif
#endif /* API_I2C_H */