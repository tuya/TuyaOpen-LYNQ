/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_scr.c
 * Description:  ec7xx openHAL screen entry source file
 * History:      Rev1.0   2024-02-02
 *
 ****************************************************************************/
#ifdef FEATURE_HAL_SCREEN_ENABLE
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "api_comm.h"
#include "api_scr.h"

#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "lspi.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "timer.h"
#define LCD_REG_STAS_MASK           (0x00000004)

#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

#include "lcdDrv.h"
lcdDrvFunc_t* lcdDev = NULL;

extern lspiRes_t *lspiResGet(void);

static uint32_t s_scrUsrIdList[EC_SCR_INDEX_LIMIT] = {0};
static uint16_t s_scrUsrIdSeed[EC_SCR_INDEX_LIMIT] = {0};
static DmaDescriptor_t __ALIGNED(16) descriptor[40];
static osEventFlagsId_t scrEventFlags = NULL;
static osMutexId_t lspiMutex = NULL;
static api_scr_inf screen_device;
static int32_t dispDmaCh = 0;
static dma_data_t dmaData;
/**
  \fn
  \brief
  \return
*/
static IsrFunc user_spi_cb = NULL;
static void spiEventHandler(void)
{
    LSPI2->STAS |= (1<<31);
    uint32_t status = (LSPI2->STAS & ~(LCD_REG_STAS_MASK));
    if(status){
        EPAT_LOG(spiEventHandler, P_ERROR, "%d,0x%X",osKernelGetTickCount(),status);
    }
    else if(user_spi_cb != NULL){
        user_spi_cb(LSPI2->STAS);
    }
}
static IsrFunc user_dma_cb = NULL;
static void dmaEventHandler(uint32_t event)
{
    if(event == DMA_EVENT_END){
        if(user_dma_cb != NULL) user_dma_cb(event);
    }
    else {
        EPAT_LOG(dmaEventHandler, P_ERROR, "0x%X",event);
    }
}
/**
  \fn
  \brief
  \return
*/
static uint32_t usrId_to_index(uint32_t usrId)
{ 
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK); 
    if(index == (s_scrUsrIdList[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_SCR_INDEX_LIMIT;
}
/**
  \fn
  \brief
  \return
*/
static int32_t spiSend(lspiRes_t *lspi,void *dataOut,uint32_t num)
{
    uint8_t     *dataListIndex  = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t     *dataLen        = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t    *tmp            = &(lspi->info->prePareSendInfo.tmp);
    uint32_t    *dataList       = lspi->info->prePareSendInfo.dataList;
    if (*dataLen % 4 != 0){
        dataList[*dataListIndex] = *tmp;
        *dataListIndex += 1;
    }
    for (int i = 0; i < *dataListIndex; i++){
        lspi->reg->TFIFO = dataList[i];
    }
    lspi->reg->LSPI_CCTRL = 1 | *dataLen<<8;
    // wait until finish
    while (!lspi->reg->LSPI_STAT);
    memset(dataList, 0, *dataListIndex * 4);
    lspi->info->prePareSendInfo.dataListIndex = 0;
    lspi->info->prePareSendInfo.tmp = 0;
    lspi->info->prePareSendInfo.trans = 0;
    lspi->info->prePareSendInfo.dataLen = 0;
    return ARM_DRIVER_OK;
}
/**
  \fn
  \brief
  \return
*/
static void spiData(lspiRes_t *lspi,uint8_t data)
{
    uint8_t *dataListIndex  = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t *trans  = &(lspi->info->prePareSendInfo.trans);
    uint8_t *dataLen    = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t    *tmp    = &(lspi->info->prePareSendInfo.tmp);
    uint32_t    *dataList   = lspi->info->prePareSendInfo.dataList;
    *tmp |= data << *trans;
    *dataLen += 1;
    *trans += 8;
    if (*trans == 32) {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex += 1;
        *trans = 0;
        *tmp = 0;
    }
}
/**
  \fn
  \brief
  \return
*/
static int dispConfig(uint16_t *regs)
{
    if(regs[0] == 0xFFFF || (regs[0] & 0xC000) == 0){
        return 0;
    }
    uint16_t num = (regs[0] & 0x3FFF);
    lspiRes_t *lspi = lspiResGet();
    uint8_t value = 0;
    int cmd_len = 0;
    for (uint16_t i = 1; i <= num; i++){
        uint16_t cmd = regs[i];
        switch(((cmd >> 8) & 0xFF)) {
            case OS_DELAY:
                osDelay(cmd & 0xFF);    //ms
                break;
            case HAL_DELAY:
                delay_us(cmd & 0xFF);   //us
                break;
            case LCD_CMD_WRITE:
                if (cmd_len) {
                    spiSend(lspi,NULL,0);
                }else if (i!=0) {
                    spiData(lspi,0);
                    spiSend(lspi,NULL,0);
                }
                lspi->reg->LSPI_CADDR = (uint8_t)(cmd & 0xFF);
                cmd_len = 0;
                break;
            case LCD_CMD_CONTINUE:
                spiData(lspi,(cmd & 0xFF));
                cmd_len++;
                break;
            case INF_COLOROUT:
                lspiCtrl.colorModeOut = (uint8_t)(cmd & 0xFF);
                lspi->reg->LSPI_CTRL = *(uint32_t*)&lspiCtrl;
                break;
            case INF_BUS_MCLK:
                value = (uint8_t)(cmd & 0xFF);
                if(value<52 && value>1){
                    uint8_t div = (uint8_t)(102/(value+1));
                    *(uint32_t*)0x4f00007c = div << 16U; //1 = 51MHz
                }
                break;
            case INF_DATALANE:
                value = (uint8_t)(cmd & 0xFF);
                #ifdef TYPE_EC718M
                if(value>1){
                    lspiCtrl.dspiEn = 1;
                }
                else lspiCtrl.dspiEn = 0;
                #else
                if(value>1){
                    lspiCtrl.data2Lane = 1;
                }
                else lspiCtrl.data2Lane = 0;
                #endif
                lspi->reg->LSPI_CTRL = *(uint32_t*)&lspiCtrl;
                break;
            case INF_BUS_TYPE:
                switch((uint8_t)(cmd & 0xFF)) {
                    case INF_SPI_3W_I:
                        #ifdef TYPE_EC718M
                        lspiCmdAddr.busType = 0;
                        #else
                        lspiCtrl.busType = 0;
                        #endif
                        lspiCtrl.line4 = 0;
                        lspi->reg->LSPI_CTRL = *(uint32_t*)&lspiCtrl;
                        break;
                    case INF_SPI_3W_II:
                        #ifdef TYPE_EC718M
                        lspiCmdAddr.busType = 1;
                        #else
                        lspiCtrl.busType = 1;
                        #endif
                        lspiCtrl.line4 = 0;
                        lspi->reg->LSPI_CTRL = *(uint32_t*)&lspiCtrl;
                        break;
                    case INF_SPI_4W_I:
                        #ifdef TYPE_EC718M
                        lspiCmdAddr.busType = 0;
                        #else
                        lspiCtrl.busType = 0;
                        #endif
                        lspiCtrl.line4 = 1;
                        lspi->reg->LSPI_CTRL = *(uint32_t*)&lspiCtrl;
                        break; 
                    case INF_SPI_4W_II:
                        #ifdef TYPE_EC718M
                        lspiCmdAddr.busType = 1;
                        #else
                        lspiCtrl.busType = 1;
                        #endif
                        lspiCtrl.line4 = 1;
                        lspi->reg->LSPI_CTRL = *(uint32_t*)&lspiCtrl;
                        break; 
                }
            default:
                if(cmd != 0xFFFF && (cmd & 0xC000) && (cmd & 0xC000)< 0xC000){
                    if((cmd & 0xC000) == 0x8000) lspiInfo.frameHeight = (cmd & 0x3FFF);
                    else if((cmd & 0xC000) == 0x4000) lspiInfo.frameWidth = (cmd & 0x3FFF);
                    lspi->reg->LSPFINFO = *(uint32_t*)&lspiInfo;
                }
                break;
        }
    }
    if(cmd_len==0) spiData(lspi,0);
    spiSend(lspi,NULL,0);
    return num;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_query(uint32_t usrId)
{
    uint32_t index = usrId_to_index(usrId);
    if(index >= EC_SCR_INDEX_START && index < EC_SCR_INDEX_LIMIT)
    {
        if(s_scrUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(s_scrUsrIdList[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else{
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}
/**
  \fn
  \brief
  \return
*/
static uint32_t scr_set_idle(uint32_t index)
{
    uint32_t ret = 0;
    if(s_scrUsrIdList[index] == OPEN_HAL_STAT_UNUSED){
        s_scrUsrIdList[index] = index;
        s_scrUsrIdSeed[index] ++;
        s_scrUsrIdList[index] |= (uint32_t)(s_scrUsrIdSeed[index] << 16);
        ret = s_scrUsrIdList[index];
    }
    else if(s_scrUsrIdList[index] & OPEN_HAL_STAT_MUSK){
        s_scrUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
        ret = s_scrUsrIdList[index];
    }
    return ret;
}
/**
  \fn
  \brief
  \return
*/
static uint32_t scr_set_used(uint32_t usrId)
{
    api_ret_t stat = api_scr_query(usrId);
    if(stat != OPEN_HAL_IDLE){
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    s_scrUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}
/**
  \fn
  \brief
  \return
*/
static uint32_t scr_set_free(uint32_t index)
{
    if(index >= EC_SCR_INDEX_START && index < EC_SCR_INDEX_LIMIT)
    {
        s_scrUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        return 1;
    }
    return 0;
}
/**
  \fn          
  \brief     LCD上电初始化接口，用于默认创建 
  \return
*/
int api_scr_startup(void* para)
{
    for(int i=EC_SCR_INDEX_START;i<EC_SCR_INDEX_LIMIT;i++)
    {
        scr_set_free(i);
    }
}
/**
  \fn
  \brief
  \return
*/
static uint32_t dispArray(dma_data_t *array, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, void* address)
{
    uint8_t ret = 0;
    array->cmds[ret++] = 0xC000;
    array->cmds[ret++] = 0x022a;
    array->cmds[ret++] = (sx>>8) + 0x0300;
    array->cmds[ret++] = (sx & 0x00ff) + 0x0300;
    array->cmds[ret++] = (ex>>8) + 0x0300;
    array->cmds[ret++] = (ex & 0x00ff) + 0x0300;
    array->cmds[ret++] = 0x022b;
    array->cmds[ret++] = (sy>>8) + 0x0300;
    array->cmds[ret++] = (sy & 0x00ff) + 0x0300;
    array->cmds[ret++] = (ey>>8) + 0x0300;
    array->cmds[ret++] = (ey & 0x00ff) + 0x0300;
    array->cmds[ret++] = 0x022c;
    array->cmds[0] |= (ret-1);
    array->totalBytes = (ey-sy+1)*(ex-sx+1)*2;
    #ifdef TYPE_EC718M
    if(lspiCtrl.dspiEn){
        array->dataLen = (array->totalBytes)/2;
    }
    #else
    if(lspiCtrl.data2Lane){
        array->dataLen = (array->totalBytes)/2;
    }
    #endif
    else array->dataLen = (array->totalBytes);
    if(address){
        array->data = address;
    }
    array->dmaTrans.sourceAddress = array->data;
    array->dmaTrans.targetAddress = (void *)&(LSPI2->TFIFO);
    array->dmaTrans.flowControl = DMA_FLOW_CONTROL_TARGET;
    array->dmaTrans.addressIncrement = DMA_ADDRESS_INCREMENT_SOURCE;
    array->dmaTrans.dataWidth = DMA_DATA_WIDTH_FOUR_BYTES;
    array->dmaTrans.burstSize = DMA_BURST_32_BYTES;

    uint32_t res = array->totalBytes;
    if(array->totalBytes%4) res += (4-(array->totalBytes)%4);
    uint32_t patch = DMA_BULK_NUM - res%DMA_BULK_NUM;
    int cnt = (res+patch)/DMA_BULK_NUM;
    uint32_t package = DMA_BULK_NUM;
    if(patch%cnt == 0) {
        package -= (patch / cnt);
        array->dmaTrans.totalLength = package;  
        DMA_buildDescriptorChain(descriptor, &(array->dmaTrans), cnt, true, true, true);
    } else {
        cnt -= 1;
        res -= cnt * DMA_BULK_NUM;
        array->dmaTrans.totalLength = res;
        DMA_buildDescriptorChain(descriptor, &(array->dmaTrans), 1, false, false, false);
        array->dmaTrans.sourceAddress = (void *)(array->data) + res;
        array->dmaTrans.totalLength = DMA_BULK_NUM;
        DMA_buildDescriptorChain(descriptor+1, &(array->dmaTrans), cnt, true, true, true);
    }
    return array->totalBytes;
}
/**
  \fn
  \brief
  \return
*/
static int dispTrans(dma_data_t *array,uint32_t timeout)
{
    lspiRes_t *lspi = lspiResGet();
    dispConfig(array->cmds);
    lspiDmaCtrl.txDmaReqEn = 1;
    lspi->reg->DMACTL = *(uint32_t*)&lspiDmaCtrl;
    lspiCmdCtrl.wrRdn = 1;
    lspiCmdCtrl.ramWr = 1;
    lspiCmdCtrl.dataLen = array->dataLen;
    lspi->reg->LSPI_CCTRL = *(uint32_t*)&lspiCmdCtrl;
    DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, dispDmaCh, descriptor);
    EPAT_LOG(dispTran, P_INFO, "%d,%d",dispDmaCh,lspiCmdCtrl.dataLen);
}
/**
  \fn
  \brief
  \return
*/
static uint32_t api_scr_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, void* buf, uint32_t timeout)
{
    if(lspiMutex == NULL) return 0;
    if (osMutexAcquire(lspiMutex, timeout) == osOK)
    {
        dispArray(&dmaData, sx, sy, ex, ey, buf);
        dispTrans(&dmaData, timeout);
        osMutexRelease(lspiMutex);
    }
}
/**
  \fn
  \brief
  \return
*/
api_scr_infp api_scr_default(screenSupportList_e type)
{
    screen_device.clockDiv = 1U;
    lspiCtrl.datSrc = 1;
    lspiCtrl.enable = 1;
    lspiCtrl.colorModeIn = 3;   // RGB565
    lspiCtrl.colorModeOut = 1;  // RGB565
    screen_device.spiCtrl = *(uint32_t*)&lspiCtrl;
    lspiIntCtrl.lspiRamWrEndEn = 1;
    screen_device.intCtrl = *(uint32_t*)&lspiIntCtrl;
    lspiDataFmt.wordSize = 31;
    lspiDataFmt.txPack = 0;
    screen_device.dataFmt = *(uint32_t*)&lspiDataFmt;
    screen_device.fill = api_scr_fill;
    screen_device.dmaISR = NULL;
    screen_device.spiISR = NULL;
    switch(type)
    {
        case HAL_SCR_ST7789:
            break;
        case HAL_SCR_AXS15231:
            break;
        case HAL_SCR_ST77922:
            break;
        default:
            EPAT_LOG(api_scr_default, P_ERROR, "invalid type %d",type);
    }
    return &screen_device;
}

/**
  \fn
  \brief
  \return
*/
uint32_t api_scr_create(screenSupportList_e type,void *cfg)
{
    uint32_t usrId = 0;
    if(type < HAL_SCR_TOTAL)
    {
        if(scrEventFlags == NULL){
            scrEventFlags = osEventFlagsNew(NULL);
        }
        if(lspiMutex == NULL){
            lspiMutex = osMutexNew(NULL);
            osMutexRelease(lspiMutex);
        }
        usrId = scr_set_idle(type);
        if(cfg != NULL){
            api_scr_infp screen_infp = (api_scr_infp)cfg;
            lspiRes_t *lspi = lspiResGet();
            EPAT_LOG(scr_create_cfg, P_INFO, "[0x%x]lspi 0x%x,div%d,spiISR 0x%x",usrId,lspi,screen_infp->clockDiv,screen_infp->spiISR);
            if(screen_infp->spiISR != NULL){
                user_spi_cb = screen_infp->spiISR;
            }
            switch(type)
            {
                #if (LCD_ST7789_ENABLE == 1)
                case HAL_SCR_ST7789:
                    lcdDev = lcdOpen(0x7789, user_spi_cb, NULL);
                    break;
                #endif
                #if (LCD_AXS15231_ENABLE == 1)
                case HAL_SCR_AXS15231:
                    lcdDev = lcdOpen(0x15231, user_spi_cb, NULL);
                    break;
                #endif
                #if (LCD_ST77922_ENABLE == 1)
                case HAL_SCR_ST77922:
                    lcdDev = lcdOpen(0x77922, user_spi_cb, NULL);
                    break;
                #endif
                default:
                    EPAT_LOG(api_scr_default, P_ERROR, "invalid type %d",type);
            }
        }
    }
end:
    EPAT_LOG(api_scr_create, P_INFO, "usrId 0x%x,dispDmaCh 0x%x",usrId,dispDmaCh);
    return usrId;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_delete(uint32_t usrId)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_SCR_INDEX_LIMIT)
    {
        if(scr_set_free(index))
        {
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_scr_delete, P_INFO, "usrId 0x%X,index %d,ret %d",usrId,index,ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_open(uint32_t usrId,void *cfg,size_t timeout)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_SCR_INDEX_LIMIT)
    {
        if(cfg != NULL)
        {
            uint8_t *value = *(uint8_t *)cfg;
            lcdBackLight(lcdDev, value); // open backlight
            // uint16_t init_regs_num = dispConfig(lcd_init_reg);
            // EPAT_LOG(api_scr_open_regs, P_INFO, "num %d",init_regs_num);
        }
        scr_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_scr_open, P_INFO, "usrId 0x%X,index %d,ret %d",usrId,index,ret);
    return ret;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    // ASSERT(index < EC_SCR_INDEX_LIMIT);
    if(ret == OPEN_HAL_USED && index < EC_SCR_INDEX_LIMIT)
    {  
        scr_set_idle(index);
        lcdBackLight(lcdDev, 0);
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_scr_close, P_INFO, "index %d,usrId 0x%X,0x%X,ret%d",index,usrId,s_scrUsrIdList[index],ret);
    return ret;
}
/**
  \fn          
  \brief   不同LCD的相关控制寄存器及配置方式不同，此处只针对ST7789进行设置（理论在LCD驱动层配置接口）     
  \return
*/
static uint8_t s_MADCTL = 0x0;  //初始默认值需要和LCD初始化配置同步
static uint8_t api_scr_direction(lcdDrvFunc_t *lcd,uint8_t cmd,DisDirection_e Dir)
{
    switch(Dir)
    {
        case DIS_MIRROR_X:  
            s_MADCTL ^= BIT(6);
            break;
        case DIS_MIRROR_Y:  
            s_MADCTL ^= BIT(7);
            break;   
        case DIS_SWAP_XY:   
            s_MADCTL ^= 0xC0;
            break; 
        case DIS_DIR_LRTB: 
            s_MADCTL &= ~(BIT(2));   
            s_MADCTL &= ~(BIT(4)); 
            s_MADCTL &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT: 
            s_MADCTL &= ~(BIT(2)); 
            s_MADCTL &= ~(BIT(5)); 
            s_MADCTL |= BIT(4);
            break;   
        case DIS_DIR_RLTB:   
            s_MADCTL &= ~(BIT(4)); 
            s_MADCTL &= ~(BIT(5)); 
            s_MADCTL |= BIT(2);
            break; 
        case DIS_DIR_RLBT: 
            s_MADCTL &= ~(BIT(5));
            s_MADCTL |=  BIT(4);  
            s_MADCTL |=  BIT(2); 
            break; 
        case DIS_DIR_TBLR: 
            s_MADCTL &= ~(BIT(2)); 
            s_MADCTL &= ~(BIT(4));
            s_MADCTL |= BIT(5);  
            break; 
        case DIS_DIR_BTLR:  
            s_MADCTL &= ~(BIT(2));
            s_MADCTL |= BIT(4); 
            s_MADCTL |= BIT(5);   
            break; 
        case DIS_DIR_TBRL:  
            s_MADCTL &= ~(BIT(4));
            s_MADCTL |= BIT(2); 
            s_MADCTL |= BIT(5);
            break; 
        case DIS_DIR_BTRL: 
            s_MADCTL |= BIT(2); 
            s_MADCTL |= BIT(4); 
            s_MADCTL |= BIT(5);  
            break; 
    }
    lspiRes_t *lspi = lspiResGet();
    lspi->reg->LSPI_CADDR = cmd;
    spiData(lspi,s_MADCTL);
    spiSend(lspi,NULL,0);
    return s_MADCTL;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_ioctl(uint32_t usrId, api_scr_ioctl_t type, void *para)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        uint8_t value = *(uint8_t *)para;
        switch (type) {
            case SCREEN_BACKLIGHT:
                if(value>99) value = 99;
                if(value<4) value = 4;
                lcdBackLight(lcdDev,value);
                break;
            case SCREEN_DIRECTION:
                // lcdDev->direction(lcdDev,value);     //理论上添加
                api_scr_direction(lcdDev,0x36,value);   //实际上添加
                break;
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_scr_ioctl, P_INFO, "index %d,usrId 0x%X,type%d,ret%d",index,usrId,type,ret);
    return ret;
}
/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL){
            if(count==0){
                // SYSLOG_INFO("[0x%08X]read\r\n",usrId);
            }
            else if(cfg->runtime == RUNTIME_SUSPEND){
                if(cfg->mode == PM_LOWPOW){
                    // SYSLOG_INFO("[0x%08X]set\r\n",usrId);
                }
            }
            ret = OPEN_HAL_DONE;
        }
    }
end:
    EPAT_LOG(api_scr_pmctl, P_INFO, "index %d,0x%X,ret%d",index,usrId,ret);
    return ret;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_write(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL){
            api_screen_fill_t *bulk = (api_screen_fill_t*)buf;
            // EPAT_LOG(scr_write_data, P_INFO, "sx%d,sy%d,ex%d,ey%d,0x%x",bulk->sx, bulk->sy, bulk->ex, bulk->ey, bulk->data);
            uint32_t fillLen = lcdSetWindow(lcdDev, bulk->sx, bulk->sy, bulk->ex, bulk->ey);
            lcdFill(lcdDev, fillLen, bulk->data);
        }
        ret = OPEN_HAL_DONE;
    }
end:
    // EPAT_LOG(api_scr_write, P_INFO, "index %d,usrId 0x%X,ret%d",index,usrId,ret);
    return ret;
}

/**
  \fn
  \brief
  \return
*/
api_ret_t api_scr_read(uint32_t usrId, void* buf, size_t count)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL){
        }
        ret = OPEN_HAL_DONE;
    }
end:
    EPAT_LOG(api_scr_read, P_INFO, "usrId 0x%X,index %d,ret%d",usrId,index,ret);
    return ret;
}
#endif 