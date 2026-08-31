/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_cam.h
 * Description:  ec7xx openHAL CAM entry header file
 * History:      Rev1.0   2024-01-11
 *
 ****************************************************************************/
#ifndef _API_CAM_H_
#define _API_CAM_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#include "hal_cam.h"


#define EC_CAM_INDEX_START (0)
#define EC_CAM_INDEX_LIMIT (1)

typedef struct CamReg_
{
    uint8_t addr;
    uint8_t value;
} CamReg_t;

typedef enum OpenCamIoctl_
{
    OPEN_CAM_IOCTL_DUMMY = 0,   /* dummy */
    OPEN_CAM_IOCTL_WR_CMOS_REG, /* 写CMOS的寄存器, 输入参数为CamReg_t*/
    OPEN_CAM_IOCTL_RD_CMOS_REG, /* 读取CMOS的寄存器,
                                   输入参数为uint8_t*，寄存器地址，输出参数为uint8_t*，寄存器数值
                                 */
    OPEN_CAM_IOCTL_REGISTER_CALLBACK, /* 注册相机采集相关回调函数，输入参数为CamCbCfg_t*
                                       */
    OPEN_CAM_IOCTL_ENUM_IMAGE_INFO, /* 枚举支持的图像信息，包含图像格式信息，分辨率信息，帧率信息等*/
    OPEN_CAM_IOCTL_CSPI2LSPI, /* 使能相机数据直接连接到LCD的功能，输入参数为uint8_t*，0为关闭，1为开启
                               */
    OPEN_CAM_IOCTL_SET_PARAM, /* 设置相机参数，输入参数为OpenCamParam_t* */
    OPEN_CAM_IOCTL_GET_PARAM, /* 获取相机参数，输入参数为OpenCamParam_t* */
    OPEN_CAM_IOCTL_REQ_BUF, /* 请求相机数据缓冲区，输入参数为OpenCamReqBuf_t*, 启动采集前必须调用，否则无法正常采集数据*/
    OPEN_CAM_IOCTL_GET_BUF, /* 获取缓冲的图像数据，输入参数为OpenCamGetBuf_t* */
    OPEN_CAM_IOCTL_RELEASE_BUF, /* 释放获取的缓冲图像数据，输入参数为CamImg_t* */
    OPEN_CAM_IOCTL_CLEAR_BUF, /* 清除相机数据缓冲区，输入参数为NULL */
    OPEN_CAM_IOCTL_MAX,
} OpenCamIoctl_e;

typedef enum OpenCamParamId_
{
    OPEN_CAM_PARAM_ID_FPS,
    OPEN_CAM_PARAM_ID_CMOS_MODEL,
    OPEN_CAM_PARAM_ID_CMOS_MAX_FPS,
    OPEN_CAM_PARAM_ID_RESOLUTION,
    OPEN_CAM_PARAM_ID_MIRROR_FLIP,
    OPEN_CAM_PARAM_ID_SCENE,
    OPEN_CAM_PARAM_ID_EV,
    OPEN_CAM_PARAM_ID_CONTRAST,
    OPEN_CAM_PARAM_ID_SATURATION,
    OPEN_CAM_PARAM_ID_SHARP,
    OPEN_CAM_PARAM_ID_AWB,
    OPEN_CAM_PARAM_ID_AE,
    OPEN_CAM_PARAM_ID_MAX,
} OpenCamParamId_e;

typedef struct OpenCamGetBuf_
{
    CamImg_t *img;
    uint32_t timeout_ms;
} OpenCamGetBuf_t;

typedef struct OpenCamParam_
{
    int id;
    int value;
} OpenCamParam_t;

/**
 * @brief 模块初始化，使用api_cam模块需要使能FEATURE_HAL_CAM_ENABLE编译选项
 *
 * 该函数初始化CAM模块。
 * @param para 填入NULL，保留使用。
 * @return  0 成功，其他值失败。
 */
int api_cam_startup(void* para);

