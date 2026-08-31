#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "cmsis_os2.h"
#include "subsys.h"
#include <stdint.h>
#include "api_scr.h"
#include "hal_screen.h"

typedef enum DISPLAY_STATUS_IDLE
{
    DISPLAY_STATUS_IDLE = 0,
    DISPLAY_STATUS_STARTING,
    DISPLAY_STATUS_STARTED,
    DISPLAY_STATUS_STOPPING,
    DISPLAY_STATUS_STOPPED,
    DISPLAY_STATUS_BUSY,
} DisplayStatus_e;

typedef struct DisplayRegion_
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} DisplayRegion_t;

typedef void (*display_transfer_done_cb)(uint32_t);

/**
 * @brief
 * 显示子系统初始化，如果使用屏幕显示，需要在初始化时调用该函数。要保证makefile中定义SUBSYS_DISPLAY_ENABLE=y
 *
 * 该函数用于初始化显示子系统
 * @return 无
 */
void subDisplayInit(void);


/**
 * @brief
 * 显示子系统参数，每个电路板都需要对指定类型的型号添加配置，该函数在subDisplayInit之后调用。在displayStart之前调用
 *
 * 该函数用于给显示子系统添加配置参数
 * @return 无
 */
int displayAddCfg(ScreenType_e type, ScrConfig_t* cfg);

/**
 * @brief
 * 启动屏幕显示功能。
 *s
 * 该函数用于启动屏幕的显示功能
 * @param type 屏幕型号
 * @param display_transfer_done_cb 传输完成回调函数
 * @return 无
 */
void displayStart(ScreenType_e type, display_transfer_done_cb cb);

/**
 * @brief
 * 关闭屏幕显示功能。
 *
 * 该函数用于关闭屏幕的显示功能
 * @return 无
 */
int displayStop(void);

/**
 * @brief
 * 配置屏幕背光功能。
 *
 * 该函数用于配置屏幕的背光功能
 * @param backlight 背光亮度，0为关闭，100为最大亮度
 * @return 无
 */
void displaySetBacklight(uint8_t backlight);

/**
 * @brief
 * 向屏幕写入数据。
 *
 * 该函数用于向屏幕写入数据
 * @param data 要写入的数据指针
 * @param size 要写入的数据大小
 * @return int 0 成功，其他值失败。
 */
int displayWriteData(uint8_t* data, uint32_t size, DisplayRegion_t *region);


/**
 * @brief
 * 配置屏幕像素显示模式是RGB还是BGR。
 *
 * 该函数用于配置屏幕像素的显示模式
 * @param mode 显示模式，参考DisplayPixMode_e。
 * @return int 0 成功，其他值失败。
 */
int displaySetPixMode(DisplayPixMode_e mode);

#endif
