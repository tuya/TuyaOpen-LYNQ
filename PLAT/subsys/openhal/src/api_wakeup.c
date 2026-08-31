#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> /* atoi */
#include <string.h> /* memset */

#include "Driver_Common.h"
#include "system_ec7xx.h"

#include DEBUG_LOG_HEADER_FILE

#include "api_comm.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "clock.h"
#include "cms_api.h"
#include "devicemanager.h"
#include "gpio.h"
#include "ic.h"
#include "mem_map.h"
#include "pad.h"
#include "slpman.h"
#include "timer.h"
#include "api_wakeup.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...)  \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif

typedef int (*pfn_PadWakeupHook)(uint32_t pad_num);
extern int RegPadWakeupIntrHook(pfn_PadWakeupHook pfunc);
extern int RegAppPadWakeupIntrHook(pfn_PadWakeupHook pfunc);

static uint32_t sWakeupUsrIdList[EC_WAKEUP_INDEX_LIMIT] = {0};
static IsrFunc wakeup_isr_cb[EC_WAKEUP_INDEX_LIMIT] = {NULL};
static uint16_t sWakeupUsrIdSeed[EC_WAKEUP_INDEX_LIMIT] = {0};

/**
  \fn          
  \brief
  \return
*/
int32_t api_wakeup_parse(char* str,wakeup_config_t *cfg)
{
    if(str == NULL) return -1;
    uint8_t num = 0;
    char *item_str[CSV_WAKEUP_ITEM_MAX];
    char *head = strtok(str, ",");
    while(head != NULL && num<CSV_WAKEUP_ITEM_MAX) 
    {
        item_str[num] = head;
        head = strtok(NULL, ",");
        num ++; 
    }
    int8_t index = atoi(item_str[0]);   // index
    EPAT_LOG(api_wakeup_parse, P_INFO, "wakeup %d", index);
    return index;
}
/**
  \fn          
  \brief    PWM初始化，使用HAL统一的参数结构，会执行硬件配置
  \return
*/
api_ret_t api_wakeup_setup(int8_t index, wakeup_config_t* para)
{
    if(index < EC_WAKEUP_INDEX_START || index >= EC_WAKEUP_INDEX_LIMIT) 
    {
        EPAT_LOG(api_wakeup_setup_1, P_ERROR, "error index %d [%d,%d]", index, EC_WAKEUP_INDEX_START, EC_WAKEUP_INDEX_LIMIT);
        return OPEN_HAL_INVALID_PARA;
    }
    else if(para == NULL)   // 执行去初始化操作
    {
        return OPEN_HAL_DONE;
    }
    else    // 执行初始化操作
    {
        return OPEN_HAL_DONE;
    }
}

/* ---------------------------------------------------------------------------------------------- */
static uint32_t usrId_to_wakeup(uint32_t usrId)
{
    ASSERT(usrId > 0);
    uint32_t wakeup = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);  // 位提取
    ASSERT(wakeup < EC_WAKEUP_INDEX_LIMIT);
    if(wakeup == (sWakeupUsrIdList[wakeup] & OPEN_HAL_PORT_MUSK))
    {
        return wakeup;
    }
    return EC_WAKEUP_INDEX_LIMIT;
}

static int wakeup_pad_hook(uint32_t pad_num)
{
    if(wakeup_isr_cb[pad_num] != NULL)
    {
        wakeup_isr_cb[pad_num](pad_num);
    }
    return 0;
}

static void wakeupIsrSet(int8_t wakeup, IsrFunc cb)
{
    if(cb != NULL)
    {
        wakeup_isr_cb[wakeup] = cb;
        RegAppPadWakeupIntrHook(wakeup_pad_hook);
    }
}

