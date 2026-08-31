#include <stdint.h>
#include "BaseObj.h"
#include "ec7xx_IIC.h"
#include "ec7xx_common.h"
#include "dataStrs.h"

#include "bsp.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"
#include "osasys.h"
#include "ostask.h"


// PikaStdDevice_IIC.c

typedef struct
{
    uint8_t port;
    ARM_DRIVER_I2C * i2c; 
} ec7xx_i2c_t;



ARM_DRIVER_I2C *i2c_drv_get(uint8_t symbol)
{
extern ARM_DRIVER_I2C   Driver_I2C0;
    if(symbol==0) return &CREATE_SYMBOL(Driver_I2C, 0);
// extern ARM_DRIVER_I2C   Driver_I2C1;
//     if(symbol==1) return &CREATE_SYMBOL(Driver_I2C, 1);
    return NULL;
}

int pika_hal_platform_IIC_open(pika_dev* dev, char* name) 
{
    int iic_num = fast_atoi(name + 3);
    if (name[0] != 'I' || name[1] != 'I' || name[2] != 'C' || iic_num > 2) {
        pika_debug("IIC %s Name Error",name);
        return 1;
    }
    dev->platform_data = pikaMalloc(sizeof(ec7xx_i2c_t));
    if (dev->platform_data == NULL) {
        pika_debug("IIC pikaMalloc error");
        return 2;
    }
    ec7xx_i2c_t* ec718 = dev->platform_data;
    ec718->port = iic_num;
    ec718->i2c = i2c_drv_get(ec718->port);
    pika_debug("open IIC%d 0x%X",ec718->port,ec718->i2c);
    return 0;
}

int pika_hal_platform_IIC_close(pika_dev* dev) 
{
    if (NULL != dev->platform_data) {
        pikaFree(dev->platform_data, sizeof(ec7xx_i2c_t));
        dev->platform_data = NULL;
    }
    pika_debug("IIC_close");
    return 0;
}

int pika_hal_platform_IIC_ioctl_config(pika_dev* dev,pika_hal_IIC_config* cfg) 
{
    ec7xx_i2c_t* ec718 = dev->platform_data;
    // dev->ioctl_config = cfg;
    // pika_debug("IIC_config %d,%d,%d,%d",cfg->baudrate,cfg->data_bits,cfg->stop_bits,cfg->parity);
    return 0;
}

int pika_hal_platform_IIC_ioctl_enable(pika_dev* dev) 
{
    return 0;
}

int pika_hal_platform_IIC_ioctl_disable(pika_dev* dev) 
{
    ec7xx_i2c_t* ec718 = dev->platform_data;
    ARM_DRIVER_I2C * sUart = ec718->i2c;
    if (NULL == sUart) {
        pika_debug("IIC NULL");
        return 1;
    }
    pika_debug("IIC_disable");
    return 0;
}

int pika_hal_platform_IIC_read(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_i2c_t* ec718 = dev->platform_data;
    ARM_DRIVER_I2C * sUart = ec718->i2c;
    if (NULL == sUart) {
        pika_debug("IIC NULL");
        return 1;
    }
    pika_debug("IIC_read 0x%X %d",sUart,count);
    // sUart->Receive(buf, 100);
    // uart_read_bytes(ec718->uartPort, buf, count, 100);
    // return sUart->Receive(buf, count);
}

int pika_hal_platform_IIC_write(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_i2c_t* ec718 = dev->platform_data;
    ARM_DRIVER_I2C * sUart = ec718->i2c;
    if (NULL == sUart) {
        pika_debug("IIC NULL");
        return 1;
    }
    pika_debug("IIC_write 0x%X %d",sUart,count);
    // sUart->GetRxCount();
    return 0;
}


