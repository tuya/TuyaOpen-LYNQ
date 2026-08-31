#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "timer.h"
#include "api_scr.h"
#include "hal_screen.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif

typedef struct PmCtrlParam_
{
    int8_t back_light;
    int8_t direct;
    int8_t ref_direct;
    int8_t disp_mode;
    int8_t pix_mode;
} PmCtrlParam_t;

static uint32_t s_scrUsrIdList[EC_SCR_INDEX_LIMIT] = {0};
static uint16_t s_scrUsrIdSeed[EC_SCR_INDEX_LIMIT] = {0};
static ScrDmaEventCb s_scrDmaCb = NULL;
static ScrUspEventCb s_scrUspCb = NULL;

static PmCtrlParam_t *get_pmctrl_cur_param()
{
    static PmCtrlParam_t s_pmtrl_param = {0};
    return &s_pmtrl_param;
}

static ScrConfig_t *get_screen_cur_cfg()
{
    static ScrConfig_t s_scr_cur_cfg = {0};
    return &s_scr_cur_cfg;
}
static uint32_t usrId_to_index(uint32_t usrId)
{
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);
    if(index == (s_scrUsrIdList[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_SCR_INDEX_LIMIT;
}

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
        else
        {
            return OPEN_HAL_IDLE;
        }
    }
    return OPEN_HAL_NONE;
}

static uint32_t scr_set_idle(uint32_t index)
{
    if(s_scrUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        s_scrUsrIdList[index] = index;
        s_scrUsrIdSeed[index] = 1;
        s_scrUsrIdList[index] |= (uint32_t)(s_scrUsrIdSeed[index] << 16);
    }
    else if(s_scrUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        s_scrUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
    }
    return s_scrUsrIdList[index];
}

static uint32_t scr_set_used(uint32_t usrId)
{
    api_ret_t stat = api_scr_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    s_scrUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}

static uint32_t scr_set_free(uint32_t index)
{
    if(index >= EC_SCR_INDEX_START && index < EC_SCR_INDEX_LIMIT)
    {
        s_scrUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        return 1;
    }
    return 0;
}

int api_scr_startup(void *para)
{
    for(int i = EC_SCR_INDEX_START; i < EC_SCR_INDEX_LIMIT; i++)
    {
        scr_set_free(i);
    }
    return 0;
}

api_ret_t api_scr_default(ScreenType_e type, ScrConfig_t *cfg)
{
    ScrConfig_t *default_cfg = hal_scr_get_default_config(type);
    if(default_cfg == NULL)
    {
        return OPEN_HAL_INVALID_PARA;
    }
    memcpy(cfg, default_cfg, sizeof(ScrConfig_t));
    return OPEN_HAL_DONE;
}

api_ret_t api_scr_create(uint32_t index, ScrConfig_t *cfg, void *out)
{
    uint32_t usrId = 0;
    ScrConfig_t *cur_cfg = NULL;

    if(index != 0)
    {
        return OPEN_HAL_INVALID_PARA;
    }
    cur_cfg = get_screen_cur_cfg();
    if(cfg != NULL)
    {
        memcpy(cur_cfg, cfg, sizeof(ScrConfig_t));
        EPAT_LOG(api_scr_create_cust_cfg, P_INFO,
                 "create screen with customize parameters");
    }
    else
    {
        ScreenType_e default_type = SCREEN_TYPE_ST7789;
        ScrConfig_t *def_cfg = hal_scr_get_default_config(default_type);
        if(def_cfg != NULL)
        {
            memcpy(cur_cfg, def_cfg, sizeof(ScrConfig_t));
            EPAT_LOG(api_scr_create_def_cfg, P_INFO,
                     "create screen with default parameters");
        }
    }

    if(cur_cfg->type >= SCREEN_TYPE_TOTAL)
    {
        EPAT_LOG(api_scr_create_invalid_type, P_INFO, "invalid screen type %d",
                 cur_cfg->type);
        return OPEN_HAL_INVALID_PARA;
    }

    usrId = scr_set_idle(index);
    if(usrId == 0)
    {
        EPAT_LOG(api_scr_create_failed, P_INFO, "create screen failed");
        return OPEN_HAL_INVALID_PARA;
    }

    if(out != NULL)
    {
        *(uint32_t *)out = usrId;
    }
    EPAT_LOG(api_scr_create, P_INFO, "usrId 0x%x,index 0x%x", usrId, index);
    return OPEN_HAL_DONE;
}

