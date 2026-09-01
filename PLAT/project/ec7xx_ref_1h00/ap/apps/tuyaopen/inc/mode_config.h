/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    mode.h
 * Description:  ec7xx mode config entry header file
 * History:      Rev1.0   2023-11-22
 *
 ****************************************************************************/
#ifndef  MODE_CONFIG_H
#define  MODE_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "ostask.h"


typedef enum Thread_Mode_bits {
    THREAD_FLAG_INIT =  (1UL << 0), 
    THREAD_FLAG_NORM =  (1UL << 1),  
    THREAD_FLAG_IDLE =  (1UL << 2), //非工作状态
    THREAD_FLAG_SLEP =  (1UL << 3), //osThreadSuspend
    THREAD_FLAG_STOP =  (1UL << 4), 
    THREAD_FLAG_TEST =  (1UL << 5), 
    THREAD_FLAG_MAX  =  (1UL << 6), 
    THREAD_FLAG_ALL  =  (THREAD_FLAG_MAX-1)
} ThreadModeBits;  

typedef enum
{
    PWR_NONE,
    PWR_IDLE,
    PWR_SLEEP,
}psStat_t;


#ifdef FEATURE_SUBSYS_APPHUB_ENABLE
#define SUBSYS_APPHUB_TASK_STACK_SIZE    (1024*5)
extern StaticTask_t             subsys_apphub_task;
extern uint8_t                  subsys_apphub_task_stack[SUBSYS_APPHUB_TASK_STACK_SIZE];
#endif
#ifdef FEATURE_SUBSYS_INPUT_ENABLE
#define SUBSYS_INPUT_TASK_STACK_SIZE     (1024*1)
extern StaticTask_t             subsys_input_task;
extern uint8_t                  subsysInputTaskStack[SUBSYS_INPUT_TASK_STACK_SIZE];
#endif
#ifdef FEATURE_SUBSYS_STATUS_ENABLE
#define SUBSYS_STATUS_TASK_STACK_SIZE    (1024*5)
extern StaticTask_t             subsysStatusTask;
extern uint8_t                  subsys_status_task_stack[SUBSYS_STATUS_TASK_STACK_SIZE];
#endif
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#define SUBSYS_STORAGE_TASK_STACK_SIZE   (1024*6)
extern StaticTask_t             subsys_storage_task;
extern uint8_t                  subsys_storage_task_stack[SUBSYS_STORAGE_TASK_STACK_SIZE];
#endif
#ifdef FEATURE_SUBSYS_MEDIA_ENABLE
// #define SUBSYS_MEDIA_TASK_STACK_SIZE     (1024*1)
// extern StaticTask_t             subsys_media_task;
// extern uint8_t                  subsys_media_task_stack[SUBSYS_MEDIA_TASK_STACK_SIZE];
#endif
#ifdef FEATURE_SUBSYS_LAUNCHER_ENABLE
#define SUBSYS_GUI_TASK_STACK_SIZE    (1024*4)
extern StaticTask_t             subsys_gui_task;
extern uint8_t                  subsys_gui_task_stack[SUBSYS_GUI_TASK_STACK_SIZE];
#ifdef FEATURE_DRIVER_LCD_ENABLE          
#include "lcdDrv.h"
#include "lcdComm.h"
extern lcdDrvFunc_t* lcdDev;
#endif
extern void guiInit(uint32_t mode);
extern void guiModeSet(ThreadModeBits mode);
#endif

#define SMS_BUFF_NUM                    (3U) 
#define SMS_SIZE_MAX                    (2*164U)
typedef struct
{
    int8_t index;  
    uint8_t length;    
    char user[25];   
    char date[25];  
    char number[25];     
    uint8_t text[SMS_SIZE_MAX];  
} sms_data_t;


#ifdef __cplusplus
}
#endif

#endif /* MODE_CONFIG_H */