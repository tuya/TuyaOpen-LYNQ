/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of EigenComm Ltd.
 * File name:    api_pad.h
 * Description:  ec7xx openhal pad entry header file
 * History:      Rev1.0   2024-01-22
 *
 ****************************************************************************/
#ifndef  _API_PAD_H_
#define  _API_PAD_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "api_def.h"

#define EC_PAD_INDEX_START    (11)
#define EC_PAD_INDEX_LIMIT    (54)

typedef enum {
    OPEN_PAD_IOCTL_FUNC = 0,    
    OPEN_PAD_IOCTL_PULLUP ,   
    OPEN_PAD_IOCTL_PULLDOWN , 
    OPEN_PAD_IOCTL_PULLSELECT ,           
} api_pad_ioctl_t;

typedef struct{
    // 前面的参数和寄存器同名配置但位域不同
    uint32_t    mux                 : 3;   
    uint32_t    pullUpEnable        : 1;   
    uint32_t    pullDownEnable      : 1;   
    uint32_t    pullSelect          : 1;   
    uint32_t    inputForceDisable   : 1;   
    uint32_t    inputControl        : 1;   
    uint32_t    outputControl       : 1;   
    uint32_t    outputForceDisable  : 1;   
    uint32_t    slewRate            : 1;   
    uint32_t    driveStrength       : 3;   
    uint32_t    swOutputValue       : 1;   
    uint32_t    swOutputEnable      : 1;   
    // 之后添加更多用户配置项
    uint32_t    locked              : 1;    // 如果锁定将不再被后续覆盖配置
    uint32_t    reserved            : 15;
} pad_config_t; // 16bit + 16bit

/**
  \fn          int8_t *api_pad_startup(int8_t *para)
  \brief       全局初始化，传入的参数需要是完整的pad表而非某个pad配置项，便于默认修改参数统一int8_t
  \param[in]   para  指向PAD配置参数数组的指针
  \return      返回指向PAD状态列表的指针
  \details     该函数用于在系统上电时初始化所有PAD设备。它会遍历所有可用的PAD索引，
               并根据传入的配置参数进行初始化设置。如果para不为NULL，则会使用传入的参数更新默认配置。
*/
int8_t *api_pad_startup(int8_t *para);

/**
  \fn          api_ret_t api_pad_setup(int8_t index, pad_config_t* para)
  \brief       单项初始化，使用HAL统一的参数格式
  \param[in]   index  PAD索引编号
  \param[in]   para   PAD配置参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于对指定的PAD进行配置，包括MUX功能选择、上拉/下拉配置等参数设置。
*/
api_ret_t api_pad_setup(int8_t index,pad_config_t* para);

/**
  \fn          void api_pad_default(int8_t (*list)[4], uint8_t count)
  \brief       重置padList到默认状态
  \param[in]   list   指向PAD列表的指针
  \param[in]   count  PAD列表中的元素数量
  \return      无
  \details     该函数用于将PAD列表重置为默认状态。
*/
void api_pad_default(int8_t (*list)[4], uint8_t count);

/**
  \fn          api_ret_t api_pad_create(uint32_t paddr, void *out)
  \brief       创建PAD设备实例，分配资源并检查依赖条件
  \param[in]   paddr  PAD物理地址 (11-53)
  \param[out]  out    输出参数，返回创建的PAD设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于创建一个新的PAD设备实例，分配相关资源并进行初始化。
*/
api_ret_t api_pad_create(uint32_t paddr, void *out);

/**
  \fn          api_ret_t api_pad_delete(uint32_t usrId)
  \brief       删除PAD设备实例，释放相关资源
  \param[in]   usrId  PAD设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于删除指定的PAD设备实例，释放相关资源。
*/
api_ret_t api_pad_delete(uint32_t usrId);

/**
  \fn          api_ret_t api_pad_open(uint32_t usrId, void *config, size_t timeout)
  \brief       打开PAD设备
  \param[in]   usrId    PAD设备ID
  \param[in]   config   PAD配置参数指针（可为NULL）
  \param[in]   timeout  超时时间（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于打开PAD设备并根据配置参数进行设置。
*/
api_ret_t api_pad_open(uint32_t usrId, void *config, size_t timeout);

/**
  \fn          api_ret_t api_pad_apply(uint32_t usrId, void *cb)
  \brief       占用pad并回调，依赖rtos
  \param[in]   usrId  PAD设备ID
  \param[in]   cb     回调函数指针
  \return      执行结果
  \details     系统级接口，非阻塞，获取pad成功后回调，可用于共用外设端口情况
*/
api_ret_t api_pad_apply(uint32_t usrId, void *cb);

