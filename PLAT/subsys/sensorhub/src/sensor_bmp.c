/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of EigenComm Ltd.
 * File name:    bh1750.c
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

#include "sensorhub.h"

#include "sw_i2c.h"
#include "hw_i2c.h"

#include "bmp2.h"
#include "sensor_bmp.h"

static uint8_t dev_addr;
bus_t bmp_bus;

/*!
 *  @brief Prints the execution status of the APIs.
 */
void bmp2_error_codes_print_result(const char api_name[], int8_t rslt)
{
    if (rslt != BMP2_OK)
    {
        printf("%s\t", api_name);

        switch (rslt)
        {
            case BMP2_E_NULL_PTR:
                printf("Error [%d] : Null pointer error.", rslt);
                printf(
                    "It occurs when the user tries to assign value (not address) to a pointer, which has been initialized to NULL.\r\n");
                break;
            case BMP2_E_COM_FAIL:
                printf("Error [%d] : Communication failure error.", rslt);
                printf(
                    "It occurs due to read/write operation failure and also due to power failure during communication\r\n");
                break;
            case BMP2_E_INVALID_LEN:
                printf("Error [%d] : Invalid length error.", rslt);
                printf("Occurs when length of data to be written is zero\n");
                break;
            case BMP2_E_DEV_NOT_FOUND:
                printf("Error [%d] : Device not found error. It occurs when the device chip id is incorrectly read\r\n",
                       rslt);
                break;
            case BMP2_E_UNCOMP_TEMP_RANGE:
                printf("Error [%d] : Uncompensated temperature data not in valid range error.", rslt);
                break;
            case BMP2_E_UNCOMP_PRESS_RANGE:
                printf("Error [%d] : Uncompensated pressure data not in valid range error.", rslt);
                break;
            case BMP2_E_UNCOMP_TEMP_AND_PRESS_RANGE:
                printf(
                    "Error [%d] : Uncompensated pressure data and uncompensated temperature data are not in valid range error.",
                    rslt);
                break;
            default:
                printf("Error [%d] : Unknown error code\r\n", rslt);
                break;
        }
    }
}
static int8_t get_data(uint32_t period, struct bmp2_config *conf, struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_E_NULL_PTR;
    int8_t idx = 1;
    struct bmp2_status status;
    struct bmp2_data comp_data;

    printf("Measurement delay : %lu us\r\n", (long unsigned int)period);

    while (idx <= 50)
    {
        rslt = bmp2_get_status(&status, dev);
        bmp2_error_codes_print_result("bmp2_get_status", rslt);

        if (status.measuring == BMP2_MEAS_DONE)
        {
            /* Delay between measurements */
            dev->delay_us(period, dev->intf_ptr);

            /* Read compensated data */
            rslt = bmp2_get_sensor_data(&comp_data, dev);
            bmp2_error_codes_print_result("bmp2_get_sensor_data", rslt);
            #ifdef BMP2_DOUBLE_COMPENSATION
            printf("Data[%d]:    Temperature: %.4lf deg C	Pressure: %.4lf Pa\r\n",
                   idx,
                   comp_data.temperature,
                   comp_data.pressure);
            #else
            printf("Data[%d]:    Temperature: %ld deg C	Pressure: %lu Pa\r\n", idx, (long int)comp_data.temperature,
                   (long unsigned int)comp_data.pressure);
            #endif
            idx++;
        }
    }

    return rslt;
}
/*!
 * I2C read function map to COINES platform
 */
BMP2_INTF_RET_TYPE bmp2_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    dev_addr = *(uint8_t*)intf_ptr;
    return bmp_bus.i2c_read(dev_addr, reg_addr, reg_data, (uint8_t)length);
}
/*!
 * I2C write function map to COINES platform
 */
BMP2_INTF_RET_TYPE bmp2_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    dev_addr = *(uint8_t*)intf_ptr;
    return bmp_bus.i2c_send(dev_addr, reg_addr, (uint8_t *)reg_data, (uint8_t)length);
}
static void bmp2_delay_us(uint32_t period, void *intf_ptr)
{
    delay_us(period);
}
int8_t bmp2_interface_selection(struct bmp2_dev *dev, uint8_t intf)
{
    int8_t rslt = BMP2_OK;
    if (dev != NULL)
    {
        if (intf == BMP2_I2C_INTF)
        {
            printf("I2C Interface\r\n");
            dev_addr = BMP2_I2C_ADDR_PRIM;
            dev->read = bmp2_i2c_read;
            dev->write = bmp2_i2c_write;
            dev->intf = BMP2_I2C_INTF;
        }
        /* Holds the I2C device addr or SPI chip selection */
        dev->intf_ptr = &dev_addr;
        /* Configure delay in microseconds */
        dev->delay_us = bmp2_delay_us;
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

void sensor_bmp_test(void)
{
    int8_t rslt;
    uint32_t meas_time;
    struct bmp2_dev dev;
    struct bmp2_config conf;

    rslt = bmp2_interface_selection(&dev, BMP2_I2C_INTF);
    bmp2_error_codes_print_result("bmp2_interface_selection", rslt);

    rslt = bmp2_init(&dev);
    bmp2_error_codes_print_result("bmp2_init", rslt);

    /* Always read the current settings before writing, especially when all the configuration is not modified */
    rslt = bmp2_get_config(&conf, &dev);
    bmp2_error_codes_print_result("bmp2_get_config", rslt);

    /* Configuring the over-sampling mode, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = BMP2_FILTER_OFF;

    /* Over-sampling mode is set as high resolution i.e., os_pres = 8x and os_temp = 1x */
    conf.os_mode = BMP2_OS_MODE_HIGH_RESOLUTION;

    /* Setting the output data rate */
    conf.odr = BMP2_ODR_250_MS;

    rslt = bmp2_set_config(&conf, &dev);
    bmp2_error_codes_print_result("bmp2_set_config", rslt);

    /* Set normal power mode */
    rslt = bmp2_set_power_mode(BMP2_POWERMODE_NORMAL, &conf, &dev);
    bmp2_error_codes_print_result("bmp2_set_power_mode", rslt);

    /* Calculate measurement time in microseconds */
    rslt = bmp2_compute_meas_time(&meas_time, &conf, &dev);
    bmp2_error_codes_print_result("bmp2_compute_meas_time", rslt);

    /* Read pressure and temperature data */
    rslt = get_data(meas_time, &conf, &dev);
    bmp2_error_codes_print_result("get_data", rslt);
}

void sensor_bmp_init(void)
{
    uint8_t chip_id[2];
    // uint8_t data[14];
    bmp_bus = hw_i2c_default(0);
    // bmp_bus = hw_i2c_default();
    // bmp_bus.i2c_init();
    bmp_bus.i2c_read(BMP2_I2C_ADDR_PRIM, BMP2_REG_CHIP_ID, 1, chip_id);
    // osDelay(100);
    // bmp_bus.i2c_read(BMP280_7b_ADDR, 0xD0, chip_id+1, 1);
    printf("\n\rBMP Sensor: 0x%02x\n\r", chip_id[0]);
    
    
    // printf("\r\nMPU-axis:");
    // for(uint8_t i = 0; i<14; i++)
    // {
    //     printf("0x%x ", data[i]);
    // }
    // printf("\r\n");
}


#endif