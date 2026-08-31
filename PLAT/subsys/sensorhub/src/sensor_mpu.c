/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    MPU.c
 * Description:  EC718
 * History:      Rev1.0   2023-08-01
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_SENSORHUB_ENABLE
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "string.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os2.h"
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "time.h"
#include "storage.h"
// #include "status.h"
#include "bsp.h"
#include "ec_i2c.h"

#include "sensorhub.h"
#include "sensor_mpu.h"

#include "eMPL_outputs.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "invensense.h"
#include "invensense_adv.h"
#include "log.h"
#include "mltypes.h"
#include "mpl.h"
#include "mpu.h"
#include "mpl_ec718.h"
#include "api_i2c.h"

static bus_t mpu_bus;
uint8_t int_reg_value;

uint8_t mpu6500_cfg_reg[] = {
    MPU_PWR_MGMT1_REG,
    MPU_PWR_MGMT2_REG,
    MPU_ACCEL_CFG2_REG,
    MPU_MDETECT_CTRL_REG,
    MPU_LP_ACCEL_ODR,
    MOT_THR,
    MOT_DUR,
    MPU_INTBP_CFG_REG,
    MPU_INT_EN_REG,
    MPU_PWR_MGMT1_REG};
uint8_t mpu6500_cfg_val[] = {
    0x0, //
    0x07,
    0x01, // Accel LPF
    0xC0, // x80 + x40
    0x00, // 唤醒频率
    0x04,
    0x0A,
    0x80, // INTBP_CFG
    0x40, // INT_EN
    0x20  // BIT5 CYCLE
};

uint8_t sensor_mpu_sleep(void)
{
    uint8_t buff[2] = {0};
    buff[0] = 0x20; // LP:BIT5 CYCLE
    mpu_bus.i2c_send(MPU_ADDR, MPU_PWR_MGMT1_REG, 1, buff);
    buff[0] = 0x07;
    mpu_bus.i2c_send(MPU_ADDR, MPU_PWR_MGMT2_REG, 1, buff);
    // bit7  中断的逻辑电平模式,高电平时，设置为0；低电平时，设置为1
    // bit6  中断驱动模式,推拉模式设置为0，开漏模式设置为1.
    buff[0] = 0x00;
    mpu_bus.i2c_send(MPU_ADDR, MPU_INTBP_CFG_REG, 1, buff);
    // buff[0] = 0x06;      //低通滤波频率,典型值:0x06(5Hz)
    // mpu_bus.i2c_send(MPU_ADDR, MPU_CFG_REG, 1, buff);
    // bit7  自由落体中断使能
    // bit6  运动检测中断使能
    // bit5  零运动检测中断使能
    buff[0] = 0x40;
    mpu_bus.i2c_send(MPU_ADDR, MPU_INT_EN_REG, 1, buff);
    buff[0] = 0xC0; // bit7+bit6
    mpu_bus.i2c_send(MPU_ADDR, MPU_MDETECT_CTRL_REG, 1, buff);
    buff[0] = 0x01; // 0x1D Accel LPF
    mpu_bus.i2c_send(MPU_ADDR, MPU_ACCEL_CFG2_REG, 1, buff);
    buff[0] = 0x02;
    mpu_bus.i2c_send(MPU_ADDR, MPU_LP_ACCEL_ODR, 1, buff);
    buff[0] = 0x02; // 1LSB = 2mg
    mpu_bus.i2c_send(MPU_ADDR, MOT_THR, 1, buff);
    buff[0] = 0x0A; // 1LSB = 1ms
    mpu_bus.i2c_send(MPU_ADDR, MOT_DUR, 1, buff);
    // buff[0] = 0x04;     //1LSB = 2mg
    // mpu_bus.i2c_send(MPU_ADDR, ZRMOT_THR, 1, buff);
    // buff[0] = 100;       //1LSB = 32ms
    // mpu_bus.i2c_send(MPU_ADDR, ZRMOT_DUR, 1, buff);
    // for (int i = 0; i < sizeof(mpu6500_cfg_reg);i++){
    //     mpu_bus.i2c_send(MPU_ADDR, mpu6500_cfg_reg[i], 1, mpu6500_cfg_val[i]);
    // }
}

