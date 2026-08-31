/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    sensorhub.c
 * Description:  EC718
 * History:      Rev1.0   2023-04-20
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SENSORHUB_ENABLE
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "string.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "cmsis_os2.h"

#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "networkmgr.h"
#include "slpman.h"
#include "bsp.h"
#include "servicemanager.h"
#include "sensorhub.h"
#include "sw_i2c.h"
#include "hw_i2c.h"

#include "sensor_light.h"
#include "sensor_bme.h"
#include "sensor_mpu.h"
#include "sensor_bmp.h"

static uint16_t sensor_flag = 0;

imu_data_t mpu = {0};
EventGroupHandle_t g_sensorEvents = NULL;

void enter_sleep(void)
{
    sensor_mpu_sleep();
    appSetCFUN(0);
    slpManSetPmuSleepMode(true, SLP_HIB_STATE, false);
}

void IMU_ISR(void)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    uint16_t portIrqMask = GPIO_saveAndSetIrqMask(IRQ_GPIO_INSTANCE);
    if (GPIO_getInterruptFlags(IRQ_GPIO_INSTANCE) & (1 << IRQ_GPIO_PIN))
    {
        gyro_data_ready_cb();
        xEventGroupSetBitsFromISR(g_sensorEvents, IMU_MOTION, &pxHigherPriorityTaskWoken);
        GPIO_clearInterruptFlags(IRQ_GPIO_INSTANCE, 1 << IRQ_GPIO_PIN);
    }
    GPIO_restoreIrqMask(IRQ_GPIO_INSTANCE, portIrqMask);
}

void sensor_irq_init(BOOL high)
{
    PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = IRQ_PAD_ALT_FUNC;
    PAD_setPinConfig(IRQ_PAD_INDEX, &padConfig);
    PAD_setPinPullConfig(IRQ_PAD_INDEX, PAD_INTERNAL_PULL_UP);

    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_INPUT;
    config.misc.interruptConfig = GPIO_INTERRUPT_FALLING_EDGE;
    GPIO_pinConfig(IRQ_GPIO_INSTANCE, IRQ_GPIO_PIN, &config);
    XIC_SetVector(PXIC1_GPIO_IRQn, IMU_ISR);
    XIC_EnableIRQ(PXIC1_GPIO_IRQn);
}

void get_imu_data(imu_data_t *tmp)
{
    memcpy(tmp, &mpu, sizeof(imu_data_t));
}

void sensorhub_poll(void)
{
    // if (sensor_flag)
    // {
    //     EventBits_t BITs = xEventGroupWaitBits(
    //         g_sensorEvents,
    //         IMU_WAKEUP | IMU_MOTION | IMU_SLEEP,
    //         pdFALSE,
    //         pdFALSE,
    //         3000);
    //     printf("BITs = %x\r\n", BITs);
    //     if (BITs & IMU_WAKEUP) // 中断唤醒后处理
    //     {
    //         xEventGroupClearBits(g_sensorEvents, IMU_WAKEUP);
    //         sensor_mpu_wake();
    //     }
    //     else if (BITs & IMU_MOTION)
    //     {
    //         xEventGroupClearBits(g_sensorEvents, IMU_MOTION);
    //         if (sensor_mpu_loop(&mpu))
    //         {
    //             printf("%d,%d,%d,%d,%d.%d",
    //                    mpu.accuracy, mpu.yaw, mpu.roll, mpu.pitch,
    //                    mpu.temp / 100, mpu.temp % 100);
    //         }
    //     }
    //     else if (BITs & IMU_SLEEP) // 进入休眠前配置
    //     {
    //         enter_sleep();
    //     }
    // }

    // sensor_mpu_wake();
    gyro_data_ready_cb();

    uint8_t loop = sensor_mpu_loop(&mpu);
    // if (loop)
    // {
    // printf("%d,%d,%d,%d,%d.%d\r\n",
    //        mpu.accuracy, mpu.yaw, mpu.roll, mpu.pitch,
    //        mpu.temp / 100, mpu.temp % 100);
    // }
}

static void sensorTask(void *arg)
{
    while (1)
    {
        sensorhub_poll();
        osDelay(1000);
    }
}

void sensorhub_init(void)
{
    sensor_flag = 0x0;
    g_sensorEvents = xEventGroupCreate();
    sensor_irq_init(TRUE);
    if (sensor_mpu_init() == 0)
    {
        sensor_flag |= 0x01;
        xEventGroupSetBits(g_sensorEvents, IMU_READY);
    }
    printf("sensor_flag = 0x%02x\r\n", sensor_flag);
    if (sensor_flag)
    {
        // ECOMM_TRACE(UNILOG_SENSOR, SHUB_INIT, P_VALUE, 1, "flag 0x%X",sensor_flag);
        osThreadAttr_t sensor_attr;
        memset(&sensor_attr, 0, sizeof(sensor_attr));
        sensor_attr.name = "sensorTask";
        sensor_attr.stack_size = SHUB_TASK_STATK_SIZE;
        sensor_attr.priority = osPriorityNormal;
#if 0
        osThreadNew(sensorTask, NULL, &sensor_attr);
#else
        char serviceName[32] = {0};
        snprintf(serviceName, sizeof(serviceName), "service:/%s", sensor_attr.name);
        Service_reg(serviceName, sensorTask, NULL, sensor_attr.cb_mem, sensor_attr.cb_size, sensor_attr.stack_mem, sensor_attr.stack_size, sensor_attr.priority);
        Service_start(serviceName);
#endif
    }
}

#endif // FEATURE_SUBSYS_SENSORHUB_ENABLE