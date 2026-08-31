/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_dma.h
 * Description:  ec7xx openhal dma entry header file
 * History:      Rev1.0   2024-02-23
 *
 ****************************************************************************/
#ifndef  API_DMA_H_
#define  API_DMA_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#define EC_DMA_INDEX_START     (0)
#define EC_DMA_INDEX_LIMIT     (16)

typedef enum {
    OPEN_DMA_IOCTL_DUMMY = 0,
    OPEN_DMA_IOCTL_CONFIG,
    OPEN_DMA_IOCTL_MAX,
} api_dma_ioctl_t;

api_ret_t api_dma_query(uint32_t usrId);
int api_dma_startup(void* para);
uint32_t api_dma_create(uint32_t index,void *cfg);
api_ret_t api_dma_delete(uint32_t usrId);
api_ret_t api_dma_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_dma_close(uint32_t usrId);
api_ret_t api_dma_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_dma_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_dma_ioctl(uint32_t usrId,api_dma_ioctl_t type, uint32_t para);
api_ret_t api_dma_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
int api_test_dma(void);
#ifdef __cplusplus
}
#endif
#endif /* API_DMA_H_ */