/**
 * @brief 探测camera的CMOS型号
 *
 * 该函数用于探测camera的CMOS型号，主要支持单一编译固件支持多种cmos的情况。探测出来的型号需要配置到create函数中。
 * 如果不需要支持多种CMOS，该函数可以不调用，则直接在create函数中填入正确的cmos型号即可。
 * 支持的型号需要再编译时增加需要的编译选项。如需要支持GC032A,则需要增加HAL_CAM_GC032A_BUILTIN选项。
 * 没有包含的CMOS就不会被正确的检出型号。
 * @param cspi_id camera连接的CSPI的ID，默认连接为CSPI1
 * @param ext_pwr_cfg
 * 指向外部电源配置结构体的指针，用于配置外部电源。探测型号需要确保cmos可以通过i2c访问，所以务必保证正确。
 * @param rst_pin_cfg
 * 指向复位引脚配置结构体的指针，用于配置复位引脚。探测型号需要确保cmos可以通过i2c访问，所以务必保证正确。
 * @param i2c_cfg
 * 指向I2C配置结构体的指针，用于配置I2C。探测型号需要确保cmos可以通过i2c访问，所以务必保证正确。
 * @return CamType_e 探测到的CMOS型号，参考CamType_e枚举。
 */
CamType_e api_cam_probe_type(int cspi_id, CamExtPwrCfg_t* ext_pwr_cfg,
                             CamRstPinCfg_t* rst_pin_cfg, CamI2cCfg_t* i2c_cfg);

/**
 * @brief 返回Camera模组的默认配置
 *
 * 该函数用于返回Camera模组的默认配置，这些配置在openref的demo板上都是正确出图的。在使用api_cam_probe_type
 * 探测到的CMOS型号，在调用该函数返回该型号的默认配置。用户的电路板可以在获取的配置上进行必要的修改。
 * @param type camera的型号，参考CamType_e枚举。
 * @param cfg
 * 指向Camera配置结构体的指针，用于返回默认配置。
 * @return int 0 成功，其他值失败。
 */
int api_cam_default(CamType_e type, CamCfg_t* cfg);

/**
 * @brief 创建Camera实例
 *
 * 该函数用于创建一个Camera实例，返回一个用户ID。用户ID用于后续的操作。
 * @param index camera设备的序号。
 * @param cfg
 * 指向Camera配置结构体的指针，用于返回默认配置。
 * @return int 0 成功，其他值失败。
 */
api_ret_t api_cam_create(int index, CamCfg_t* cfg, uint32_t* usrId);

/**
 * @brief 销毁Camera实例
 *
 * 该函数用于销毁一个Camera实例。
 * @param usrId 用户ID，调用api_cam_create生成。。
 * @return int 0 成功，其他值失败。
 */
api_ret_t api_cam_delete(uint32_t usrId);

/**
 * @brief 打开相机采集
 *
 * 该函数用于打开相机采集。
 * @param usrId 用户ID，调用api_cam_create生成。
 * @param cfg 指向Camera配置结构体的指针，用于配置相机采集。
 * @param timeout 超时时间，单位为毫秒。
 * @return int 0 成功，其他值失败。
 */
api_ret_t api_cam_open(uint32_t usrId, CamCfg_t *cfg, size_t timeout);

/**
 * @brief 关闭相机采集
 *
 * 该函数用于关闭相机采集。
 * @param usrId 用户ID，调用api_cam_create生成。。
 * @return int 0 成功，其他值失败。
 */
api_ret_t api_cam_close(uint32_t usrId);

/**
 * @brief 读取一帧图像
 *
 * 该函数用于读取一帧相机图像。
 * @param usrId 用户ID，调用api_cam_create生成。
 * @param buf 图像采集的缓存。
 * @param count 缓存大小。
 * @return int 0 成功，其他值失败。
 */
api_ret_t api_cam_read(uint32_t usrId, void* buf, size_t count);

api_ret_t api_cam_write(uint32_t usrId, void* buf, size_t count);
/**
 * @brief 控制配置图像参数
 *
 * 该函数用于读取一帧相机图像。
 * @param usrId 用户ID，调用api_cam_create生成。
 * @param type 参考OpenCamIoctl_e
 * @param para 控制参数，具体的参考各个type的数值。
 * @return int 0 成功，其他值失败。
 */
api_ret_t api_cam_ioctl(uint32_t usrId, OpenCamIoctl_e type, void* para);

api_ret_t api_cam_pmctl(uint32_t usrId, open_hal_pm_t* cfg, size_t count);

#ifdef __cplusplus
}
#endif
#endif /* _API_CAM_H_ */