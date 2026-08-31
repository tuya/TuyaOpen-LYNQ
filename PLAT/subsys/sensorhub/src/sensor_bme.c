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
#include "bme68x.h"
#include "bme68x_defs.h"

#include "sensor_bme.h"

struct bme68x_dev bme;

static uint8_t dev_addr;

bus_t bme_bus;

void bme_delay_us(uint32_t period, void *intf_ptr)
{
    delay_us(period);
}
/*!
 * I2C read function map to COINES platform
 */
BME68X_INTF_RET_TYPE bme_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    dev_addr = *(uint8_t*)intf_ptr;
    return bme_bus.i2c_read(dev_addr, reg_addr, reg_data, (uint8_t)length);
}
/*!
 * I2C write function map to COINES platform
 */
BME68X_INTF_RET_TYPE bme_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    dev_addr = *(uint8_t*)intf_ptr;
    return bme_bus.i2c_send(dev_addr, reg_addr, (uint8_t *)reg_data, (uint8_t)length);
}
void bme68x_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BME68X_OK:
            /* Do nothing */
            break;
        case BME68X_E_NULL_PTR:
            printf("API name [%s]  Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BME68X_E_COM_FAIL:
            printf("API name [%s]  Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BME68X_E_INVALID_LENGTH:
            printf("API name [%s]  Error [%d] : Incorrect length parameter\r\n", api_name, rslt);
            break;
        case BME68X_E_DEV_NOT_FOUND:
            printf("API name [%s]  Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        case BME68X_E_SELF_TEST:
            printf("API name [%s]  Error [%d] : Self test error\r\n", api_name, rslt);
            break;
        case BME68X_W_NO_NEW_DATA:
            printf("API name [%s]  Warning [%d] : No new data found\r\n", api_name, rslt);
            break;
        default:
            printf("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

int8_t bme_bus_init(struct bme68x_dev *bme, uint8_t intf)
{
    int8_t rslt = BME68X_OK;
    if (bme != NULL)
    {
        /* Bus configuration : I2C */
        if (intf == BME68X_I2C_INTF)
        {
            // printf("I2C Interface\n");
            dev_addr = BME680_7b_ADDR;
            bme->read = bme_i2c_read;
            bme->write = bme_i2c_write;
            bme->intf = BME68X_I2C_INTF;
        }
        /* Bus configuration : SPI */
        else if (intf == BME68X_SPI_INTF)
        {
            // printf("SPI Interface\n");
            // dev_addr = COINES_SHUTTLE_PIN_7;
            // bme->read = bme68x_spi_read;
            // bme->write = bme68x_spi_write;
            bme->intf = BME68X_SPI_INTF;
        }
        bme->delay_us = bme_delay_us;
        bme->intf_ptr = &dev_addr;
        bme->amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */
    }
    else
    {
        rslt = BME68X_E_NULL_PTR;
    }
    return rslt;
}


//BME68X_SEQUENTIAL_MODE
#define SAMPLE_COUNT  UINT8_C(3)
#define BME68X_VALID_DATA  UINT8_C(0xB0)

void bme_test_parallel_mode(void)
{
    int8_t rslt;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data[3];
    uint32_t del_period;
    uint8_t n_fields;
    uint32_t time_ms = 0;
    uint16_t sample_count = 1;

    /* Heater temperature in degree Celsius */
    uint16_t temp_prof[10] = { 320, 100, 100, 100, 200, 200, 200, 320, 320, 320 };

    /* Multiplier to the shared heater duration */
    uint16_t mul_prof[10] = { 5, 2, 10, 30, 5, 5, 5, 5, 5, 5 };

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    rslt = bme68x_get_conf(&conf, &bme);
    bme68x_check_rslt("bme68x_get_conf", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_1X;
    conf.os_pres = BME68X_OS_16X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf, &bme);
    bme68x_check_rslt("bme68x_set_conf", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp_prof = temp_prof;
    heatr_conf.heatr_dur_prof = mul_prof;

    /* Shared heating duration in milliseconds */
    heatr_conf.shared_heatr_dur = 140 - (bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &conf, &bme) / 1000);

    heatr_conf.profile_len = 10;
    rslt = bme68x_set_heatr_conf(BME68X_PARALLEL_MODE, &heatr_conf, &bme);
    bme68x_check_rslt("bme68x_set_heatr_conf", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    rslt = bme68x_set_op_mode(BME68X_PARALLEL_MODE, &bme);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);

    // printf(
    //     "\r\nPrint parallel mode data if mask for new data(0x80), gas measurement(0x20) and heater stability(0x10) are set\n\n");

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    printf(
        "\r\nSample, TimeStamp(ms), Temperature(deg C), Pressure(Pa), Humidity(%%), Gas resistance(ohm), Status, Gas index, Meas index");
    while (sample_count <= SAMPLE_COUNT)
    {
        /* Calculate delay period in microseconds */
        del_period = bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &conf, &bme) + (heatr_conf.shared_heatr_dur * 1000);
        bme.delay_us(del_period, bme.intf_ptr);

        time_ms = xTaskGetTickCount();

        rslt = bme68x_get_data(BME68X_PARALLEL_MODE, data, &n_fields, &bme);
        bme68x_check_rslt("bme68x_get_data", rslt);

        /* Check if rslt == BME68X_OK, report or handle if otherwise */
        for (uint8_t i = 0; i < n_fields; i++)
        {
            if (data[i].status == BME68X_VALID_DATA)
            {
                printf("\r\n%u,\t%lu,\t%d,\t%lu,\t%lu,\t%lu,\t0x%x,\t%d,\t%d",
                       sample_count,
                       (long unsigned int)time_ms,
                       (data[i].temperature / 100),
                       (long unsigned int)data[i].pressure,
                       (long unsigned int)(data[i].humidity / 1000),
                       (long unsigned int)data[i].gas_resistance,
                       data[i].status,
                       data[i].gas_index,
                       data[i].meas_index);
                sample_count++;
            }
        }
    }
    return 0;
}

void sensor_bme_test(void)
{
    int8_t rslt;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data[3];
    uint32_t del_period;
    uint32_t time_ms = 0;
    uint8_t n_fields;
    uint16_t sample_count = 1;

    /* Heater temperature in degree Celsius */
    uint16_t temp_prof[10] = { 200, 240, 280, 320, 360, 360, 320, 280, 240, 200 };

    /* Heating duration in milliseconds */
    uint16_t dur_prof[10] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    rslt = bme68x_get_conf(&conf, &bme);
    bme68x_check_rslt("bme68x_get_conf", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE; /* This parameter defines the sleep duration after each profile */
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf, &bme);
    bme68x_check_rslt("bme68x_set_conf", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp_prof = temp_prof;
    heatr_conf.heatr_dur_prof = dur_prof;
    heatr_conf.profile_len = 10;
    rslt = bme68x_set_heatr_conf(BME68X_SEQUENTIAL_MODE, &heatr_conf, &bme);
    bme68x_check_rslt("bme68x_set_heatr_conf", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    rslt = bme68x_set_op_mode(BME68X_SEQUENTIAL_MODE, &bme);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    printf("\r\nSample, TimeStamp, Temperature, Pressure, Humidity, Gas(ohm), Status, Profile, index");
    while (sample_count <= SAMPLE_COUNT)
    {
        /* Calculate delay period in microseconds */
        del_period = bme68x_get_meas_dur(BME68X_SEQUENTIAL_MODE, &conf, &bme) + (heatr_conf.heatr_dur_prof[0] * 1000);
        bme.delay_us(del_period, bme.intf_ptr);
        time_ms = xTaskGetTickCount();
        rslt = bme68x_get_data(BME68X_SEQUENTIAL_MODE, data, &n_fields, &bme);
        bme68x_check_rslt("bme68x_get_data", rslt);

        /* Check if rslt == BME68X_OK, report or handle if otherwise */
        for (uint8_t i = 0; i < n_fields; i++)
        {
            printf("\r\n%u,\t%lu ms,\t%d C,\t%lu,\t%lu %%,\t%lu,\t0x%x,\t%d,\t%d",
                   sample_count,
                   (long unsigned int)time_ms + (i * (del_period / 2000)),
                   (data[i].temperature / 100),
                   (long unsigned int)data[i].pressure,
                   (long unsigned int)(data[i].humidity / 1000),
                   (long unsigned int)data[i].gas_resistance,
                   data[i].status,
                   data[i].gas_index,
                   data[i].meas_index);
            sample_count++;
        }
    }
    return 0;
}

/*
 * @brief This API performs Self-test of low and high gas variants of BME68X
 */
int8_t bme_selftest_check(const struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t n_fields;
    uint8_t i = 0;
    struct bme68x_data data[BME68X_N_MEAS] = { { 0 } };
    struct bme68x_dev t_dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;

    /* Copy required parameters from reference bme68x_dev struct */
    t_dev.amb_temp = 25;
    t_dev.read = dev->read;
    t_dev.write = dev->write;
    t_dev.intf = dev->intf;
    t_dev.delay_us = dev->delay_us;
    t_dev.intf_ptr = dev->intf_ptr;
    rslt = bme68x_init(&t_dev);
    if (rslt == BME68X_OK)
    {
        /* Set the temperature, pressure and humidity & filter settings */
        conf.os_hum = BME68X_OS_1X;
        conf.os_pres = BME68X_OS_16X;
        conf.os_temp = BME68X_OS_2X;

        /* Set the remaining gas sensor settings and link the heating profile */
        heatr_conf.enable = BME68X_ENABLE;
        heatr_conf.heatr_dur = BME68X_HEATR_DUR1;
        heatr_conf.heatr_temp = BME68X_HIGH_TEMP;
        rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &t_dev);
        if (rslt == BME68X_OK)
        {
            rslt = bme68x_set_conf(&conf, &t_dev);
            if (rslt == BME68X_OK)
            {
                rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &t_dev); /* Trigger a measurement */
                if (rslt == BME68X_OK)
                {
                    /* Wait for the measurement to complete */
                    t_dev.delay_us(BME68X_HEATR_DUR1_DELAY, t_dev.intf_ptr);
                    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data[0], &n_fields, &t_dev);
                    if (rslt == BME68X_OK)
                    {
                        if ((data[0].idac != 0x00) && (data[0].idac != 0xFF) &&
                            (data[0].status & BME68X_GASM_VALID_MSK))
                        {
                            rslt = BME68X_OK;
                        }
                        else
                        {
                            rslt = BME68X_E_SELF_TEST;
                        }
                    }
                }
            }
        }

        heatr_conf.heatr_dur = BME68X_HEATR_DUR2;
        while ((rslt == BME68X_OK) && (i < BME68X_N_MEAS))
        {
            if (i % 2 == 0)
            {
                heatr_conf.heatr_temp = BME68X_HIGH_TEMP; /* Higher temperature */
            }
            else
            {
                heatr_conf.heatr_temp = BME68X_LOW_TEMP; /* Lower temperature */
            }

            rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &t_dev);
            if (rslt == BME68X_OK)
            {
                rslt = bme68x_set_conf(&conf, &t_dev);
                if (rslt == BME68X_OK)
                {
                    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &t_dev); /* Trigger a measurement */
                    if (rslt == BME68X_OK)
                    {
                        /* Wait for the measurement to complete */
                        t_dev.delay_us(BME68X_HEATR_DUR2_DELAY, t_dev.intf_ptr);
                        rslt = bme68x_get_data(BME68X_FORCED_MODE, &data[i], &n_fields, &t_dev);
                    }
                }
            }

            i++;
        }

        if (rslt == BME68X_OK)
        {
            rslt = analyze_sensor_data(data, BME68X_N_MEAS);
        }
    }

    return rslt;
}

