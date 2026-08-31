#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

// 服务状态定义
typedef enum {
    SERVICE_STATE_UNREGISTERED = 0,
    SERVICE_STATE_REGISTERED,
    SERVICE_STATE_RUNNING,
    SERVICE_STATE_SUSPENDED,
    SERVICE_STATE_STOPPED,
    SERVICE_STATE_ERROR
} service_state_t;

// 服务统计信息
typedef struct {
    uint32_t start_count;
    uint32_t stop_count;
    uint32_t suspend_count;
    uint32_t resume_count;
    uint32_t error_count;
    uint64_t total_runtime_ms;
} service_stat_t;

// 服务信息结构
typedef struct service_info {
    char name[32];                  // 服务名称
    void (*entry_point)(void *);    // 服务入口函数
    void *arg;                      // 服务参数
    osThreadId_t thread_id;         // 服务线程ID
    service_state_t state;          // 服务状态
    service_stat_t statistics;      // 服务统计信息
    void *cb_mem;                   // memory for control block
    uint32_t cb_size;               // size of provided memory for control block
    void *stack_mem;                // memory for stack
    uint32_t stack_size;            // 栈大小
    osPriority_t priority;          // 优先级
    struct service_info *next;      // 下一个服务节点
} service_info_t;

// 服务管理器API
int32_t Service_manager_init(void);
int32_t Service_reg(const char *name, void (*entry_point)(void *), void *arg, 
                   void *cb_mem, uint32_t cb_size, void *stack_mem,
                   uint32_t stack_size, osPriority_t priority);
int32_t Service_unreg(const char *name);
int32_t Service_get_stat(const char *name, service_stat_t *stat);
int32_t Service_start(const char *name);
int32_t Service_stop(const char *name);
int32_t Services_list(void);
int32_t Services_suspend(const char *name);
int32_t Services_resume(const char *name);

// 内部使用的线程函数
void service_thread_wrapper(void *arg);

#endif // SERVICE_MANAGER_H
