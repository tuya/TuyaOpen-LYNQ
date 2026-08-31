#include <stdint.h>
#include "BaseObj.h"
#include "ec7xx_PWM.h"
#include "ec7xx_common.h"
#include "dataStrs.h"

#include "bsp.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "osasys.h"
#include "ostask.h"


// PikaStdDevice_PWM.c

int pika_hal_platform_PWM_open(pika_dev* dev, char* name) 
{
    // if (name[0] != 'U' || name[1] != 'A' || name[2] != 'R' || name[3] != 'T') {
    //     pika_debug("SPI Name:%s",name);
    //     return 1;
    // }
    return 0;
}

int pika_hal_platform_PWM_close(pika_dev* dev) 
{
    // if (NULL != dev->platform_data) {
    //     pikaFree(dev->platform_data, sizeof(ec7xx_spi_t));
    //     dev->platform_data = NULL;
    // }
    // pika_debug("SPI_close");
    return 0;
}

int pika_hal_platform_PWM_ioctl_config(pika_dev* dev,pika_hal_PWM_config* cfg) 
{
    // ec7xx_spi_t* ec718 = dev->platform_data;
    // dev->ioctl_config = cfg;
    // pika_debug("SPI_config %d,%d,%d,%d",cfg->baudrate,cfg->data_bits,cfg->stop_bits,cfg->parity);
    return 0;
}

int pika_hal_platform_PWM_ioctl_enable(pika_dev* dev) 
{
    // ec7xx_spi_t* ec718 = dev->platform_data;
    return 0;
}

int pika_hal_platform_PWM_ioctl_disable(pika_dev* dev) 
{
    // pika_debug("SPI_disable");
    return 0;
}

int pika_hal_platform_PWM_read(pika_dev* dev, void* buf, size_t count) 
{
    // ec7xx_spi_t* ec718 = dev->platform_data;
    // ARM_DRIVER_PWM * sUart = ec718->spi;

}

int pika_hal_platform_PWM_write(pika_dev* dev, void* buf, size_t count) 
{
    return 0;
}


