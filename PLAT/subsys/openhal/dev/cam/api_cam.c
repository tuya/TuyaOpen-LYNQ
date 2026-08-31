/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_cam.c
 * Description:  ec7xx openHAL Cam entry source file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_Common.h"
#include "drivers/hal_cam.h"
#include DEBUG_LOG_HEADER_FILE
#include "devicemanager.h"
#include "api_comm.h"
#include "api_cam.h"

#ifdef EPAT_HAL_DEBUG
#define EPAT_LOG(subId, debugLevel, format, ...) \
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, subId, debugLevel, format, ##__VA_ARGS__)
#else
#define EPAT_LOG(subId, debugLevel, format, ...)
#endif

#define CLAMP_PARAM_VALUE(value, min, max) \
    do                                     \
    {                                      \
        if(value < min)                    \
        {                                  \
            value = min;                   \
        }                                  \
        if(value > max)                    \
        {                                  \
            value = max;                   \
        }                                  \
    } while(0)

static uint32_t s_camUsrIdList[EC_CAM_INDEX_LIMIT] = {0};
static uint16_t s_camUsrIdSeed[EC_CAM_INDEX_LIMIT] = {0};
// 数量上限
#define EC_DEV_CAM_INDEX_LIMIT (1)
static int s_camParam[OPEN_CAM_PARAM_ID_MAX] = {0};
static void api_cam_set_param_default(int* param)
{
    SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
    param[OPEN_CAM_PARAM_ID_FPS] = sns_obj->max_fps;
    param[OPEN_CAM_PARAM_ID_CMOS_MODEL] = sns_obj->type;
    param[OPEN_CAM_PARAM_ID_CMOS_MAX_FPS] = sns_obj->max_fps;
    param[OPEN_CAM_PARAM_ID_MIRROR_FLIP] = CAM_DIR_NORMAL;
    param[OPEN_CAM_PARAM_ID_SCENE] = CAM_SCENE_DAY;
    param[OPEN_CAM_PARAM_ID_EV] = 50;
    param[OPEN_CAM_PARAM_ID_CONTRAST] = 50;
    param[OPEN_CAM_PARAM_ID_SATURATION] = 50;
    param[OPEN_CAM_PARAM_ID_SHARP] = 50;
    param[OPEN_CAM_PARAM_ID_AWB] = 50;
    param[OPEN_CAM_PARAM_ID_AE] = 50;
}

static CamCfg_t* get_cam_cur_cfg()
{
    static CamCfg_t s_cam_cur_cfg = {0};
    return &s_cam_cur_cfg;
}

static uint32_t usrId_to_index(uint32_t usrId)
{
    uint32_t index = (uint32_t)(usrId & OPEN_HAL_PORT_MUSK);
    if(index == (s_camUsrIdList[index] & OPEN_HAL_PORT_MUSK))
    {
        return index;
    }
    return EC_CAM_INDEX_LIMIT;
}

