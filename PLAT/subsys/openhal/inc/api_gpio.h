/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_gpio.h
 * Description:  openhal gpio entry header file
 * History:      Rev1.0   2024-01-10
 *
 ****************************************************************************/
#ifndef  _API_GPIO_H_
#define  _API_GPIO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"
#define EC_GPIO_INDEX_START         (0)
#define EC_GPIO_INDEX_LIMIT         (39)

#define GPIO_PIN_0                 ((uint16_t)0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint16_t)0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint16_t)0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint16_t)0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint16_t)0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint16_t)0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint16_t)0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint16_t)0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint16_t)0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint16_t)0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint16_t)0x0400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint16_t)0x2000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint16_t)0x4000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint16_t)0x8000)  /* Pin 15 selected   */
#define GPIO_PIN_All               ((uint16_t)0xFFFF)  /* All pins selected */

typedef enum {
    OPEN_GPIO_IOCTL_DIR_OUT = 0,
    OPEN_GPIO_IOCTL_OUT_ACT ,       //active level: 当前定义逻辑，如果配置0（默认值）则读写为实际值，如果这个配置被重新配置1，则对应GPIO读写都会进行一次反向
    OPEN_GPIO_IOCTL_DIR_IN  ,
    OPEN_GPIO_IOCTL_ISR_CB  ,       //输入中断回调
    OPEN_GPIO_IOCTL_LOCKED  ,
} api_gpio_ioctl_t;

typedef struct {
    uint32_t Locked     : 1;
    uint32_t Active     : 1;
    uint32_t Direction  : 1;
    uint32_t Default    : 3;    // 上电状态
    uint32_t Latch      : 1;
    uint32_t mode       : 4;
    uint32_t port       : 2;
    uint32_t pin        : 4;
} gpio_config_t;

typedef struct {
    int (*setDirection)(void*);
    int (*setActiveType)(void*);
    int (*setGpioCallback)(void*);
    int (*setEdgeTriggerType)(void*);
}api_gpio_inf, *api_gpio_infp;

/**
  \fn          int8_t *api_gpio_startup(int8_t *pin, int8_t *pad)
  \brief       系统上电时初始化所有GPIO引脚至指定配置状态
  \param[in]   pin    指向GPIO配置参数数组的指针
  \param[in]   pad    指向PAD电气特性配置参数数组的指针
  \return      指向初始化后GPIO状态列表的指针；若失败则返回 NULL
  \details     该函数在系统启动阶段调用，用于批量初始化所有GPIO引脚。
               根据传入的 pin 和 pad 配置数组，完成方向、电平、驱动能力等设置。
*/
int8_t *api_gpio_startup(int8_t *pin, int8_t *pad);

/**
  \fn          api_ret_t api_gpio_setup(int8_t index, gpio_config_t* para)
  \brief       使用HAL统一格式配置单个GPIO引脚
  \param[in]   index  GPIO索引编号（范围：0 - 38）
  \param[in]   para   指向gpio_config_t结构体的配置参数指针
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数对指定索引的GPIO进行独立配置，支持方向、默认输出值、锁存、模式等属性设置。
               是底层硬件初始化的核心接口之一。
*/
api_ret_t api_gpio_setup(int8_t index, gpio_config_t* para);

//TODO:use gpio enum
/**
  \fn          api_ret_t api_gpio_create(uint32_t index, void *cfg, void *out)
  \brief       创建指定GPIO的设备实例并分配资源
  \param[in]   index  GPIO索引编号（范围：0 - 38）
  \param[in]   cfg    可选配置参数指针（可为 NULL，使用默认配置）
  \param[out]  out    输出参数，用于返回分配的GPIO设备句柄（ID）
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数为指定GPIO创建一个设备实例，完成资源分配与初步初始化。
               设备ID通过 out 参数返回，后续操作需使用该句柄。
*/
api_ret_t api_gpio_create(uint32_t index, void *cfg, void *out);

/**
  \fn          api_ret_t api_gpio_delete(uint32_t usrId)
  \brief       删除GPIO设备实例并释放其占用资源
  \param[in]   usrId  要删除的GPIO设备ID
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数销毁由 api_gpio_create 创建的设备实例，
               并回收其关联的内存及硬件资源，防止资源泄漏。
*/
api_ret_t api_gpio_delete(uint32_t usrId);

/**
  \fn          api_ret_t api_gpio_open(uint32_t usrId, void *cfg, size_t timeout)
  \brief       打开已创建的GPIO设备实例
  \param[in]   usrId    GPIO设备ID
  \param[in]   cfg      可选配置参数指针（可为 NULL）
  \param[in]   timeout  操作超时时间（当前未启用）
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数激活指定的GPIO设备，可重新应用配置参数。
               是访问GPIO前必须调用的操作，确保设备处于就绪状态。
*/
api_ret_t api_gpio_open(uint32_t usrId,void *cfg,size_t timeout);