/**
  \fn          api_ret_t api_pad_close(uint32_t usrId)
  \brief       关闭PAD设备
  \param[in]   usrId  PAD设备ID
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于关闭指定的PAD设备。
*/
api_ret_t api_pad_close(uint32_t usrId);

/**
  \fn          api_ret_t api_pad_ioctl(uint32_t usrId, api_pad_ioctl_t type, void *para)
  \brief       PAD设备控制接口，用于配置设备的各种参数
  \param[in]   usrId  PAD设备ID
  \param[in]   type   控制类型，参考api_pad_ioctl_t枚举
  \param[in]   para   控制参数指针
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于对PAD设备进行各种控制操作，如设置功能、上拉/下拉电阻等。
*/
api_ret_t api_pad_ioctl(uint32_t usrId, api_pad_ioctl_t type, void *para);

/**
  \fn          api_ret_t api_pad_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count)
  \brief       对设备功耗和模式进行配置
  \param[in]   usrId  PAD设备ID
  \param[in]   cfg    功耗配置参数指针
  \param[in]   count  参数数量（暂未使用）
  \return      执行结果，OPEN_HAL_DONE表示成功
  \details     该函数用于控制PAD设备的功耗模式。
*/
api_ret_t api_pad_pmctl(uint32_t usrId, open_hal_pm_t *cfg, size_t count);

/**
  \fn          api_ret_t api_pad_write(uint32_t usrId, void* buf, size_t count)
  \brief       对设备进行写操作  
  \param[in]   usrId   PAD设备ID
  \param[in]   buf     要写入的数据缓冲区指针（暂未使用）
  \param[in]   count   要写入的数据大小（字节数，暂未使用）
  \return      执行结果
  \details     该函数用于向PAD设备写入数据。目前尚未实现具体功能。
  \note        待实现：需要根据具体应用场景实现PAD写操作
*/
api_ret_t api_pad_write(uint32_t usrId, void* buf, size_t count);

/**
  \fn          api_ret_t api_pad_read(uint32_t usrId, void* buf, size_t count)
  \brief       对设备进行读操作   
  \param[in]   usrId   PAD设备ID
  \param[out]  buf     读取数据的缓冲区指针（暂未使用）
  \param[in]   count   要读取的数据大小（字节数，暂未使用）
  \return      执行结果
  \details     该函数用于从PAD设备读取数据。目前尚未实现具体功能。
  \note        待实现：需要根据具体应用场景实现PAD读操作
*/
api_ret_t api_pad_read(uint32_t usrId, void* buf, size_t count);

/**
  \fn          api_ret_t api_pad_query(uint32_t usrId)
  \brief       查询PAD设备状态
  \param[in]   usrId  PAD设备ID或物理地址
  \return      PAD设备当前状态
  \details     该函数用于查询指定PAD设备的当前状态（空闲、使用中等）。
*/
api_ret_t api_pad_query(uint32_t usrId);

/**
  \fn          int api_test_pad(void)
  \brief       PAD设备测试接口
  \return      测试结果
  \details     该函数用于测试PAD设备的基本功能。
*/
int api_test_pad(void);

#define CSV_PAD_ITEM_MAX    (7)
/**
  \fn          int32_t api_pad_parse(char* str, pad_config_t *cfg)
  \brief       解析PAD配置字符串
  \param[in]   str   配置字符串
  \param[out]  cfg   解析后的配置参数结构体指针
  \return      PAD索引编号
  \details     该函数用于解析CSV格式的PAD配置字符串，并填充到配置结构体中。
*/
int32_t api_pad_parse(char* str, pad_config_t *cfg);

/**
  \fn          api_ret_t check_pad_mux(int8_t paddr, int8_t mux)
  \brief       检查PAD的MUX配置
  \param[in]   paddr  PAD物理地址
  \param[in]   mux    MUX功能选择
  \return      检查结果，OPEN_HAL_INVALID_PARA表示失败，其他值表示成功
  \details     该函数用于查询确认底层PAD功能是否配置正确。
*/
api_ret_t check_pad_mux(int8_t paddr, int8_t mux);     // 用于查询确认底层PAD功能是否配置
#ifdef __cplusplus
}
#endif
#endif /* _API_PAD_H_ */