api_ret_t api_scr_delete(uint32_t usrId)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_SCR_INDEX_LIMIT)
    {
        scr_set_free(index);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_scr_delete, P_INFO, "usrId 0x%X,index %d,ret %d", usrId, index,
             ret);
    return ret;
}

static void api_scr_dma_cb(uint32_t event)
{
    if(s_scrDmaCb)
    {
        s_scrDmaCb(event);
    }
}

static void api_scr_usp_cb()
{
    if(s_scrUspCb)
    {
        s_scrUspCb();
    }
}

static void init_pmctrl_param(ScrConfig_t *cur_cfg)
{
    PmCtrlParam_t *p = get_pmctrl_cur_param();
    p->back_light = cur_cfg->init_backlight_level;
    p->direct = -1;
    p->ref_direct = -1;
    p->disp_mode = -1;
    p->pix_mode = -1;
}

static int restore_pmctrl_param()
{
    PmCtrlParam_t *p = get_pmctrl_cur_param();
    ScrConfig_t *cur_cfg = get_screen_cur_cfg();
    if(p->back_light != -1)
    {
        // hal_screen_set_backlight(cur_cfg->drv_id, p->back_light);
    }
    if(p->direct != -1)
    {
        hal_screen_set_direction(cur_cfg->drv_id, p->direct);
    }
    if(p->ref_direct != -1)
    {
        hal_screen_set_refresh_dir(cur_cfg->drv_id, p->ref_direct);
    }
    if(p->disp_mode != -1)
    {
        hal_screen_set_display_mode(cur_cfg->drv_id, p->disp_mode);
    }
    if(p->pix_mode != -1)
    {
        hal_screen_set_display_pix_mode(cur_cfg->drv_id, p->pix_mode);
    }
    return 0;
}

