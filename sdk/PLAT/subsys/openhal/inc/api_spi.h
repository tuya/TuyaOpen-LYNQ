/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_spi.h
 * Description:  ec7xx openhal spi entry header file
 * History:      Rev1.0   2024-02-23
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

typedef enum {
    OPEN_SPI_IOCTL_CLK = 0,
    OPEN_SPI_IOCTL_MODE,
    OPEN_SPI_IOCTL_CONFIG,
    OPEN_SPI_IOCTL_START_SEND,
    OPEN_SPI_IOCTL_STOP_SEND,
} api_spi_ioctl_t;

api_ret_t api_spi_query(uint32_t usrId);
int api_spi_startup(void* para);
uint32_t api_spi_create(uint32_t index,void *cfg);
api_ret_t api_spi_delete(uint32_t usrId);
api_ret_t api_spi_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_spi_close(uint32_t usrId);
api_ret_t api_spi_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_spi_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_spi_ioctl(uint32_t usrId,api_spi_ioctl_t type, uint32_t para);
api_ret_t api_spi_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
int api_test_spi(void);
#ifdef __cplusplus
}
#endif
#endif /* API_SPI_H_ */