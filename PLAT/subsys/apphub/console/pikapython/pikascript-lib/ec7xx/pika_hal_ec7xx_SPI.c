#include <stdint.h>
#include "BaseObj.h"
#include "ec7xx_SPI.h"
#include "ec7xx_common.h"
#include "dataStrs.h"

#include "bsp.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "osasys.h"
#include "ostask.h"


// PikaStdDevice_SPI.c

typedef struct
{
    uint8_t port;
    ARM_DRIVER_SPI * spi; 
} ec7xx_spi_t;




ARM_DRIVER_SPI *spi_drv_get(uint8_t symbol)
{
extern ARM_DRIVER_SPI   Driver_SPI0;
    if(symbol==0) return &CREATE_SYMBOL(Driver_SPI, 0);
// extern ARM_DRIVER_SPI   Driver_SPI1;
//     if(symbol==1) return &CREATE_SYMBOL(Driver_SPI, 1);
    return NULL;
}


int pika_hal_platform_SPI_open(pika_dev* dev, char* name) 
{
    int spi_num = fast_atoi(name + 3);
    if (name[0] != 'S' || name[1] != 'P' || name[2] != 'I' || spi_num > 3) {
        pika_debug("SPI Name:%s",name);
        return 1;
    }
    dev->platform_data = pikaMalloc(sizeof(ec7xx_spi_t));
    if (dev->platform_data == NULL) {
        pika_debug("SPI pikaMalloc error");
        return 2;
    }
    ec7xx_spi_t* ec718 = dev->platform_data;
    ec718->port = spi_num;
    ec718->spi = spi_drv_get(ec718->port);
    pika_debug("open SPI%d 0x%X",ec718->port,ec718->spi);
    return 0;
}

int pika_hal_platform_SPI_close(pika_dev* dev) 
{
    if (NULL != dev->platform_data) {
        pikaFree(dev->platform_data, sizeof(ec7xx_spi_t));
        dev->platform_data = NULL;
    }
    pika_debug("SPI_close");
    return 0;
}

int pika_hal_platform_SPI_ioctl_config(pika_dev* dev,pika_hal_SPI_config* cfg) 
{
    // ec7xx_spi_t* ec718 = dev->platform_data;
    // dev->ioctl_config = cfg;
    // pika_debug("SPI_config %d,%d,%d,%d",cfg->baudrate,cfg->data_bits,cfg->stop_bits,cfg->parity);
    return 0;
}

int pika_hal_platform_SPI_ioctl_enable(pika_dev* dev) 
{
    pika_debug("SPI_enable");
    // ec7xx_spi_t* ec718 = dev->platform_data;
    // pika_hal_SPI_config* cfg = (pika_hal_SPI_config*)dev->ioctl_config;
    return 0;
}

int pika_hal_platform_SPI_ioctl_disable(pika_dev* dev) 
{
    ec7xx_spi_t* ec718 = dev->platform_data;
    ARM_DRIVER_SPI * sUart = ec718->spi;
    if (NULL == sUart) {
        pika_debug("SPI NULL");
        return 1;
    }
    pika_debug("SPI_disable");
    return 0;
}

int pika_hal_platform_SPI_read(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_spi_t* ec718 = dev->platform_data;
    ARM_DRIVER_SPI * sUart = ec718->spi;
    if (NULL == sUart) {
        pika_debug("SPI NULL");
        return 1;
    }
    pika_debug("SPI_read 0x%X %d",sUart,count);

}

int pika_hal_platform_SPI_write(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_spi_t* ec718 = dev->platform_data;
    ARM_DRIVER_SPI * sUart = ec718->spi;
    if (NULL == sUart) {
        pika_debug("SPI NULL");
        return 1;
    }
    pika_debug("SPI_write 0x%X %d",sUart,count);
    // sUart->GetRxCount();
    return 0;
}


