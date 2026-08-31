#ifndef __OPENSDK_H__
#define __OPENSDK_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


extern const uint32_t __ocsyscall_app[];
extern const uint32_t psyscall_index;


typedef struct
{
    uint32_t openapiTable;
    uint32_t selfLocation;
    uint32_t openddkTable;
    uint32_t ddkHalDriver;      //任务依赖的外设资源
    uint32_t threadHandle;      //任务控制句柄
    uint32_t functionList;      //相关接口函数结构体获取地址
    uint32_t messageQueue;      //相关数结输出消息队列地址
    uint32_t controlEvent;     
} open_list;

typedef int32_t (*OcSyscallT)(void);


#define OPENSDK_FLAG_EXPORT(flag)               __attribute__((used)) __attribute__((section(".sect_syscalltable_flag"))) const char       __ocsyscall_flag[4]                           = #flag;
#define OPENSDK_FUNC_EXPORT(func, index)        __attribute__((used)) __attribute__((section(".sect_syscalltable_func"))) const OcSyscallT __ocsyscall_##func##_##index                  = (OcSyscallT)func;
#define OPENSDK_FUNC_RESERVED(count, index)     __attribute__((used)) __attribute__((section(".sect_syscalltable_func"))) const OcSyscallT __ocsyscall_reserved_##index##_##count[count] = {0};
#define OPENSDK_APP_EXPORT()                    __attribute__((used)) __attribute__((section(".sect_syscalltable_app")))  const uint32_t   __ocsyscall_app[8]                            = {0x5a5a5a51, 0x5a5a5a52, 0x5a5a5a53, 0x5a5a5a54, 0x5a5a5a55, 0x5a5a5a56, 0x5a5a5a57, 0x5a5a5a58};


void     opensdkInit(void);
int32_t  globalmapInit(uint32_t index, void *data, uint32_t length);
int32_t  globalItemSet(uint32_t index, void *data);
void    *globalItemGet(uint32_t index);


#ifdef __cplusplus
}
#endif
#endif
