/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_i2c.h
 * Description:  ec7xx openhal i2c entry header file
 * History:      Rev1.0   2025-08-15
 *
 ****************************************************************************/
#ifndef API_I2C_H
#define API_I2C_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "api_def.h"
#include "sctdef.h"

#define EC_I2C_INDEX_START (0)
#define EC_I2C_INDEX_LIMIT (2)
typedef struct {
    uint32_t sda        : 6;    // paddr < 64
    uint32_t scl        : 6;    // paddr < 64
    uint32_t clk        : 3;    // 时钟等级梯度化
    uint32_t poll       : 1;    // 轮询模式
    uint32_t mode       : 4;
    // 非配置信息
    uint32_t port       : 2;
} i2c_config_t;

typedef struct {
    int8_t sda_port;
    int8_t sda_pin;
    uint16_t sda_mask;
    int8_t scl_port;
    int8_t scl_pin;
    uint16_t scl_mask;
    int32_t speed;
} ec_i2c_cfg_t; // 主要用于GPIO模拟I2C功能的相关参数传递

typedef struct {
    ec_i2c_cfg_t cfg;
    uint8_t (*CheckAddress)     (ec_i2c_cfg_t*, uint8_t);   
    // 以下函数命名和功能兼容cmsis接口
    int32_t (*Initialize)       (void*);
    int32_t (*Uninitialize)     (void);
    int32_t (*PowerControl)     (uint8_t);
    int32_t (*MasterTransmit)   (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending);
    int32_t (*MasterReceive)    (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending);
    int32_t (*Control)          (uint32_t control, uint32_t arg);
} ec_i2c_bus_t;

typedef enum I2cIoctl_
{
    OPEN_I2C_IOCTL_ISR_CB = 0,  // 输入中断回调
    OPEN_I2C_IOCTL_SPEED,
    OPEN_I2C_IOCTL_SLAVE_ADDR,
} I2cIoctl_e;

typedef enum I2cSpeed_
{
    OPEN_I2C_SPEED_100KHZ = 1,
    OPEN_I2C_SPEED_400KHZ,
    OPEN_I2C_SPEED_1000KHZ,
    OPEN_I2C_SPEED_3400KHZ,
} I2cSpeed_e;
typedef struct {
    uint8_t   addr;         // Address
    uint8_t   reg;          // register
    uint8_t *data;          // data to transfer
    uint32_t  num;          // Number of bytes to transfer
} api_i2c_master_t;

typedef struct I2cCfg_
{
    uint8_t slave_addr;
    I2cSpeed_e speed;  
} I2cCfg_t;

int8_t *api_i2c_startup(void *para, int8_t *pad);
api_ret_t api_i2c_setup(int8_t index, i2c_config_t* para);
// create 接口用于确认资源是否存在,不会实际配置硬件
api_ret_t api_i2c_create(uint32_t index, I2cCfg_t *cfg, void *out);
api_ret_t api_i2c_delete(uint32_t usrId);
api_ret_t api_i2c_open(uint32_t usrId, I2cCfg_t *cfg, size_t timeout);
api_ret_t api_i2c_close(uint32_t usrId);
api_ret_t api_i2c_write(uint32_t usrId, void *buf, uint32_t count, int xfer_pending);
api_ret_t api_i2c_read(uint32_t usrId, void *buf, uint32_t count, int xfer_pending);
api_ret_t api_i2c_ioctl(uint32_t usrId, I2cIoctl_e type, void *para);
api_ret_t api_i2c_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count);
uint32_t i2c_fault_wait(uint32_t usrId, size_t timeout);
api_ret_t api_i2c_query(uint32_t usrId);
int api_test_i2c(void);

#define CSV_I2C_ITEM_MAX    (6)
int32_t api_i2c_parse(char* str, i2c_config_t *cfg);

#ifdef __cplusplus
}
#endif
#endif /* API_I2C_H */