api_ret_t api_scr_open(uint32_t usrId, ScrConfig_t *cfg, size_t timeout)
{
    ScrConfig_t *cur_cfg = NULL;
    if(usrId == 0)
    {
        EPAT_LOG(api_scr_open_failed1, P_INFO, "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);

    if((ret != OPEN_HAL_IDLE) || (index >= EC_SCR_INDEX_LIMIT))
    {
        EPAT_LOG(api_scr_open_failed2, P_INFO, "usrId 0x%X,index %d,ret %d",
                 usrId, index, ret);
        return OPEN_HAL_INVALID_PARA;
    }

    cur_cfg = get_screen_cur_cfg();
    if(cfg != NULL)
    {
        memcpy(cur_cfg, cfg, sizeof(ScrConfig_t));
        EPAT_LOG(api_scr_open_cust_cfg, P_INFO,
                 "open screen with customize parameters");
    }
    ret = hal_screen_open(cur_cfg->drv_id, cur_cfg, api_scr_dma_cb,
                          api_scr_usp_cb);
    init_pmctrl_param(cur_cfg);
    scr_set_used(usrId);
    ret = OPEN_HAL_DONE;
    EPAT_LOG(api_scr_open, P_INFO, "usrId 0x%X,index %d,ret %d", usrId, index,
             ret);
    return ret;
}

api_ret_t api_scr_close(uint32_t usrId)
{
    ScrConfig_t *cur_cfg = NULL;
    if(usrId == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_scr_close_failed1, P_INFO,
                      "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED && index < EC_SCR_INDEX_LIMIT)
    {
        scr_set_idle(index);
        cur_cfg = get_screen_cur_cfg();
        hal_screen_close(cur_cfg->drv_id);
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_scr_close, P_INFO, "index %d,usrId 0x%X,0x%X,ret%d", index,
             usrId, s_scrUsrIdList[index], ret);
    return ret;
}

api_ret_t api_scr_ioctl(uint32_t usrId, OpenScreenIoctl_e type, void *para)
{
    api_ret_t ret = api_scr_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
    ScrConfig_t *cur_cfg = get_screen_cur_cfg();
    PmCtrlParam_t *cur_param = get_pmctrl_cur_param();
    if(ret == OPEN_HAL_USED)
    {
        uint8_t value = *(uint8_t *)para;
        switch(type)
        {
            case OPEN_SCREEN_BACKLIGHT_SET: {
                hal_screen_set_backlight(cur_cfg->drv_id, value);
                if(value != 0)
                {
                    cur_param->back_light = value;
                }
                break;
            }
            case OPEN_SCREEN_BACKLIGHT_GET: {
                hal_screen_get_backlight(cur_cfg->drv_id, (uint8_t *)para);
                break;
            }
            case OPEN_SCREEN_DIRECTION: {
                hal_screen_set_direction(cur_cfg->drv_id, value);
                cur_param->direct = value;
                break;
            }
            case OPEN_SCREEN_RST_CLEAR_FIFO: {
                hal_screen_rst_and_clear_fifo(cur_cfg->drv_id);
                break;
            }
            case OPEN_SCREEN_CAM_PREVIEW: {
                ScrPrevInfo_t *info = (ScrPrevInfo_t *)para;
                hal_screen_cam_preview(cur_cfg->drv_id, info);
                break;
            }
            case OPEN_SCREEN_SET_REFRESH_DIR: {
                hal_screen_set_refresh_dir(cur_cfg->drv_id, value);
                cur_param->ref_direct = value;
                break;
            }
            case OPEN_SCREEN_SET_DISPLAY_MODE: {
                hal_screen_set_display_mode(cur_cfg->drv_id, value);
                cur_param->disp_mode = value;
                break;
            }
            case OPEN_SCREEN_SET_DISPLAY_PIX_MODE: {
                hal_screen_set_display_pix_mode(cur_cfg->drv_id, value);
                cur_param->pix_mode = value;
                break;
            }
            case OPEN_SCREEN_SET_DMA_CALLBACK: {
                s_scrDmaCb = (ScrDmaEventCb)para;
                break;
            }
            case OPEN_SCREEN_SET_USP_CALLBACK: {
                s_scrUspCb = (ScrUspEventCb)para;
                break;
            }
            case OPEN_SCREEN_SET_LCD_WINDOW: {
                LcdDispWin_t *win = (LcdDispWin_t *)para;
                hal_screen_set_refresh_loc(cur_cfg->drv_id, win->start_x,
                                           win->start_y, win->width,
                                           win->height);
                break;
            }
            case OPEN_SCREEN_BACKLIGHT_OPEN: {
                hal_screen_set_backlight(cur_cfg->drv_id, cur_param->back_light);
                break;
            }
            case OPEN_SCREEN_BACKLIGHT_CLOSE: {
                hal_screen_set_backlight(cur_cfg->drv_id, 0);
                break;
            }
            default:
                break;
        }
        ret = OPEN_HAL_DONE;
    }

    EPAT_LOG(api_scr_ioctl, P_INFO, "index %d,usrId 0x%X,type%d,ret%d", index,
             usrId, type, ret);
    return ret;
}

api_ret_t api_scr_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
{
    api_ret_t ret = api_scr_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
    ScrConfig_t *cur_cfg = get_screen_cur_cfg();
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL)
        {
            if(cfg->runtime == RUNTIME_SUSPEND)
            {
                hal_screen_suspend(cur_cfg->drv_id);
                cur_cfg = get_screen_cur_cfg();
                hal_screen_close(cur_cfg->drv_id);
            }
            else if(cfg->runtime == RUNTIME_RESUME)
            {
                cur_cfg = get_screen_cur_cfg();
                hal_screen_open(cur_cfg->drv_id, cur_cfg, api_scr_dma_cb,
                                api_scr_usp_cb);
                restore_pmctrl_param();
            }
            ret = OPEN_HAL_DONE;
        }
    }

    EPAT_LOG(api_scr_pmctl, P_INFO, "index %d,0x%X,ret%d", index, usrId, ret);
    return ret;
}

api_ret_t api_scr_write(uint32_t usrId, void *buf, size_t count)
{
    api_ret_t ret = api_scr_query(usrId);
    // uint32_t index = usrId_to_index(usrId);
    ScrConfig_t *cur_cfg = get_screen_cur_cfg();
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL)
        {
            ScrWriteParam_t *wr_param = (ScrWriteParam_t *)buf;
            uint32_t fillLen = hal_screen_set_refresh_loc(
                cur_cfg->drv_id, wr_param->start_x, wr_param->start_y,
                wr_param->width, wr_param->height);
            if((fillLen != 0) && (wr_param->size >= fillLen))
            {
                hal_screen_refresh(cur_cfg->drv_id, wr_param->data, fillLen);
            }
        }
        ret = OPEN_HAL_DONE;
    }
    return ret;
}

api_ret_t api_scr_read(uint32_t usrId, void *buf, size_t count)
{
    api_ret_t ret = api_scr_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    index = index;
    if(ret == OPEN_HAL_USED)
    {
        if(buf != NULL)
        {
        }
        ret = OPEN_HAL_DONE;
    }
    EPAT_LOG(api_scr_read, P_INFO, "usrId 0x%X,index %d,ret%d", usrId, index,
             ret);
    return ret;
}