static api_ret_t api_cam_query(uint32_t usrId)
{
    uint32_t index = usrId_to_index(usrId);
    if(index >= EC_CAM_INDEX_START && index < EC_CAM_INDEX_LIMIT)
    {
        if(s_camUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
        {
            return OPEN_HAL_FREE;
        }
        else if(s_camUsrIdList[index] & OPEN_HAL_STAT_MUSK)
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

static uint32_t cam_set_used(uint32_t usrId)
{
    api_ret_t stat = api_cam_query(usrId);
    if(stat != OPEN_HAL_IDLE)
    {
        return 0;
    }
    uint32_t index = usrId_to_index(usrId);
    s_camUsrIdList[index] |= OPEN_HAL_STAT_MUSK;
    return 1;
}

static uint32_t cam_set_free(uint32_t index)
{
    if(index >= EC_CAM_INDEX_START && index < EC_CAM_INDEX_LIMIT)
    {
        s_camUsrIdList[index] = OPEN_HAL_STAT_UNUSED;
        return 1;
    }
    return 0;
}

static uint32_t cam_set_idle(uint32_t index)
{
    if(s_camUsrIdList[index] == OPEN_HAL_STAT_UNUSED)
    {
        s_camUsrIdList[index] = index;
        s_camUsrIdSeed[index] = 1;
        s_camUsrIdList[index] |= (uint32_t)(s_camUsrIdSeed[index] << 16);
    }
    else if(s_camUsrIdList[index] & OPEN_HAL_STAT_MUSK)
    {
        s_camUsrIdList[index] &= ~(OPEN_HAL_STAT_MUSK);
    }
    return s_camUsrIdList[index];
}

static int api_cam_set_param(OpenCamParam_t* param)
{
    SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
    switch(param->id)
    {
        case OPEN_CAM_PARAM_ID_FPS: {
            if(sns_obj->pfn_set_fps == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, sns_obj->max_fps);
            sns_obj->pfn_set_fps(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_MIRROR_FLIP: {
            if(sns_obj->pfn_set_mirror_flip == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, CAM_DIR_NUM - 1);
            sns_obj->pfn_set_mirror_flip(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_SCENE: {
            if(sns_obj->pfn_set_scene == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, CAM_SCENE_NUM - 1);
            sns_obj->pfn_set_scene(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_EV: {
            if(sns_obj->pfn_set_ev == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, 100);
            sns_obj->pfn_set_ev(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_CONTRAST: {
            if(sns_obj->pfn_set_contrast == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, 100);
            sns_obj->pfn_set_contrast(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_SATURATION: {
            if(sns_obj->pfn_set_saturation == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, 100);
            sns_obj->pfn_set_saturation(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_SHARP: {
            if(sns_obj->pfn_set_sharp == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, 100);
            sns_obj->pfn_set_sharp(param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_AWB: {
            if(sns_obj->pfn_set_awb == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, CAM_WB_NUM - 1);
            sns_obj->pfn_set_awb(param->value == 0, param->value);
            break;
        }
        case OPEN_CAM_PARAM_ID_AE: {
            if(sns_obj->pfn_set_ae == NULL)
            {
                return -1;
            }
            CLAMP_PARAM_VALUE(param->value, 0, 1);
            sns_obj->pfn_set_ae(param->value);
            break;
        }
        default:
            return -1;
    }
    s_camParam[param->id] = param->value;
    return 0;
}

static int api_cam_get_param(OpenCamParam_t* param)
{
    SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
    switch(param->id)
    {
        case OPEN_CAM_PARAM_ID_CMOS_MODEL: {
            param->value = sns_obj->sensor_id;
            break;
        }
        case OPEN_CAM_PARAM_ID_CMOS_MAX_FPS: {
            param->value = sns_obj->max_fps;
            break;
        }
        case OPEN_CAM_PARAM_ID_RESOLUTION: {
            param->value = (sns_obj->img_width << 16) | (sns_obj->img_height);
            break;
        }
        case OPEN_CAM_PARAM_ID_MIRROR_FLIP:
        case OPEN_CAM_PARAM_ID_SCENE:
        case OPEN_CAM_PARAM_ID_EV:
        case OPEN_CAM_PARAM_ID_CONTRAST:
        case OPEN_CAM_PARAM_ID_SATURATION:
        case OPEN_CAM_PARAM_ID_SHARP:
        case OPEN_CAM_PARAM_ID_AWB:
        case OPEN_CAM_PARAM_ID_AE: {
            param->value = s_camParam[param->id];
            break;
        }
        default:
            return -1;
    }
    return 0;
}

int api_cam_startup(void* para)
{
    for(int i = EC_CAM_INDEX_START; i < EC_CAM_INDEX_LIMIT; i++)
    {
        cam_set_free(i);
    }
    return 0;
}

int api_cam_default(CamType_e type, CamCfg_t* cfg)
{
    CamCfg_t* default_cfg = hal_cam_get_default_cfg(type);
    if(default_cfg == NULL)
    {
        return -1;
    }
    memcpy(cfg, default_cfg, sizeof(CamCfg_t));
    return 0;
}

CamType_e api_cam_probe_type(int cspi_id, CamExtPwrCfg_t* ext_pwr_cfg,
                             CamRstPinCfg_t* rst_pin_cfg, CamI2cCfg_t* i2c_cfg)
{

    int ret = 0;
    ret = hal_cam_cmos_init(cspi_id, ext_pwr_cfg, rst_pin_cfg, i2c_cfg);
    if(ret != 0)
    {
        return CAM_TYPE_UNKNWON;
    }
    return hal_cam_get_type();
}

api_ret_t api_cam_create(int index, CamCfg_t* cfg, uint32_t* out)
{
    uint32_t usrId = 0;
    CamCfg_t* cur_cfg = NULL;

    if(index >= EC_CAM_INDEX_LIMIT)
    {
        return OPEN_HAL_INVALID_PARA;
    }
    cur_cfg = get_cam_cur_cfg();
    if(cfg != NULL)
    {
        memcpy(cur_cfg, cfg, sizeof(CamCfg_t));
        EPAT_LOG(api_cam_create_cust_cfg, P_INFO,
                 "create camera with customize parameters");
    }
    else
    {
        CamType_e default_type = CAM_TYPE_GC032A;
        CamCfg_t* def_cfg = hal_cam_get_default_cfg(default_type);
        if(def_cfg != NULL)
        {
            memcpy(cur_cfg, def_cfg, sizeof(CamCfg_t));
            EPAT_LOG(api_cam_create_def_cfg, P_INFO,
                     "create camera with default parameters");
        }
    }

    if(cur_cfg->type >= CAM_TYPE_NUM)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_create_failed0, P_INFO,
                      "api_cam_create type is invalid\r\n");
        return OPEN_HAL_INVALID_PARA;
    }

    usrId = cam_set_idle(index);
    if(usrId == 0)
    {
        EPAT_LOG(api_cam_create_failed, P_INFO, "create camera failed");
        return OPEN_HAL_INVALID_PARA;
    }

    if(out != NULL)
    {
        *(uint32_t*)out = usrId;
    }
    EPAT_LOG(api_cam_create, P_INFO, "usrId 0x%x,index 0x%x", usrId, index);
    return OPEN_HAL_DONE;
}

api_ret_t api_cam_delete(uint32_t usrId)
{
    api_ret_t ret = api_cam_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_IDLE && index < EC_CAM_INDEX_LIMIT)
    {
        cam_set_free(index);
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_delete_exe, P_INFO,
                      "api_cam_delete usrId 0x%X, ret: %d, index:%d, list:0x%x",
                      usrId, ret, index, s_camUsrIdList[index]);
        ret = OPEN_HAL_DONE;
    }
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_delete, P_INFO,
                  "usrId 0x%X,index %d,ret %d", usrId, index, ret);
    return ret;
}

api_ret_t api_cam_open(uint32_t usrId, CamCfg_t* cfg, size_t timeout)
{
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_open, P_INFO, "usrId 0x%X", usrId);
    if(usrId == 0)
    {
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_cam_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_open_state, P_INFO,
                  "api_cam_open usrId 0x%X, ret: %d, index:%d, list:0x%x",
                  usrId, ret, index, s_camUsrIdList[index]);
    if(ret != OPEN_HAL_IDLE || index >= EC_CAM_INDEX_LIMIT)
    {
        return OPEN_HAL_INVALID_PARA;
    }
    CamCfg_t* cur_cfg = get_cam_cur_cfg();
    if(cfg != NULL)
    {
        memcpy(cur_cfg, cfg, sizeof(CamCfg_t));
    }

    GPR_swReset(cur_cfg->drv_id == 0 ? RST_FCLK_USP0 : RST_FCLK_USP1);

    ret = hal_cam_init(cur_cfg);
    if(ret != 0)
    {
        EPAT_LOG(api_cam_open_failed0, P_INFO, "camera init failed");
        return OPEN_HAL_INVALID_PARA;
    }
    ret = hal_cam_init_reglist(cur_cfg->int_mode);
    if(ret != 0)
    {
        EPAT_LOG(api_cam_open_failed1, P_INFO, "camera init reglist failed");
        return OPEN_HAL_INVALID_PARA;
    }

    CamReqBuf_t req_buf = {0};
    req_buf.pool_addr = cur_cfg->qbuf_cfg.pool_addr;
    req_buf.buff_size = cur_cfg->qbuf_cfg.item_size;
    req_buf.buff_count = cur_cfg->qbuf_cfg.item_count;
    ret = hal_cam_req_buf(&req_buf);
    if(ret != 0)
    {
        EPAT_LOG(api_cam_open_failed2, P_INFO, "camera req buf failed");
        return OPEN_HAL_INVALID_PARA;
    }

    ret = hal_cam_start();
    if(ret != 0)
    {
        EPAT_LOG(api_cam_open_failed3, P_INFO, "camera start failed");
        return OPEN_HAL_INVALID_PARA;
    }
    cam_set_used(usrId);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_open_exe, P_INFO,
                  "camera open type: %d", hal_cam_get_type());
    api_cam_set_param_default(&s_camParam[0]);
    return ret;
}

api_ret_t api_cam_close(uint32_t usrId)
{
    if(usrId == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_close_failed1, P_INFO,
                      "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }
    api_ret_t ret = api_cam_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_close_state, P_INFO,
                  "api_cam_close usrId 0x%X, ret: %d, index:%d, list:0x%x",
                  usrId, ret, index, s_camUsrIdList[index]);
    if(ret == OPEN_HAL_USED && index < EC_CAM_INDEX_LIMIT)
    {
        hal_cam_deinit();
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_close_exe, P_INFO,
                      "camera close\r\n");
        cam_set_idle(index);
        ret = OPEN_HAL_DONE;
    }

    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_close, P_INFO,
                  "cam%d,usrId 0x%X,0x%X,ret%d", index, usrId,
                  s_camUsrIdList[index], ret);
    return ret;
}

api_ret_t api_cam_read(uint32_t usrId, void* buf, size_t count)
{
    if(usrId == 0)
    {
        ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_read_failed1, P_INFO,
                      "usrId is 0");
        return OPEN_HAL_INVALID_PARA;
    }

    int ret = hal_cam_read_data((CamImg_t*)buf, count);
    if(ret != 0)
    {
        return OPEN_HAL_TIMEOUT;
    }
    return OPEN_HAL_DONE;
}

api_ret_t api_cam_write(uint32_t usrId, void* buf, size_t count)
{
    return OPEN_HAL_DONE;
}
api_ret_t api_cam_ioctl(uint32_t usrId, OpenCamIoctl_e type, void* para)
{
    api_ret_t ret = api_cam_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_ioctl, P_INFO,
                  "usrId 0x%X, index: %d,ret %d, type %d, para %p", usrId,
                  index, ret, type, para);
    ret = OPEN_HAL_DONE;
    switch(type)
    {
        case OPEN_CAM_IOCTL_WR_CMOS_REG: {
            CamReg_t* reg = (CamReg_t*)para;
            SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
            hal_cam_write_reg(sns_obj->dev_addr, reg->addr, reg->value);
            break;
        }
        case OPEN_CAM_IOCTL_RD_CMOS_REG: {
            uint8_t addr = *(uint8_t*)para;
            uint8_t value = 0;
            SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
            hal_cam_read_reg(sns_obj->dev_addr, addr, &value);
            *(uint8_t*)para = value;
            break;
        }
        case OPEN_CAM_IOCTL_REGISTER_CALLBACK: {
            CamCbCfg_t* cb_cfg = (CamCbCfg_t*)para;
            hal_cam_register_cb(cb_cfg);
            break;
        }
        case OPEN_CAM_IOCTL_ENUM_IMAGE_INFO: {
            CamImgInfo_t* img_info = (CamImgInfo_t*)para;
            hal_cam_get_img_info(img_info);
            break;
        }
        case OPEN_CAM_IOCTL_CSPI2LSPI: {
            uint8_t enable = *(uint8_t*)para;
            hal_cam_cspi_2_lspi(enable);
            break;
        }
        case OPEN_CAM_IOCTL_SET_PARAM: {
            OpenCamParam_t* param = (OpenCamParam_t*)para;
            if(param != NULL)
            {
                api_cam_set_param(param);
            }
            break;
        }
        case OPEN_CAM_IOCTL_GET_PARAM: {
            OpenCamParam_t* param = (OpenCamParam_t*)para;
            if(param != NULL)
            {
                api_cam_get_param(param);
            }
            break;
        }
        case OPEN_CAM_IOCTL_REQ_BUF: {
            CamReqBuf_t* req_buf = (CamReqBuf_t*)para;
            if(req_buf != NULL)
            {
                hal_cam_req_buf(req_buf);
            }
            break;
        }
        case OPEN_CAM_IOCTL_GET_BUF: {
            OpenCamGetBuf_t *buf = (OpenCamGetBuf_t *)para;
            if(buf != NULL)
            {
                if (0 !=hal_cam_get_buf(buf->img, buf->timeout_ms))
                {
                    ret = OPEN_HAL_TIMEOUT;
                }
            }
            break;
        }
        case OPEN_CAM_IOCTL_RELEASE_BUF: {
            CamImg_t* img = (CamImg_t*)para;
            if(img != NULL)
            {
                hal_cam_release_buf(img);
            }
            break;
        }
        case OPEN_CAM_IOCTL_CLEAR_BUF: {
            hal_cam_clear_buff();
            break;
        }

        default:
            break;
    }

    return ret;
}

api_ret_t api_cam_pmctl(uint32_t usrId, open_hal_pm_t* cfg, size_t count)
{
    api_ret_t ret = api_cam_query(usrId);
    uint32_t index = usrId_to_index(usrId);
    if(ret == OPEN_HAL_USED)
    {
        if(cfg != NULL)
        {
            if(cfg->runtime == RUNTIME_SUSPEND)
            {
                SYSLOG_INFO("cam%d pmctl suspend\r\n", index);
                SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
                hal_cam_stop();
                if(sns_obj)
                {
                    sns_obj->pfn_power_down();
                }
            }
            else if(cfg->runtime == RUNTIME_RESUME)
            {
                SYSLOG_INFO("cam%d pmctl resume\r\n", index);
                SensorFuncObj_t* sns_obj = hal_cam_get_func_obj();
                if(sns_obj)
                {
                    sns_obj->pfn_power_up();
                }

                CamCfg_t* cur_cfg = get_cam_cur_cfg();
                hal_cam_init(cur_cfg);
                hal_cam_init_reglist(cur_cfg->int_mode);
                CamReqBuf_t req_buf = {0};
                req_buf.pool_addr = cur_cfg->qbuf_cfg.pool_addr;
                req_buf.buff_size = cur_cfg->qbuf_cfg.item_size;
                req_buf.buff_count = cur_cfg->qbuf_cfg.item_count;
                hal_cam_req_buf(&req_buf);
                hal_cam_start();
            }
            ret = OPEN_HAL_DONE;
        }
    }
    else
    {
        SYSLOG_INFO("cam%d not used, no need to suspend or resume\r\n", index);
        ret = OPEN_HAL_DONE;
    }

    ECPLAT_PRINTF(UNILOG_OPEN_HAL, api_cam_pmctl, P_INFO, "cam%d,0x%X,ret%d",
                  index, usrId, ret);
    return ret;
}