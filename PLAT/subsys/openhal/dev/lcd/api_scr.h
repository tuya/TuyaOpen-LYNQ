/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_scr.h
 * Description:  ec7xx openHAL屏幕接口入口头文件
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
#include "api_def.h"
#include "hal_screen.h"

/**
 * @def EC_SCR_INDEX_START
 * @brief 屏幕实例索引起始值
 * @details 定义屏幕设备实例的起始索引，用于API接口调用时标识设备
 */
#define EC_SCR_INDEX_START 0

/**
 * @def EC_SCR_INDEX_LIMIT
 * @brief 屏幕实例索引限制值
 * @details 定义屏幕设备实例的索引上限，用于API接口调用时验证索引有效性
 */
#define EC_SCR_INDEX_LIMIT 1

/**
 * @enum OpenScreenIoctl_e
 * @brief 屏幕IO控制命令枚举
 * @details 定义了屏幕设备支持的各种控制命令，用于api_scr_ioctl函数
 */
typedef enum OpenScreenIoctl_
{
    OPEN_SCREEN_BACKLIGHT_SET = 0,    /**< 设置屏幕背光级别 */
    OPEN_SCREEN_BACKLIGHT_GET,        /**< 获取屏幕背光级别 */
    OPEN_SCREEN_DIRECTION,            /**< 设置屏幕显示方向 */
    OPEN_SCREEN_RST_CLEAR_FIFO,       /**< 重置并清除FIFO */
    OPEN_SCREEN_CAM_PREVIEW,          /**< 配置相机预览 */
    OPEN_SCREEN_SET_REFRESH_DIR,      /**< 设置刷新方向 */
    OPEN_SCREEN_SET_DISPLAY_MODE,     /**< 设置显示模式 */
    OPEN_SCREEN_SET_DISPLAY_PIX_MODE, /**< 设置显示像素模式 */
    OPEN_SCREEN_SET_DMA_CALLBACK,     /**< 设置DMA回调函数 */
    OPEN_SCREEN_SET_USP_CALLBACK,     /**< 设置USP回调函数 */
    OPEN_SCREEN_SET_LCDIO,            /**< 设置LCD IO控制 */
    OPEN_SCREEN_SET_LCD_WINDOW,       /**< 设置LCD显示窗口 */
    OPEN_SCREEN_BACKLIGHT_OPEN,       /**< 打开屏幕背光 */
    OPEN_SCREEN_BACKLIGHT_CLOSE,      /**< 关闭屏幕背光 */
} OpenScreenIoctl_e;

typedef struct LcdDispWin_
{
    uint32_t start_x;
    uint32_t start_y;
    uint32_t width;
    uint32_t height;
} LcdDispWin_t;

/**
 * @enum OpenScreenDataFmt_e
 * @brief 屏幕数据格式枚举
 * @details 定义了屏幕支持的数据格式，目前仅支持RGB565格式
 */
typedef enum OpenScreenDataFmt_
{
    OPEN_SCREEN_DATA_FMT_RGB565 = 0, /**< RGB565格式 (16位色) */
} OpenScreenDataFmt_e;

/**
 * @struct ScrWriteParam_t
 * @brief 屏幕写入参数结构体
 * @details 用于api_scr_write函数，定义了写入屏幕的数据参数
 */
typedef struct ScrWriteParam_
{
    OpenScreenDataFmt_e fmt; /**< 数据格式 */
    uint32_t start_x;        /**< 写入起始X坐标 */
    uint32_t start_y;        /**< 写入起始Y坐标 */
    uint32_t width;          /**< 写入宽度 */
    uint32_t height;         /**< 写入高度 */
    uint8_t *data;           /**< 数据指针 */
    uint32_t size;           /**< 数据大小(字节) */
} ScrWriteParam_t;

/**
 * @brief 屏幕模块启动函数
 * @param para 启动参数（保留未使用）
 * @return 成功返回0，失败返回错误码
 * @details 初始化屏幕模块所需的资源和配置
 */
int api_scr_startup(void *para);

/**
 * @brief 获取屏幕默认配置
 * @param type 屏幕类型
 * @param cfg 输出参数，用于存储默认配置
 * @return 成功返回API_OK，失败返回错误码
 * @details 根据屏幕类型获取对应的默认配置参数
 */
api_ret_t api_scr_default(ScreenType_e type, ScrConfig_t *cfg);

/**
 * @brief 创建屏幕实例
 * @param index 屏幕索引
 * @param cfg 屏幕配置
 * @param out 输出参数，用于存储创建成功的用户ID
 * @return 成功返回API_OK，失败返回错误码
 * @details 根据配置创建一个屏幕实例并返回用户ID
 */
api_ret_t api_scr_create(uint32_t index, ScrConfig_t *cfg, void *out);

/**
 * @brief 删除屏幕实例
 * @param usrId 用户ID
 * @return 成功返回API_OK，失败返回错误码
 * @details 根据用户ID删除对应的屏幕实例，释放相关资源
 */
api_ret_t api_scr_delete(uint32_t usrId);

/**
 * @brief 打开屏幕设备
 * @param usrId 用户ID
 * @param cfg 屏幕配置
 * @param timeout 超时时间
 * @return 成功返回API_OK，失败返回错误码
 * @details 根据用户ID打开对应的屏幕设备，配置屏幕参数
 */
api_ret_t api_scr_open(uint32_t usrId, ScrConfig_t *cfg, size_t timeout);

/**
 * @brief 关闭屏幕设备
 * @param usrId 用户ID
 * @return 成功返回API_OK，失败返回错误码
 * @details 关闭指定用户ID的屏幕设备，停止相关操作
 */
api_ret_t api_scr_close(uint32_t usrId);

/**
 * @brief 从屏幕读取数据
 * @param usrId 用户ID
 * @param buf 数据缓冲区
 * @param count 要读取的数据量
 * @return 成功返回API_OK，失败返回错误码
 * @details 从屏幕读取指定数量的数据到缓冲区
 */
api_ret_t api_scr_read(uint32_t usrId, void *buf, size_t count);

/**
 * @brief 向屏幕写入数据
 * @param usrId 用户ID
 * @param buf 数据缓冲区，通常为ScrWriteParam_t类型指针
 * @param count 缓冲区大小
 * @return 成功返回API_OK，失败返回错误码
 * @details 根据ScrWriteParam_t参数向屏幕指定区域写入数据
 */
api_ret_t api_scr_write(uint32_t usrId, void *buf, size_t count);

/**
 * @brief 屏幕IO控制
 * @param usrId 用户ID
 * @param type 控制命令类型
 * @param para 命令参数
 * @return 成功返回API_OK，失败返回错误码
 * @details 执行各种屏幕控制命令，如设置背光、方向等
 */
api_ret_t api_scr_ioctl(uint32_t usrId, OpenScreenIoctl_e type, void *para);

/**
 * @brief 屏幕电源管理控制
 * @param usrId 用户ID
 * @param cfg 电源管理配置
 * @param count 配置大小
 * @return 成功返回API_OK，失败返回错误码
 * @details 配置屏幕的电源管理模式，如挂起、恢复等
 */
api_ret_t api_scr_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count);

/**
 * @brief 查询屏幕状态
 * @param usrId 用户ID
 * @return 成功返回屏幕状态码，失败返回错误码
 * @details 查询指定屏幕的当前工作状态
 */
api_ret_t api_scr_query(uint32_t usrId);

#ifdef __cplusplus
}
#endif
#endif /* _API_SCR_H_ */