/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "osasys.h"
#include "ostask.h"
#include "os_common.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "ol_log.h"


/*
Note:
    Supports floating point:

    1 - the stack of tasks must be aligned with 8 bytes,
        and 4 bytes must be reserved. For example (2048+4 / 4096+4)
    2 - stack space can only be allocated statically
*/
#define TEST_TASK_STACK_SIZE    (4096+4)

__ALIGNED(8) static uint8_t test_taskstack1[TEST_TASK_STACK_SIZE];
static uint8_t test_taskstack2[TEST_TASK_STACK_SIZE];

static StaticTask_t test_task1_t;
static StaticTask_t test_task2_t;

int test_value = 0;
static osMutexId_t test_mutex = NULL;
static osEventFlagsId_t test_EvtHandle = NULL;

static osTimerId_t test_timerid = NULL;


void test_float(void)
{
    float val1 = 0.0;
    float val2 = 0.0;
    float val3 = 0.0;
    char temp[128] = {0};
    char buffer[] = {"0.996 0.007"};


    OL_LOG_INFO("------ test_float ------");
    memset(temp, 0, sizeof(temp));
    sprintf(temp, "float sprintf: %lf", 3.141592654);
    OL_LOG_INFO("%s", temp);

    memset(temp, 0, sizeof(temp));
    sscanf(buffer, "%f %f", &val1, &val2);
    val3 = val1 + val2;
    snprintf(temp, sizeof(temp), "float sscanf: %f + %f = %f", val1, val2, val3);
    OL_LOG_INFO("%s", temp);
}

void test_task_1(void* arg)
{
    int ret = 0;
    int index = 0;

    test_float();
    while(index++ < 5)
    {
        osMutexAcquire(test_mutex, osWaitForever);
        OL_LOG_INFO("---- test_value = %d ----", test_value++);
        osMutexRelease(test_mutex);
        osDelay(200);
    }

    OL_LOG_INFO("--- FlagsWait ---");
    ret = osEventFlagsWait(test_EvtHandle, 0x00000001, osFlagsWaitAny, osWaitForever);
    OL_LOG_INFO("--- FlagsWait end---, ret=%d", ret);

    osMutexAcquire(test_mutex, osWaitForever);
    test_value = 0;
    osMutexRelease(test_mutex);

    osMutexDelete(test_mutex);
    osEventFlagsDelete(test_EvtHandle);
    test_mutex = NULL;
    test_EvtHandle = NULL;
    osThreadExit();
}

void test_task_2(void* arg)
{
    int ret = 0;
    int index = 0;

    test_float();

    while(index++ < 5)
    {
        osMutexAcquire(test_mutex, osWaitForever);
        OL_LOG_INFO("---- test_value = %d ----", test_value++);
        osMutexRelease(test_mutex);
        osDelay(300);
    }

    osDelay(5000);
    OL_LOG_INFO("task2 set event flag to task1");
    ret = osEventFlagsSet(test_EvtHandle, 0x00000001);
    OL_LOG_INFO("task2 osEventFlagsSet ret = %d", ret);

    while(1)
    {
        OL_LOG_INFO("---- test_task_2 = %d ----", test_value++);
        osDelay(3000);
    }

    osThreadExit();
}

void test_tasks(void)
{
    osThreadAttr_t task1_attr = {0};
    osThreadAttr_t task2_attr = {0};

    if(NULL == test_mutex)
    {
        test_mutex = osMutexNew(NULL);
    }

    if(NULL == test_EvtHandle)
    {
        test_EvtHandle = osEventFlagsNew(NULL);
    }

    osMutexAcquire(test_mutex, osWaitForever);
    if(test_value > 0)
    {
        OL_LOG_INFO("check test_value is %d, return", test_value);
        osMutexRelease(test_mutex);
        return;
    }
    osMutexRelease(test_mutex);

    memset(&task1_attr, 0, sizeof(task1_attr));
    memset(test_taskstack1, 0xA5, TEST_TASK_STACK_SIZE);
    task1_attr.name = "testtask1";
    task1_attr.stack_mem = test_taskstack1;
    task1_attr.stack_size = TEST_TASK_STACK_SIZE;
    task1_attr.priority = osPriorityNormal;
    task1_attr.cb_mem = &test_task1_t;
    task1_attr.cb_size = sizeof(StaticTask_t);
    osThreadNew(test_task_1, NULL, &task1_attr);

    osDelay(100);

    memset(&task2_attr, 0, sizeof(task1_attr));
    memset(test_taskstack2, 0xA5, TEST_TASK_STACK_SIZE);
    task2_attr.name = "testtask2";
    task2_attr.stack_mem = test_taskstack2;
    task2_attr.stack_size = TEST_TASK_STACK_SIZE;
    task2_attr.priority = osPriorityNormal;
    task2_attr.cb_mem = &test_task2_t;
    task2_attr.cb_size = sizeof(StaticTask_t);
    osThreadNew(test_task_2, NULL, &task2_attr);
}

void test_timer_cb(void)
{
    OL_LOG_INFO("test_timer_cb");
}

void test_timer(void)
{
    if(test_timerid == NULL)
    {
        test_timerid = osTimerNew((osTimerFunc_t)test_timer_cb, osTimerOnce, NULL, NULL);
        osTimerStart(test_timerid, 5 * 1000);
    }
    else
    {
        OL_LOG_INFO("test_timerid state: %d", osTimerIsRunning(test_timerid));
        if(osTimerIsRunning(test_timerid) != 0)
        {
            osTimerStop(test_timerid);
        }
        osTimerDelete(test_timerid);
        test_timerid = NULL;
    }
}

void os_demo(void)
{
    OL_LOG_INFO("----ANSI÷–Œƒlog≤‚ ‘----");

    OL_LOG_INFO("get TotalHeapSize = %d",xPortGetTotalHeapSize());
    OL_LOG_INFO("get FreeHeapSize = %d",xPortGetFreeHeapSize());
    OL_LOG_INFO("get MiniFreeHeapSize = %d",xPortGetMinimumEverFreeHeapSize());

    OL_LOG_INFO("get Cust TotalHeapSize = %d",xPortGetTotalHeapSizeCust());
    OL_LOG_INFO("get Cust FreeHeapSize = %d",xPortGetFreeHeapSizeCust());
    OL_LOG_INFO("get Cust MiniFreeHeapSize = %d",xPortGetMinimumEverFreeHeapSizeCust());

    test_timer();
    test_tasks();
}

