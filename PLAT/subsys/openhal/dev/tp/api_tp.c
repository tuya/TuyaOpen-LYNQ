/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_tp.c
 * Description:  ec7xx openHAL TP entry header file
 * History:      Rev1.0   2024-08-29
 *
 ****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Driver_Common.h"
#include "cmsis_os2.h"
#include "ostask.h"
#include "system_ec7xx.h"

#include DEBUG_LOG_HEADER_FILE
#include "api_comm.h"
#include "api_i2c.h"
#include "api_tp.h"
#include "api_wakeup.h"
#include "axs15231_func.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "cst816_func.h"
#include "devicemanager.h"
#include "ft6336_func.h"
#include "slpman.h"
#include "st77922_func.h"

#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)

typedef struct ApiTpCtx_
{
    uint32_t tpUsrId;
    uint32_t i2cUsrId;
    uint32_t PadUsrId;
    uint32_t GpioUsrId;
    uint32_t WakupUsrId;
} ApiTpCtx_t;

static uint32_t s_tpUsrIdList[EC_TP_INDEX_LIMIT] = {0};
static uint16_t s_tpUsrIdSeed[EC_TP_INDEX_LIMIT] = {0};

static api_tp_inf *get_tp_default_cfg()
{
    static api_tp_inf s_tp_default_cfg = {0};
    return &s_tp_default_cfg;
}

static api_tp_inf *get_tp_cur_cfg()
{
    static api_tp_inf s_tp_cur_cfg = {0};
    return &s_tp_cur_cfg;
}

static ApiTpCtx_t *get_tp_ctx()
{
    static ApiTpCtx_t s_tpCtx = {0};
    return &s_tpCtx;
}

