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

#define EC_PAD_INDEX_START      		(11)
#define EC_PAD_INDEX_LIMIT              (54)

typedef enum {
    OPEN_PAD_IOCTL_FUNC = 0,    
    OPEN_PAD_IOCTL_PULLUP ,   
    OPEN_PAD_IOCTL_PULLDOWN , 
    OPEN_PAD_IOCTL_PULLSELECT ,           
} api_pad_ioctl_t;
//如果参数mux值为7表示该pad禁用
typedef struct
{
    uint8_t         mux                        : 3; 
    uint8_t         pullUpEnable               : 1;   
    uint8_t         pullDownEnable             : 1;   
    uint8_t         pullSelect                 : 1;   
    uint8_t         inputControl               : 1;   
    uint8_t         outputControl              : 1;  
} PadConfigLite_t;

//上电加载默认配置，读存储区配置默认值，返回初始化的PAD数量
int api_pad_startup(void* para);
uint32_t api_pad_default(char* str,PadConfigLite_t *cfg);
//申请pad端口并更新配置，返回用户操作ID（应用层区分）
uint32_t api_pad_create(uint32_t paddr,void *config);
api_ret_t api_pad_delete(uint32_t usrId);
//申请等待pad资源
api_ret_t api_pad_open(uint32_t usrId,void *config,size_t timeout);
api_ret_t api_pad_apply(uint32_t usrId,void *cb);
api_ret_t api_pad_close(uint32_t usrId);
//配置接口
api_ret_t api_pad_ioctl(uint32_t usrId,api_pad_ioctl_t type, void *para);
api_ret_t api_pad_pmctl(uint32_t usrId,open_hal_pm_t *cfg, size_t count);
//查询接口，返回枚举量
api_ret_t api_pad_query(uint32_t usrId);
//测试接口
int api_test_pad(void);
#ifdef __cplusplus
}
#endif
#endif /* _API_PAD_H_ */