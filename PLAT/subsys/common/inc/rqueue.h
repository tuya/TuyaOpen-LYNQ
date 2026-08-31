/**
 * @file rqueue.h
 * @brief 环形队列（Ring Queue）实现的头文件
 * @details 提供了线程安全的环形队列实现，支持自动扩容和多种锁机制
 */
#ifndef __RQUEUE_H__
#define __RQUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"

/**
 * @brief 环形队列操作结果枚举
 */
typedef enum RQueueErr_
{
    RQ_SUCCESS = 0,           /*!< 操作成功 */
    RQ_ERR_INVALIDPARAM = -1, /*!< 无效参数错误 */
    RQ_ERR_OVERFLOW = -2,     /*!< 队列溢出错误 */
    RQ_ERR_OUTOFMEM = -3,     /*!< 内存不足错误 */
    RQ_ERR_NOITEM = -4,       /*!< 队列无元素错误 */
} RQueueErr_e;

/**
 * @brief 环形队列锁类型枚举
 */
typedef enum RBufferLockType_
{
    RQ_LOCK_TYPE_NONE = 0,     /*!< 无锁机制 */
    RQ_LOCK_TYPE_MUTEX = 1,    /*!< 使用互斥锁（Mutex） */
    RQ_LOCK_TYPE_CRITICAL = 2, /*!< 使用临界区（Critical Section） */
} RBufferLockType_e;

/**
 * @brief 环形队列结构体定义
 */
typedef struct RQueue_
{
    uint32_t front;              /*!< 队列头指针位置 */
    uint32_t rear;               /*!< 队列尾指针位置 */
    uint32_t count;              /*!< 当前队列中的元素数量 */
    uint32_t item_size;          /*!< 每个元素的大小（字节） */
    bool auto_increase;          /*!< 是否自动扩容 */
    uint8_t* item_array;         /*!< 元素数组指针 */
    uint32_t item_array_size;    /*!< 元素数组的大小（元素个数） */
    RBufferLockType_e lock_type; /*!< 锁类型 */
    osMutexId_t* lock;           /*!< 互斥锁指针 */
} RQueue_t;

/**
 * @brief 创建一个新的环形队列
 * @param[in] item_size 每个元素的大小（字节）
 * @param[in] item_count 队列初始容量（元素个数）
 * @param[in] auto_increase 是否支持自动扩容
 * @param[in] lock_type 锁类型选择
 * @return 成功返回队列指针，失败返回NULL
 */
RQueue_t* rqueue_create(int item_size, int item_count, bool auto_increase,
                        RBufferLockType_e lock_type);

/**
 * @brief 销毁环形队列并释放资源
 * @param[in] rq 队列指针
 * @return 成功返回RQ_SUCCESS，失败返回错误码
 */
int rqueue_destroy(RQueue_t* rq);

/**
 * @brief 获取队列容量
 * @param[in] rq 队列指针
 * @return 返回队列的最大容量
 */
int rqueue_get_capacity(RQueue_t* rq);

/**
 * @brief 增加队列容量
 * @param[in] rq 队列指针
 * @return 成功返回RQ_SUCCESS，失败返回错误码
 */
int rqueue_increase_capacity(RQueue_t* rq);

/**
 * @brief 获取当前队列中的元素数量
 * @param[in] rq 队列指针
 * @return 返回队列中的元素数量
 */
uint32_t rqueue_get_count(RQueue_t* rq);

/**
 * @brief 检查队列是否为空
 * @param[in] rq 队列指针
 * @return 队列为空返回true，否则返回false
 */
bool rqueue_is_empty(RQueue_t* rq);

/**
 * @brief 检查队列是否已满
 * @param[in] rq 队列指针
 * @return 队列已满返回true，否则返回false
 */
bool rqueue_is_full(RQueue_t* rq);

/**
 * @brief 清空队列中的所有元素
 * @param[in] rq 队列指针
 * @return 成功返回RQ_SUCCESS，失败返回错误码
 */
int rqueue_clear(RQueue_t* rq);

/**
 * @brief 向队列尾部添加一个元素
 * @param[in] rq 队列指针
 * @param[in] item 要添加的元素指针
 * @return 成功返回RQ_SUCCESS，失败返回错误码
 */
int rqueue_push_back(RQueue_t* rq, void* item);

/**
 * @brief 从队列头部移除一个元素
 * @param[in] rq 队列指针
 * @param[out] item 用于存储移除元素的缓冲区指针
 * @return 成功返回RQ_SUCCESS，失败返回错误码
 */
int rqueue_pop_front(RQueue_t* rq, void* item);

/**
 * @brief 获取队列头部元素但不移除
 * @param[in] rq 队列指针
 * @param[out] item 用于存储头部元素的缓冲区指针
 * @return 成功返回RQ_SUCCESS，失败返回错误码
 */
int rqueue_get_front(RQueue_t* rq, void* item);

#endif /* __RQUEUE_H__ */