/**
  \fn          api_ret_t api_gpio_close(uint32_t usrId)
  \brief       关闭已打开的GPIO设备实例
  \param[in]   usrId  GPIO设备ID
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数停止对GPIO设备的访问，将其置于非活动状态。
               与 api_gpio_open 成对使用，保证资源使用的安全性。
*/
api_ret_t api_gpio_close(uint32_t usrId);

/**
  \fn          api_ret_t api_gpio_ioctl(uint32_t usrId, api_gpio_ioctl_t type, void *para)
  \brief       对GPIO设备执行控制命令（IOCTL）
  \param[in]   usrId  GPIO设备ID
  \param[in]   type   控制命令类型，参见 api_gpio_ioctl_t 枚举定义
  \param[in]   para   命令所需的参数指针（依 type 而定）
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数提供灵活的GPIO控制能力，例如设置方向、电平极性、注册中断回调等。
               是实现动态配置和事件响应的关键接口。
*/
api_ret_t api_gpio_ioctl(uint32_t usrId,api_gpio_ioctl_t type, void *para);

/**
  \fn          api_ret_t api_gpio_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
  \brief       配置GPIO设备的电源管理策略
  \param[in]   usrId  GPIO设备ID
  \param[in]   cfg    指向电源管理配置结构体的指针
  \param[in]   count  配置项数量（当前保留未用）
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数用于设置GPIO在低功耗模式下的行为，如休眠时的状态保持或唤醒能力。
               支持系统级节能策略的实现。
*/
api_ret_t api_gpio_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);

/**
  \fn          api_ret_t api_gpio_write(uint32_t usrId, void* buf, size_t count)
  \brief       向GPIO输出引脚写入电平数据
  \param[in]   usrId   GPIO设备ID
  \param[in]   buf     指向待写入数据的缓冲区（通常为 uint8_t*）
  \param[in]   count   数据长度（以字节为单位）
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数将缓冲区中的值写入指定GPIO引脚，用于控制外部电路状态。
               仅适用于配置为输出方向的GPIO。
*/
api_ret_t api_gpio_write(uint32_t usrId, void* buf, size_t count);

/**
  \fn          api_ret_t api_gpio_read(uint32_t usrId, void* buf, size_t count)
  \brief       从GPIO输入引脚读取电平数据
  \param[in]   usrId   GPIO设备ID
  \param[out]  buf     指向存储读取数据的缓冲区
  \param[in]   count   请求读取的数据长度（字节数）
  \return      API执行状态码，成功时返回 OPEN_HAL_DONE
  \details     该函数读取指定GPIO引脚的当前电平状态（高/低），并存入缓冲区。
               仅适用于配置为输入方向的GPIO。
*/
api_ret_t api_gpio_read(uint32_t usrId, void* buf, size_t count);

/**
  \fn          api_ret_t api_gpio_query(uint32_t usrId)
  \brief       查询GPIO设备的当前运行状态
  \param[in]   usrId  GPIO设备ID
  \return      返回设备状态码（如：空闲、忙、已关闭等）
  \details     该函数用于获取指定GPIO设备的实时状态信息，辅助进行资源管理和调试。
               可判断设备是否正被其他任务占用。
*/
api_ret_t api_gpio_query(uint32_t usrId);

/**
  \fn          int api_test_gpio(void)
  \brief       执行GPIO模块自检与功能性测试
  \return      测试结果码：0 表示通过，非0 表示错误代码
  \details     该函数运行一系列内置测试用例，验证GPIO读写、中断、配置等功能是否正常。
               主要用于生产测试或系统诊断场景。
*/
int api_test_gpio(void);

#define CSV_GPIO_ITEM_MAX   (6)
/**
  \fn          int32_t api_gpio_parse(char* str, gpio_config_t *cfg)
  \brief       解析CSV格式的GPIO配置字符串
  \param[in]   str   输入的配置字符串（CSV格式）
  \param[out]  cfg   指向输出配置结构体的指针，用于存储解析结果
  \return      成功时返回对应的GPIO索引号；失败时返回负值错误码
  \details     该函数将形如 "index,mode,direction..." 的CSV字符串解析为 gpio_config_t 结构体，
               便于通过文本配置快速初始化GPIO。
*/
int32_t api_gpio_parse(char* str, gpio_config_t *cfg);
#ifdef __cplusplus
}
#endif
#endif /* _API_GPIO_H_ */