static uint32_t usrId_to_index(uint32_t usrId)
{
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);
    if(index == (s_tpUsrIdList[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_TP_INDEX_LIMIT;
}

api_ret_t api_tp_query(uint32_t usrId)
{
    uint32_t index = usrId_to_index(usrId);
    if(index >= EC_TP_INDEX_START && index < EC_TP_INDEX_LIMIT)
    {
        if(s_tpUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(s_tpUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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

static uint32_t tp_set_free(uint32_t index)
{
    if(index >= EC_TP_INDEX_START && index < EC_TP_INDEX_LIMIT)
    {
        s_tpUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        return 1;
    }
    return 0;
}

static uint32_t tp_set_idle(uint32_t index)
{
    uint32_t ret = 0;
    if(s_tpUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        s_tpUsrIdList[index] = index;
        s_tpUsrIdSeed[index]++;
        s_tpUsrIdList[index] |= (uint32_t)(s_tpUsrIdSeed[index] << 16);
        ret = s_tpUsrIdList[index];
    }
    else if(s_tpUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        s_tpUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
        ret = s_tpUsrIdList[index];
    }
    return ret;
}

static uint32_t tp_set_used(uint32_t usrId)
{
    api_ret_t stat = api_tp_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    s_tpUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}

int api_tp_startup(void *para)
{
    for(int i = EC_TP_INDEX_START; i < EC_TP_INDEX_LIMIT; i++)
    {
        tp_set_free(i);
    }
    return 0;
}

static void tp_bus_cb(uint32_t event)
{
    EPAT_LOG(tp_bus_cb, P_INFO, "event 0x%X", event);
}

api_tp_infp api_tp_default(tpSupportList_e type)
{
    api_tp_infp def_cfg = get_tp_default_cfg();
    memset(def_cfg, 0, sizeof(api_tp_inf));

#if TP_IRQ_PAD_INDEX
    def_cfg->PadId = TP_IRQ_PAD_INDEX;
    def_cfg->GpioId = (TP_IRQ_GPIO_INSTANCE * 16 + TP_IRQ_GPIO_PIN);
    def_cfg->PadMux = TP_IRQ_PAD_ALT_FUNC;
#endif
#ifdef TP_I2C_PORT_NUM
    def_cfg->i2cId = TP_I2C_PORT_NUM;
#endif
    switch(type)
    {
        case HAL_TP_FT6336: {
            TpDevFt6336_t *dev = ft6336_dev_get();
            def_cfg->type = HAL_TP_FT6336;
            def_cfg->devFunc = dev->hyn_fuc_used;
            def_cfg->scan = dev->hyn_fuc_used->tp_scan;
            break;
        }
        case HAL_TP_AXS15231: {
            TpDevAxs15231_t *dev = axs15231_dev_get();
            def_cfg->devFunc = dev->hyn_fuc_used;
            def_cfg->scan = dev->hyn_fuc_used->tp_scan;
            def_cfg->type = HAL_TP_AXS15231;
            break;
        }
        case HAL_TP_ST77922: {
            TpDevSt77922_t *dev = st77922_dev_get();
            def_cfg->devFunc = dev->hyn_fuc_used;
            def_cfg->scan = dev->hyn_fuc_used->tp_scan;
            def_cfg->type = HAL_TP_ST77922;
            break;
        }
        case HAL_TP_CST816: {
            TpDevCst816_t *dev = cst816_dev_get();
            def_cfg->devFunc = dev->hyn_fuc_used;
            def_cfg->scan = dev->hyn_fuc_used->tp_scan;
            def_cfg->type = HAL_TP_CST816;
            break;
        }
        default: {
            EPAT_LOG(api_tp_default, P_ERROR, "invalid type %d", type);
            return NULL;
        }
    }

    EPAT_LOG(api_tp_default_resl, P_INFO, "tp: type: %d, scan: %x", type,
             def_cfg->scan);
    return def_cfg;
}

static int api_tp_disable_io_req()
{
    api_tp_infp cur_cfg = get_tp_cur_cfg();
    ApiTpCtx_t *ctx = get_tp_ctx();
    if((cur_cfg->PadId != 45) && (cur_cfg->PadId != 46) &&
       (cur_cfg->PadId != 47))
    {
        api_gpio_ioctl(ctx->GpioUsrId, OPEN_GPIO_IOCTL_ISR_CB, NULL);
    }
    else
    {
        int enable = 0;
        api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_INTERRUPT, &enable);
    }
    return 0;
}

static void api_tp_set_io_req(int pad_id, int gpio_id, int pad_mux,
                              IsrFunc isr_func)
{
    ApiTpCtx_t *ctx = get_tp_ctx();
    if((pad_id != 45) && (pad_id != 46) && (pad_id != 47))
    {
        GpioPinConfig_t pinConfig = {0};
        pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
        pinConfig.misc.interruptConfig = GPIO_INTERRUPT_RISING_EDGE;
        api_ret_t ret = api_gpio_create(gpio_id, NULL, &ctx->GpioUsrId);
        if((ret == OPEN_HAL_DONE) || (ctx->GpioUsrId == 0))
        {
            api_gpio_open(ctx->GpioUsrId, &pinConfig, 10);
            api_gpio_ioctl(ctx->GpioUsrId, OPEN_GPIO_IOCTL_ISR_CB, isr_func);
            EPAT_LOG(api_tp_set_io_req_1, P_INFO, "pad%d gpio%d, usrid: %d",
                     pad_id, gpio_id, ctx->GpioUsrId);
        }
    }
    else
    {
        APmuWakeupPadSettings_t wakeupPadSetting;
        int enable = 1;
        wakeupPadSetting.negEdgeEn = false;
        wakeupPadSetting.posEdgeEn = true;
        wakeupPadSetting.pullDownEn = false;
        wakeupPadSetting.pullUpEn = true;
        if(pad_id == 45)
        {
            ctx->WakupUsrId =
                api_wakeup_create(WAKEUP_PIN_3, &wakeupPadSetting);
            api_wakeup_open(ctx->WakupUsrId, NULL, 0);
            api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_INTERRUPT,
                             &enable);
            api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_ISR_CB,
                             isr_func);
        }
        else if(pad_id == 46)
        {
            ctx->WakupUsrId =
                api_wakeup_create(WAKEUP_PIN_4, &wakeupPadSetting);
            api_wakeup_open(ctx->WakupUsrId, NULL, 0);
            api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_INTERRUPT,
                             &enable);
            api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_ISR_CB,
                             isr_func);
        }
        else if(pad_id == 47)
        {
            ctx->WakupUsrId =
                api_wakeup_create(WAKEUP_PIN_5, &wakeupPadSetting);
            api_wakeup_open(ctx->WakupUsrId, NULL, 0);
            api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_INTERRUPT,
                             &enable);
            api_wakeup_ioctl(ctx->WakupUsrId, OPEN_WAKEUP_IOCTL_ISR_CB,
                             isr_func);
        }
    }
}