uint8_t sensor_dmp_init(void)
{
    struct int_param_s int_param;
    inv_error_t result = inv_mpu_init(&int_param);
    if (result)
    {
        // ECOMM_TRACE(UNILOG_SENSOR, MPU_DMP_INIT, P_ERROR, 1, "result 0x%X",result);
        // printf("Could not initialize MPU.\r\n");
        // MPL_LOGE("Could not initialize MPU.\r\n");
        return 2;
    }
    result = inv_init_mpl();
    if (result)
    {
        // ECOMM_TRACE(UNILOG_SENSOR, MPU_MPL_INIT, P_ERROR, 1, "result 0x%X",result);
        // printf("Could not initialize MPL.\r\n");
        // MPL_LOGE("Could not initialize MPL.\r\n");
        return 3;
    }
    sensor_mpu_config();
    return 0;
}

static void test_i2c_cb(uint32_t event)
{
    // printf("event = %d\r\n", event);
}

uint8_t sensor_mpu_init(void)
{
    uint32_t i2c_id = 0;
    uint32_t index = 0;
    i2c_id = api_i2c_create(index, NULL);
    ec_hal_i2c_set_id(i2c_id);
    api_i2c_open(i2c_id, NULL, 1000);
    api_i2c_ioctl(i2c_id, OPEN_I2C_IOCTL_ISR_CB, test_i2c_cb);
    uint8_t speed = 2;
    api_i2c_ioctl(i2c_id, OPEN_I2C_IOCTL_SPEED, &speed);
    uint8_t temp[1] = {0};

    api_i2c_master_t buffer = {
        .addr = MPU_ADDR,
        .reg = MPU_INT_STA_REG,
        .data = temp,
        .num = 1,
    };

    api_i2c_read(i2c_id, &buffer, sizeof(buffer));
    // printf("status = 0x%02x\r\n", buffer.data[0]);

    api_i2c_read(i2c_id, &buffer, sizeof(buffer));
    // printf("status = 0x%02x\r\n", buffer.data[0]);

    api_i2c_read(i2c_id, &buffer, sizeof(buffer));
    // printf("status = 0x%02x\r\n", buffer.data[0]);

    if (buffer.data[0] & 0x40) // 中断唤醒
    {
        int_reg_value = buffer.data[0];
        xEventGroupSetBits(g_sensorEvents, IMU_WAKEUP);
    }

    buffer.reg = MPU_DEVICE_ID_REG;

    api_i2c_read(i2c_id, &buffer, sizeof(buffer));
    // printf("id = 0x%02x\r\n", buffer.data[0]);

    api_i2c_read(i2c_id, &buffer, sizeof(buffer));
    // printf("id = 0x%02x\r\n", buffer.data[0]);

    api_i2c_read(i2c_id, &buffer, sizeof(buffer));
    // printf("id = 0x%02x\r\n", buffer.data[0]);

    if (buffer.data[0] == MPU6500_WHO_AM_I || buffer.data[0] == MPU9250_WHO_AM_I || buffer.data[0] == MPU6050_WHO_AM_I)
    {
        uint8_t check_ret = sensor_dmp_init();
        // printf("sensor_dmp_init = %d\r\n", check_ret);
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t sensor_mpu_wake(void)
{
    // mpu_bus.i2c_read(MPU_ADDR, MPU_INT_STA_REG, 1, data);
    // mpu_bus.i2c_read(MPU_ADDR, MOT_DETECT_STATUS, 1, data+1);
    // if(data[0] & 0x40) return 1;
    // ECOMM_TRACE(UNILOG_SENSOR, MPU_WAKEUP, P_INFO, 1, "0x3A-0x%X",int_reg_value);
    return 0;
}

uint8_t sensor_mpu_loop(imu_data_t *data)
{
    long temperature = 0;
    if (mpl_data_loop(&temperature))
    {
        data->temp = temperature / 1000;
        inv_execute_on_data();
        read_from_mpl(data);
        data->yaw = -(data->euler[2] / 1000);
        data->roll = (data->euler[0] / 1000);
        data->pitch = -(data->euler[1] / 1000);
        // osDelay(500);
        return 1;
    }
    return 0;
}

#endif
