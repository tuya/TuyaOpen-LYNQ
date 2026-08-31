/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_spi.h
 * Description:  ec7xx openhal spi entry header file
 * History:      Rev1.0   2025-09-18
 *
 ****************************************************************************/
#ifndef  API_SPI_H_
#define  API_SPI_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#define EC_SPI_INDEX_START     (0)
#define EC_SPI_INDEX_LIMIT     (2)
typedef struct {
    uint32_t cs         : 6;    // paddr < 54
    uint32_t mosi       : 5;    // paddr < 32
    uint32_t miso       : 5;    // paddr < 32
    uint32_t sclk       : 5;    // paddr < 32
    uint32_t freq       : 5;    // 时钟频率枚举
    uint32_t poll       : 2;    // 传输模式枚举
    uint32_t mode       : 4;    // 运行模式枚举
} spi_config_t;

typedef enum {
    OPEN_SPI_IOCTL_CLK = 0,
    OPEN_SPI_IOCTL_MODE,
    OPEN_SPI_IOCTL_CONFIG,
    OPEN_SPI_IOCTL_START_SEND,
    OPEN_SPI_IOCTL_STOP_SEND,
} api_spi_ioctl_t;

api_ret_t api_spi_checkout(int8_t mosi, int8_t miso, int8_t sclk);
api_ret_t api_spi_query(uint32_t usrId);
int8_t *api_spi_startup(void* para, int8_t *pad);
api_ret_t api_spi_setup(int8_t index, spi_config_t* para);
api_ret_t api_spi_create(uint32_t index,void *cfg, void *out);
api_ret_t api_spi_delete(uint32_t usrId);
api_ret_t api_spi_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_spi_close(uint32_t usrId);
api_ret_t api_spi_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_spi_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_spi_ioctl(uint32_t usrId,api_spi_ioctl_t type, void *para);
api_ret_t api_spi_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
int api_test_spi(void);

#define CSV_CFG_SPI_ITEMS   (8) // 需要和csv文件标题对应
int32_t api_spi_parse(char* str,spi_config_t *cfg);

#ifdef __cplusplus
}
#endif
#endif /* API_SPI_H_ */