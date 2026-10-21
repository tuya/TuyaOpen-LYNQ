/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_scr.h
 * Description:  ec7xx openHAL screen entry header file
 * History:      Rev1.0   2024-02-02
 *
 ****************************************************************************/
#ifndef _API_SCR_H_
#define _API_SCR_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include "ec7xx.h"
#include "bsp.h"
#include "lspi.h"
#include "api_def.h"

#define EC_SCR_INDEX_START      (0)
#define EC_SCR_INDEX_LIMIT      (2)

typedef enum {
    SCREEN_BACKLIGHT = 0, 
    SCREEN_DIRECTION ,
    SCREEN_SUPPORT_NUM ,
} api_scr_ioctl_t;

typedef enum {
    HAL_SCR_ST7789 = 0,
    HAL_SCR_AXS15231 ,
    HAL_SCR_ST77922 ,
    HAL_SCR_TOTAL
} screenSupportList_e;

typedef enum {
    OS_DELAY = 0,
    HAL_DELAY,
    LCD_CMD_WRITE,
    LCD_CMD_CONTINUE,
    INF_COLOROUT = 0x0A,
    INF_BUS_MCLK = 0x0C,
    INF_BUS_TYPE = 0x0E,
    INF_DATALANE = 0x0F,
}lcdConfig_e;

typedef enum {
    INF_SPI_3W_I,
    INF_SPI_3W_II,
    INF_SPI_4W_I,
    INF_SPI_4W_II,
}lcdInterfaceType_e;

typedef struct {
    uint16_t sx;
    uint16_t sy;
    uint16_t ex;
    uint16_t ey;
    void * data;
} api_screen_fill_t;

typedef struct dma_data_t {
    DmaTransferConfig_t dmaTrans;
    struct dma_data_t *next;
    uint32_t totalBytes;
    uint32_t dataLen;   //LSPI Trans <0x3FFFF
    uint16_t cmds[16];
    void *data;
} dma_data_t;

typedef struct {
    uint8_t clockDiv;
    IsrFunc dmaISR;
    IsrFunc spiISR;
    uint32_t *spiCtrl;
    uint32_t *intCtrl;
    uint32_t *dataFmt;
    uint32_t (*fill)(uint16_t,uint16_t,uint16_t,uint16_t,void*,uint32_t);
}api_scr_inf, *api_scr_infp;

int api_scr_startup(void* para);
api_scr_infp api_scr_default(screenSupportList_e type);
uint32_t api_scr_create(screenSupportList_e type,void *cfg);
api_ret_t api_scr_delete(uint32_t usrId);
api_ret_t api_scr_open(uint32_t usrId,void *cfg,size_t timeout);
api_ret_t api_scr_close(uint32_t usrId);
api_ret_t api_scr_read(uint32_t usrId, void* buf, size_t count);
api_ret_t api_scr_write(uint32_t usrId, void* buf, size_t count);
api_ret_t api_scr_ioctl(uint32_t usrId,api_scr_ioctl_t type, void *para);
api_ret_t api_scr_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
#ifdef __cplusplus
}
#endif
#endif /* _API_SCR_H_ */