api_ret_t api_wakeup_query(uint32_t usrId)
{
    uint32_t index = usrId_to_wakeup(usrId);
    if(index >= EC_WAKEUP_INDEX_START && index < EC_WAKEUP_INDEX_LIMIT)
    {
        if(sWakeupUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(sWakeupUsrIdList[index] & OPEN_HAL_STAT_MUSK)
        {
            return OPEN_HAL_USED;
        }
        else
        {
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}

static uint32_t wakeup_set_free(uint32_t index)
{
    if(index >= EC_WAKEUP_INDEX_START && index < EC_WAKEUP_INDEX_LIMIT)
    {
        sWakeupUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        wakeup_isr_cb[index] = NULL;
        return 1;
    }
    return 0;
}

static uint32_t wakeup_set_idle(uint32_t index)
{
    ASSERT(index < EC_WAKEUP_INDEX_LIMIT);
    if(sWakeupUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        sWakeupUsrIdList[index] = index;
        sWakeupUsrIdSeed[index] = 1;
        sWakeupUsrIdList[index] |= (uint32_t)(sWakeupUsrIdSeed[index] << 16);
    }
    else if(sWakeupUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        sWakeupUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
    }
    return sWakeupUsrIdList[index];
}

static uint32_t wakeup_set_used(uint32_t usrId)
{
    api_ret_t stat = api_wakeup_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_wakeup(usrId);
    sWakeupUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 0;
}

int api_wakeup_startup(void *para)
{
    for(int i = EC_WAKEUP_INDEX_START; i < EC_WAKEUP_INDEX_LIMIT; i++)
    {
        wakeup_set_free(i);
    }
    return 0;
}

uint32_t api_wakeup_create(uint32_t index, void *cfg)
{
    uint32_t usrId = wakeup_set_idle(index);
    if(usrId)
    {
        if(cfg != NULL)
        {
            APmuWakeupPadSettings_t *wakeupPadSetting =
                (APmuWakeupPadSettings_t *)cfg;
            slpManSetWakeupPadCfg(index, true, wakeupPadSetting);
        }
    }
    EPAT_LOG(api_wakeup_create, P_INFO, "get user id %d", usrId);
    return usrId;
}

api_ret_t api_wakeup_delete(uint32_t usrId)
{
    api_ret_t ret = api_wakeup_query(usrId);
    uint32_t index = usrId_to_wakeup(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(wakeup_set_free(index))
        {
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

api_ret_t api_wakeup_open(uint32_t usrId, void *cfg, size_t timeout)
{
    ASSERT(usrId > 0);
    api_ret_t ret = api_wakeup_query(usrId);
    uint32_t index = usrId_to_wakeup(usrId);
    if(ret == OPEN_HAL_IDLE)
    {
        if(cfg != NULL)
        {
            APmuWakeupPadSettings_t *wakeupPadSetting =
                (APmuWakeupPadSettings_t *)cfg;
            slpManSetWakeupPadCfg(index, true, wakeupPadSetting);
        }
        wakeup_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

api_ret_t api_wakeup_close(uint32_t usrId)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_wakeup_query(usrId);
    uint32_t index = usrId_to_wakeup(usrId);
    if(ret == OPEN_HAL_USED)
    {
        wakeup_set_idle(index);
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

api_ret_t api_wakeup_ioctl(uint32_t usrId, api_wakeup_ioctl_e type, void *para)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_wakeup_query(usrId);
    uint32_t index = usrId_to_wakeup(usrId);
    EPAT_LOG(api_wakeup_ioctl_enter, P_INFO, "ret: %d, index: %d", ret, index);
    if(ret == OPEN_HAL_USED)
    {
        switch(type)
        {
            case OPEN_WAKEUP_IOCTL_CFG: {
                APmuWakeupPadSettings_t *wakeupPadSetting =
                    (APmuWakeupPadSettings_t *)para;
                slpManSetWakeupPadCfg(index, true, wakeupPadSetting);
                break;
            }
            case OPEN_WAKEUP_IOCTL_INTERRUPT: {
                uint8_t enable = *(int *)para;
                if(index == WAKEUP_PIN_0)
                {
                    if(enable)
                    {
                        NVIC_EnableIRQ(PadWakeup0_IRQn);
                    }
                    else
                    {
                        NVIC_DisableIRQ(PadWakeup0_IRQn);
                    }
                }
                else if(index == WAKEUP_PIN_1)
                {
                    if(enable)
                    {
                        NVIC_EnableIRQ(PadWakeup1_IRQn);
                    }
                    else
                    {
                        NVIC_DisableIRQ(PadWakeup1_IRQn);
                    }
                }
                else if(index == WAKEUP_PIN_2)
                {
                    if(enable)
                    {
                        NVIC_EnableIRQ(PadWakeup2_IRQn);
                    }
                    else
                    {
                        NVIC_DisableIRQ(PadWakeup2_IRQn);
                    }
                }
                else if(index == WAKEUP_PIN_3)
                {
                    if(enable)
                    {
                        NVIC_EnableIRQ(PadWakeup3_IRQn);
                    }
                    else
                    {
                        NVIC_DisableIRQ(PadWakeup3_IRQn);
                    }
                }
                else if(index == WAKEUP_PIN_4)
                {
                    if(enable)
                    {
                        NVIC_EnableIRQ(PadWakeup4_IRQn);
                    }
                    else
                    {
                        NVIC_DisableIRQ(PadWakeup4_IRQn);
                    }
                }
                else if(index == WAKEUP_PIN_5)
                {
                    if(enable)
                    {
                        NVIC_EnableIRQ(PadWakeup5_IRQn);
                    }
                    else
                    {
                        NVIC_DisableIRQ(PadWakeup5_IRQn);
                    }
                }
                EPAT_LOG(api_wakeup_ioctl_interupt, P_INFO, "set index %d interrupt: %d", index, enable);
                break;
            }
            case OPEN_WAKEUP_IOCTL_ISR_CB: {
                wakeupIsrSet(index, para);
                EPAT_LOG(api_wakeup_ioctl_cb, P_INFO, "set index %d interrupt: %x", index, para);
                break;
            }
            case OPEN_WAKEUP_IOCTL_GET_LEVEL: {
                uint8_t *level = (uint8_t *)para;
                *level = ((slpManGetWakeupPinValue() & (1 << index)) == 0) ? 0 : 1;
                break;
            }

            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

/**
  \fn
  \brief    对设备功耗和模式进行配置
  \return
*/
api_ret_t api_wakeup_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_wakeup_query(usrId);
    //uint32_t index = usrId_to_wakeup(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg->runtime == RUNTIME_SUSPEND)
        {
            if(cfg->mode == PM_LOWPOW)
            {
            }
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}
