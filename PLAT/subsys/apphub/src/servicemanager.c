#include "servicemanager.h"
#include <string.h>
#include <stdio.h>
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif


// 服务管理器全局变量
static service_info_t *service_list = NULL;
static osMutexId_t service_mutex = NULL;

// 服务管理器初始化
int32_t Service_manager_init(void)
{
    // 创建互斥锁用于线程安全
    osMutexAttr_t mutex_attr = {
        .name = "service_mutex",
        .attr_bits = osMutexRecursive
    };
    
    service_mutex = osMutexNew(&mutex_attr);
    if (service_mutex == NULL) {
        return -1;
    }
    
    return 0;
}

// 查找服务
static service_info_t *find_service(const char *name)
{
    service_info_t *current = service_list;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 注册服务
int32_t Service_reg(const char *name, void (*entry_point)(void *), void *arg,
                   void *cb_mem, uint32_t cb_size, void *stack_mem,
                   uint32_t stack_size, osPriority_t priority)
{
    if (name == NULL || entry_point == NULL) {
        return -1;
    }
    
    osMutexAcquire(service_mutex, osWaitForever);
    
    // 检查服务是否已存在
    if (find_service(name) != NULL) {
        osMutexRelease(service_mutex);
        return -2;
    }
    
    // 创建新的服务节点
    service_info_t *new_service = (service_info_t *)malloc(sizeof(service_info_t));
    if (new_service == NULL) {
        osMutexRelease(service_mutex);
        return -3;
    }
    
    // 初始化服务信息
    strncpy(new_service->name, name, sizeof(new_service->name) - 1);
    new_service->name[sizeof(new_service->name) - 1] = '\0';
    new_service->entry_point = entry_point;
    new_service->arg = arg;
    new_service->thread_id = NULL;
    new_service->state = SERVICE_STATE_REGISTERED;
    new_service->cb_mem = cb_mem;
    new_service->cb_size = cb_size;
    new_service->stack_mem = stack_mem;
    new_service->stack_size = stack_size;
    new_service->priority = priority;
    
    // 初始化统计信息
    memset(&new_service->statistics, 0, sizeof(service_stat_t));
    
    // 添加到链表
    new_service->next = service_list;
    service_list = new_service;
    
    osMutexRelease(service_mutex);
    return 0;
}

// 注销服务
int32_t Service_unreg(const char *name)
{
    if (name == NULL) {
        return -1;
    }
    
    osMutexAcquire(service_mutex, osWaitForever);
    
    service_info_t *current = service_list;
    service_info_t *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // 如果服务正在运行，先停止
            if (current->state == SERVICE_STATE_RUNNING || 
                current->state == SERVICE_STATE_SUSPENDED) {
                Service_stop(name);
            }
            
            // 从链表中移除
            if (prev == NULL) {
                service_list = current->next;
            } else {
                prev->next = current->next;
            }
            
            free(current);
            osMutexRelease(service_mutex);
            return 0;
        }
        prev = current;
        current = current->next;
    }
    
    osMutexRelease(service_mutex);
    return -2; // 服务未找到
}

// 获取服务统计信息
int32_t Service_get_stat(const char *name, service_stat_t *stat)
{
    if (name == NULL || stat == NULL) {
        return -1;
    }
    
    osMutexAcquire(service_mutex, osWaitForever);
    
    service_info_t *service = find_service(name);
    if (service == NULL) {
        osMutexRelease(service_mutex);
        return -2;
    }
    
    memcpy(stat, &service->statistics, sizeof(service_stat_t));
    
    osMutexRelease(service_mutex);
    return 0;
}

// 服务线程包装器
void service_thread_wrapper(void *arg)
{
    service_info_t *service = (service_info_t *)arg;
    
    // 更新状态为运行中
    service->state = SERVICE_STATE_RUNNING;
    service->statistics.start_count++;
    
    // 记录开始时间
    uint32_t start_time = osKernelGetTickCount();
    
    // 执行服务入口函数
    service->entry_point(service->arg);
    
    // 计算运行时间
    uint32_t end_time = osKernelGetTickCount();
    service->statistics.total_runtime_ms += (end_time - start_time);
    
    // 更新状态为停止
    service->state = SERVICE_STATE_STOPPED;
    service->statistics.stop_count++;
    
    // 清理线程资源
    service->thread_id = NULL;
}

