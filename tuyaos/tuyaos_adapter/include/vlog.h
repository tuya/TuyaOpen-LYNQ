#ifndef __VLOG_H__
#define __VLOG_H__

#include "ol_log.h"

// 定义宏拼接辅助宏，用于生成唯一标识符
#define TUYA__CONCAT(a, b) a##_##b
#define TUYA_CONCAT(a, b) TUYA__CONCAT(a, b)
#define TUYA_CONCAT3(a, b, c) TUYA_CONCAT(TUYA_CONCAT(a, b), c)

// 定义LOGI宏，自动使用 __CURRENT_FILE_NAME__、行号和日志级别生成唯一ID
// 唯一ID格式: 文件名_行号_日志级别(I/D/E)
#define LOGI(fmt, ...) ECCUST_PRINTF(UNILOG_APP, TUYA_CONCAT3(__CURRENT_FILE_NAME__, __LINE__, TYI), P_INFO, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) ECCUST_PRINTF(UNILOG_APP, TUYA_CONCAT3(__CURRENT_FILE_NAME__, __LINE__, TYD), P_INFO, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) ECCUST_PRINTF(UNILOG_APP, TUYA_CONCAT3(__CURRENT_FILE_NAME__, __LINE__, TYE), P_INFO, fmt, ##__VA_ARGS__)

#endif