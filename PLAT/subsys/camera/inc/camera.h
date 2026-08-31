#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "cmsis_os2.h"
#include "subsys.h"
#include <stdint.h>
#include "hal_cam.h"
#include "api_cam.h"

typedef enum
{
    CAMERA_PIC_MODE,
    CAMERA_REC_MODE,
} camera_mode_t;

typedef enum CAMERA_STATUS_IDLE
{
    CAMERA_STATUS_IDLE = 0,
    CAMERA_STATUS_STARTING,
    CAMERA_STATUS_STARTED,
    CAMERA_STATUS_STOPPING,
    CAMERA_STATUS_STOPPED,
    CAMERA_STATUS_BUSY,
} CamStatus_e;

typedef enum
{
    CAMERA_ERR_NONE,
} camera_err_t;

typedef enum CamSetting_
{
    CAMERA_FPS,
    CAMERA_CMOS_MODEL,
    CAMERA_CMOS_MAX_FPS,
    CAMERA_RESOLUTION,
    CAMERA_MIRROR_FLIP,
    CAMERA_SCENE,
    CAMERA_EV,
    CAMERA_CONTRAST,
    CAMERA_SATURATION,
    CAMERA_SHARP,
    CAMERA_AWB,
    CAMERA_AE,
} CamSetting_e;

typedef enum
{
    CAMERA_NAME,
    CAMERA_MAX_RESOLUTION,
} camera_hw_info_t;

typedef struct ImageData_
{
    uint32_t width;
    uint32_t height;
    CamImgFmt_e fmt;
    uint32_t timestamp;
    uint32_t size;
    uint8_t* data;
} ImageData_t;

typedef void (*data_frame_cb)(ImageData_t* data, void* param);

/**
 * @brief 该函数用于定制化电路板上的相机配置
 *
 * 该函数用于定制化电路板上的相机配置，如果没有调用过该函数，底层会自动使用默认配置，这些默认配置在ref板上可以测试通过，如果
 * camera接口的io发生了变化，需要调用该函数加载定制化的配置。可以重复调用增加多个型号，这样底层会自动检测型号并自动加载正确的配置。
 * @param type camera的型号，参考CamType_e枚举。
 * @param cfg
 * 指向Camera配置结构体的指针，用于返回默认配置。配置的具体内容详见hal_cam.h。
 * @return int 0 成功，其他值失败。
 */
int camAddCfg(CamType_e type, CamCfg_t* cfg);

/**
 * @brief
 * 相机子系统初始化，如果使用相机，需要在初始化时调用该函数。要保证makefile中定义SUBSYS_CAMERA_ENABLE=y
 *
 * 该函数用于初始化相机子系统
 * @return 无
 */
void subCameraInit(void);

/**
 * @brief
 * 相机启动数据采集和传输。
 *
 * 该函数用于启动相机数据采集和传输
 * @return 无
 */
void cameraStart(void);

/**
 * @brief
 * 相机关闭数据采集和传输。
 *
 * 该函数用于关闭相机数据采集和传输
 * @return 无
 */
void cameraStop(void);

/**
 * @brief 该函数用于配置相机的参数
 *
 * 该函数用于于配置相机的参数。
 * @param settings 参数的类型，参考CamSetting_e枚举
 * @param value
 * 参数数值。超出返回会自动限定都在范围内。
 * @return int 0 成功，其他值失败。
 */
int cameraSetSettings(CamSetting_e settings, uint32_t value);

/**
 * @brief 该函数用于获取相机的参数
 *
 * 该函数用于获取相机的参数。
 * @param settings 参数的类型，参考CamSetting_e枚举
 * @param value
 * 参数数值存储地址。
 * @return int 0 成功，其他值失败。
 */
int cameraGetSettings(CamSetting_e settings, uint32_t* value);

/**
 * @brief 该函数用于启动预览功能。
 *
 * 该函数用于启动预览功能。内部会建立一条从cspi到lspi的dma通道。采集的图像数据会自动显示在屏幕上。
 * 分辨率会自动进行裁剪和亚采样。
 * @param screenId 显示的屏幕user id。用api_scr_create()创建的屏幕id。
 * @return 无。
 */
void cameraStartPreview(uint32_t screenId);

/**
 * @brief 该函数用于关闭预览功能。
 *
 * 该函数用于关闭预览功能。
 * @param screenId 显示的屏幕user id。用api_scr_create()创建的屏幕id。
 * @return 无。
 */
void cameraStopPreview(uint32_t screenId);

/**
 * @brief 该函数用于采集图像数据。
 *
 * 该函数用于采集图像数据。可以周期性的调用该函数获取相机的图像数据
 * @param data_frame_cb 图像数据返回的回调函数。
 * @param param 回调函数的参数。
 * @return 无。
 */
void cameraGetFrame(data_frame_cb callback_func, void* param);

/**
 * @brief 该函数用于从环形队列中获取获取相机图像数据地址。
 *
 * 该函数用于从环形队列中获取获取相机图像数据地址,该函数是阻塞函数，会等待超时时间，超时时间为0时为非阻塞函数。
 * @param img 图像数据缓存结构体指针。图像缓存结构体由系统提供地址，不需要进行内存分配
 * @param timeout 超时时间，单位毫秒。
 * @return int 0 成功，其他值失败。
 */
int cameraGetBuf(CamImg_t* img, int timeout);

/**
 * @brief 该函数用于清空相机图像数据缓存。
 *
 * 该函数用于清空相机图像数据缓存。
 * @return int 0 成功，其他值失败。
 */
int cameraClearBuf(void);

/**
 * @brief 该函数用于归还相机图像数据缓存。
 *
 * 该函数用于归还相机图像数据缓存。
 * @param img 图像数据缓存结构体指针。
 * @return int 0 成功，其他值失败。
 */
int cameraReleaseBuf(CamImg_t* img);

/**
 * @brief 该函数用于写入相机寄存器。
 *
 * 该函数用于写入相机寄存器,调试使用，一般可以不调用。
 * @param addr 寄存器地址。
 * @param value 寄存器数值。
 * @return 无。
 */
int cameraWrReg(uint32_t addr, uint32_t value);

/**
 * @brief 该函数用于读取相机寄存器。
 *
 * 该函数用于读取相机寄存器,调试使用，一般可以不调用。
 * @param addr 寄存器地址。
 * @return uint32_t 寄存器数值。
 */
uint32_t cameraRdReg(uint32_t addr);
#endif