// 启动服务
int32_t Service_start(const char *name)
{
    if (name == NULL) {
        return 0;
    }
    
    osMutexAcquire(service_mutex, osWaitForever);
    
    service_info_t *service = find_service(name);
    if (service == NULL) {
        osMutexRelease(service_mutex);
        return 0;
    }
    
    if (service->state == SERVICE_STATE_RUNNING) {
        osMutexRelease(service_mutex);
        return 0; // 服务已在运行
    }
    
    if (service->state == SERVICE_STATE_SUSPENDED) {
        // 如果是挂起状态，恢复运行
        Services_resume(name);
        osMutexRelease(service_mutex);
        return (int32_t)service->thread_id;
    }
    
    // 创建服务线程
    osThreadAttr_t thread_attr = {
        .name = service->name,
        .cb_mem = service->cb_mem,
        .cb_size = service->cb_size,
        .stack_mem = service->stack_mem,
        .stack_size = service->stack_size,
        .priority = service->priority
    };
    
    service->thread_id = osThreadNew(service_thread_wrapper, service, &thread_attr);
    if (service->thread_id == NULL) {
        service->state = SERVICE_STATE_ERROR;
        service->statistics.error_count++;
        osMutexRelease(service_mutex);
        return 0;
    }
    
    osMutexRelease(service_mutex);
    return (int32_t)service->thread_id;
}

// 停止服务
int32_t Service_stop(const char *name)
{
    if (name == NULL) {
        return -1;
    }
    
    osMutexAcquire(service_mutex, osWaitForever);
    
    service_info_t *service = find_service(name);
    if (service == NULL) {
        osMutexRelease(service_mutex);
        return -2;
    }
    
    if (service->thread_id != NULL) {
        // 终止线程
        if (osThreadGetId() == service->thread_id) {
            service->state = SERVICE_STATE_STOPPED;
            service->statistics.stop_count++;
            service->thread_id = NULL;
            osMutexRelease(service_mutex);
            osThreadTerminate(osThreadGetId());
            return 0;
        }
        else
        {
            osThreadTerminate(service->thread_id);
            service->thread_id = NULL;
        }
    }
    
    service->state = SERVICE_STATE_STOPPED;
    service->statistics.stop_count++;
    
    osMutexRelease(service_mutex);
    return 0;
}

// 列出所有服务
int32_t Services_list(void)
{
    osMutexAcquire(service_mutex, osWaitForever);
    
    service_info_t *current = service_list;
    uint32_t count = 0;
    
    printf("=== Service List ===\n");
    while (current != NULL) {
        printf("Service: %s\n", current->name);
        printf("  State: %d\n", current->state);
        printf("  Thread ID: %p\n", current->thread_id);
        printf("  Start Count: %u\n", current->statistics.start_count);
        printf("  Total Runtime: %llu ms\n", current->statistics.total_runtime_ms);
        printf("-------------------\n");
        
        count++;
        current = current->next;
    }
    
    printf("Total services: %u\n", count);
    
    osMutexRelease(service_mutex);
    return count;
}

// 挂起服务
int32_t Services_suspend(const char *name)
{
    osMutexAcquire(service_mutex, osWaitForever);
    
    int32_t result = 0;
    service_info_t *current = service_list;
    
    if (name == NULL || strlen(name) == 0) {
        // 挂起所有运行中的服务
        while (current != NULL) {
            if (current->state == SERVICE_STATE_RUNNING && current->thread_id != NULL) {
                SYSLOG_DEBUG("name=%s\r\n", current->name);
                osThreadSuspend(current->thread_id);
                current->state = SERVICE_STATE_SUSPENDED;
                current->statistics.suspend_count++;
                result++; // 记录成功挂起的服务数量
            }
            current = current->next;
        }
    } else {
        // 挂起指定名称的服务
        service_info_t *service = find_service(name);
        if (service == NULL) {
            osMutexRelease(service_mutex);
            return -2;
        }
        
        if (service->state != SERVICE_STATE_RUNNING) {
            osMutexRelease(service_mutex);
            return -3; // 只有运行中的服务才能挂起
        }
        
        if (service->thread_id != NULL) {
            osThreadSuspend(service->thread_id);
            service->state = SERVICE_STATE_SUSPENDED;
            service->statistics.suspend_count++;
            result = 1; // 成功挂起一个服务
        }
    }
    
    osMutexRelease(service_mutex);
    return result;
}

// 恢复服务
int32_t Services_resume(const char *name)
{
    osMutexAcquire(service_mutex, osWaitForever);
    
    int32_t result = 0;
    service_info_t *current = service_list;
    
    if (name == NULL || strlen(name) == 0) {
        // 恢复所有挂起的服务
        while (current != NULL) {
            if (current->state == SERVICE_STATE_SUSPENDED && current->thread_id != NULL) {
                osThreadResume(current->thread_id);
                current->state = SERVICE_STATE_RUNNING;
                current->statistics.resume_count++;
                result++; // 记录成功恢复的服务数量
            }
            current = current->next;
        }
    } else {
        // 恢复指定名称的服务
        service_info_t *service = find_service(name);
        if (service == NULL) {
            osMutexRelease(service_mutex);
            return -2;
        }
        
        if (service->state != SERVICE_STATE_SUSPENDED) {
            osMutexRelease(service_mutex);
            return -3; // 只有挂起的服务才能恢复
        }
        
        if (service->thread_id != NULL) {
            osThreadResume(service->thread_id);
            service->state = SERVICE_STATE_RUNNING;
            service->statistics.resume_count++;
            result = 1; // 成功恢复一个服务
        }
    }
    
    osMutexRelease(service_mutex);
    return result;
}