api_ret_t api_tp_create(uint32_t index, api_tp_infp cfg, uint32_t *usrId)
{
    api_ret_t ret = OPEN_HAL_DONE;
    uint32_t usr_id = tp_set_idle(index);
    ApiTpCtx_t *ctx = get_tp_ctx(usr_id);
    memset(ctx, 0, sizeof(ApiTpCtx_t));
    ctx->tpUsrId = usr_id;
    if(cfg)
    {
        api_tp_infp cur_cfg = get_tp_cur_cfg();
        switch(cfg->type)
        {
            case HAL_TP_FT6336:
            case HAL_TP_AXS15231:
            case HAL_TP_ST77922:
            case HAL_TP_CST816: {
                memcpy(cur_cfg, cfg, sizeof(api_tp_inf));
                break;
            }
            default: {
                break;
            }
        }
    }
    *usrId = usr_id;
    EPAT_LOG(api_tp_create, P_INFO, "usrId 0x%x,index 0x%x, ret:%d", usrId,
             index, ret);
    return ret;
}

api_ret_t api_tp_delete(uint32_t usrId)
{
    if(usrId == 0)
    {
        EPAT_LOG(api_tp_delete_failed1, P_INFO, "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_TP_INDEX_LIMIT)
    {
        tp_set_free(index);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_tp_delete, P_INFO, "delete tp%d", index);
    return ret;
}

api_ret_t api_tp_open(uint32_t usrId, api_tp_infp cfg, uint32_t timeout)
{
    if(usrId == 0)
    {
        EPAT_LOG(api_tp_open_failed1, P_INFO, "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_TP_INDEX_LIMIT)
    {
        api_tp_infp cur_cfg = get_tp_cur_cfg();
        if(cfg)
        {
            memcpy(cur_cfg, cfg, sizeof(api_tp_inf));
        }
        switch(cur_cfg->type)
        {
            case HAL_TP_FT6336: {
                TpDevFt6336_t *dev = ft6336_dev_get();
                cur_cfg->devFunc = dev->hyn_fuc_used;
                cur_cfg->scan = dev->hyn_fuc_used->tp_scan;
                ft6336_dev_init(cur_cfg->i2cId, 1, tp_bus_cb);
                break;
            }
            case HAL_TP_AXS15231: {
                TpDevAxs15231_t *dev = axs15231_dev_get();
                cur_cfg->devFunc = dev->hyn_fuc_used;
                cur_cfg->scan = dev->hyn_fuc_used->tp_scan;
                axs15231_dev_init(cur_cfg->i2cId, 1, tp_bus_cb);
                break;
            }
            case HAL_TP_ST77922: {
                TpDevSt77922_t *dev = st77922_dev_get();
                cur_cfg->devFunc = dev->hyn_fuc_used;
                cur_cfg->scan = dev->hyn_fuc_used->tp_scan;
                st77922_dev_init(cur_cfg->i2cId, 1, tp_bus_cb);
                break;
            }
            case HAL_TP_CST816: {
                TpDevCst816_t *dev = cst816_dev_get();
                cur_cfg->devFunc = dev->hyn_fuc_used;
                cur_cfg->scan = dev->hyn_fuc_used->tp_scan;
                dev->plat_data.reverse_x = cur_cfg->reverse_x;
                dev->plat_data.reverse_y = cur_cfg->reverse_y;
                dev->plat_data.x_resolution = cur_cfg->origin_width;
                dev->plat_data.y_resolution = cur_cfg->origin_height;
                dev->plat_data.output_x_resolution = cur_cfg->output_width;
                dev->plat_data.output_y_resolution = cur_cfg->output_height;
                cst816_dev_init(cur_cfg->i2cId, 1, cur_cfg->fw_id, tp_bus_cb);
                break;
            }
            default:
                EPAT_LOG(api_tp_open_invalid, P_ERROR, "invalid type %d",
                         cur_cfg->type);
                return OPEN_HAL_INVALID_PARA;
        }
        if(cfg->gpioISR != NULL)
        {
            api_tp_set_io_req(cur_cfg->PadId, cur_cfg->GpioId, cur_cfg->PadMux,
                              cur_cfg->gpioISR);
        }
        tp_set_used(usrId);
        ret = OPEN_HAL_DONE;
    }

    EPAT_LOG(api_tp_open, P_INFO, "open tp%d,ret %d", index, ret);
    return ret;
}

api_ret_t api_tp_close(uint32_t usrId)
{
    if(usrId == 0)
    {
        EPAT_LOG(api_tp_close_failed1, P_INFO, "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    api_tp_infp cur_cfg = get_tp_cur_cfg();
    if(ret == OPEN_HAL_USED && index < EC_TP_INDEX_LIMIT)
    {
        if(cur_cfg->gpioISR != NULL)
        {
            api_tp_disable_io_req();
        }
        api_tp_infp cfg = get_tp_cur_cfg();

        if(cfg->devFunc != NULL && cfg->devFunc->tp_supend != NULL)
        {
            cfg->devFunc->tp_supend();
        }
        tp_set_idle(index);
        ret = OPEN_HAL_DONE;
    }

    EPAT_LOG(api_tp_close, P_INFO, "tp%d,usrId 0x%X,0x%X,ret%d", index, usrId,
             s_tpUsrIdList[index], ret);
    return ret;
}

api_ret_t api_tp_ioctl(uint32_t usrId, api_tp_ioctl_t type, void *para)
{
    if(usrId == 0)
    {
        EPAT_LOG(api_tp_ioctl_failed1, P_INFO, "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        switch(type)
        {
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }

    EPAT_LOG(api_tp_ioctl, P_INFO, "tp%d,0x%X,type%d,ret%d", index, usrId, type,
             ret);
    return ret;
}

api_ret_t api_tp_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    // ASSERT(usrId > 0);
    api_ret_t ret = api_tp_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        api_tp_infp cur_cfg = get_tp_cur_cfg();
        if(cfg != NULL)
        {
            if(count == 0)
            {
                // SYSLOG_INFO("[0x%08X]read\r\n",usrId);
            }
            else if(cfg->runtime == RUNTIME_SUSPEND)
            {
                if(cur_cfg->gpioISR != NULL)
                {
                    api_tp_disable_io_req();
                    SYSLOG_INFO("tp%d pmctl disable GPIO ISR\r\n", index);
                }
                if(cur_cfg->devFunc != NULL &&
                   cur_cfg->devFunc->tp_supend != NULL)
                {
                    cur_cfg->devFunc->tp_supend();
                    SYSLOG_INFO("tp%d pmctl suspend\r\n", index);
                }
            }
            else if(cfg->runtime == RUNTIME_RESUME)
            {
                if(cur_cfg->devFunc != NULL &&
                   cur_cfg->devFunc->tp_resum != NULL)
                {
                    cur_cfg->devFunc->tp_resum();
                    SYSLOG_INFO("tp%d pmctl resume\r\n", index);
                }
                if(cur_cfg->gpioISR != NULL)
                {
                    api_tp_set_io_req(cur_cfg->PadId, cur_cfg->GpioId,
                                      cur_cfg->PadMux, cur_cfg->gpioISR);
                    SYSLOG_INFO("tp%d pmctl enable GPIO ISR\r\n", index);
                }
            }
            ret = OPEN_HAL_DONE;
        }
    }

    EPAT_LOG(api_tp_pmctl, P_INFO, "tp%d,0x%X,ret%d", index, usrId, ret);
    return ret;
}

api_ret_t api_tp_read(uint32_t usrId, void *buf, size_t count)
{
    api_ret_t ret = api_tp_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
    TpData_t *tp_data = (TpData_t *)buf;
    if(ret == OPEN_HAL_USED)
    {
        if((count < sizeof(TpData_t)) || (!buf))
        {
            return OPEN_HAL_INVALID_PARA;
        }
        api_tp_infp cur_cfg = get_tp_cur_cfg();
        if(cur_cfg->scan != NULL)
        {
            int16_t data[2] = {0};
            tp_data->pt_num = cur_cfg->scan(usrId, &data[0]);
            tp_data->pox_x = data[0];
            tp_data->pox_y = data[1];
        }
    }
    return OPEN_HAL_DONE;
}

api_ret_t api_tp_write(uint32_t usrId, void *buf, size_t count)
{
    return OPEN_HAL_DONE;
}