void sensor_bme_init(void)
{
    uint8_t chip_id[2];
    // uint8_t data[14];
    // bme_bus = sw_i2c_default();
    bme_bus = hw_i2c_default(0);
    // bme_bus.i2c_init();
    bme_bus.i2c_read(BME680_7b_ADDR, BME68X_REG_CHIP_ID, 1, chip_id);
    // osDelay(100);
    // bme_bus.i2c_read(BMP280_7b_ADDR, 0xD0, chip_id+1, 1);
    // printf("\n\rBME 0x%02x: 0x%02x\n\r", BME680_7b_ADDR,chip_id[0]);

    /* Interface preference is updated as a parameter
     * For I2C : BME68X_I2C_INTF
     * For SPI : BME68X_SPI_INTF
     */
    int8_t rslt = bme_bus_init(&bme, BME68X_I2C_INTF);
    bme68x_check_rslt("bme_bus_init", rslt);

    rslt = bme68x_init(&bme);
    bme68x_check_rslt("bme68x_init", rslt);
    // sensor_bme_test();
    // bme_test_parallel_mode();
    // rslt = bme68x_selftest_check(&bme);
    // bme68x_check_rslt("bme68x_selftest_check", rslt);
    // if (rslt == BME68X_OK)
    // {
    //     printf("Self-test passed\r\n");
    // }
    // else if (rslt == BME68X_E_SELF_TEST)
    // {
    //     printf("Self-test failed\r\n");
    // }
}

void sensor_bme_loop(void)
{
    // bme_test_parallel_mode();
    sensor_bme_test();
    // osDelay(120);
}

#endif