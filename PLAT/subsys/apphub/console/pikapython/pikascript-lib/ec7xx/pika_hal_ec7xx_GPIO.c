#include <stdint.h>
#include "BaseObj.h"
#include "ec7xx_GPIO.h"
#include "ec7xx_common.h"
#include "dataStrs.h"

#include "gpio.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
// PikaStdDevice_GPIO.c
// pika_hal.h

typedef struct
{
    uint8_t pad;
    uint8_t pin;
    uint8_t instance;
    PadConfig_t PadConfig;
    GpioPinConfig_t PinConfig;  
} ec7xx_gpio_t;

int pika_hal_platform_GPIO_open(pika_dev* dev, char* name) 
{
    if (strIsStartWith(name, "GPIO")) {
        pika_debug("GPIO Name error:%s",name);
        return 1;
    }
    uint32_t gpio = name[0];
    dev->platform_data = pikaMalloc(sizeof(ec7xx_gpio_t));
    if (dev->platform_data == NULL) {
        pika_debug("pikaMalloc error");
        return 2;
    }
    ec7xx_gpio_t* ec718 = dev->platform_data;
    // ec718->instance = gpio / 16;
    // ec718->pin = gpio % 16;
    // ec718->pad = ec7xx_list_gpio2pad[gpio];
    // PAD_getDefaultConfig(&(ec718->PadConfig));
    // if(ec718->pad > 14) ec718->PadConfig.mux = PAD_MUX_ALT0;
    // else ec718->PadConfig.mux = PAD_MUX_ALT4;
    pika_debug("GPIO:%d,%d,%d,%d",gpio,ec718->instance,ec718->pin,ec718->pad);
    return 0;
}

int pika_hal_platform_GPIO_close(pika_dev* dev) 
{
    if (NULL != dev->platform_data) {
        pikaFree(dev->platform_data, sizeof(ec7xx_gpio_t));
        dev->platform_data = NULL;
    }
    pika_debug("GPIO_close");
    return 0;
}

int pika_hal_platform_GPIO_ioctl_config(pika_dev* dev, pika_hal_GPIO_config* cfg) 
{
    int flag = 0;
    ec7xx_gpio_t* ec718 = dev->platform_data;
    if(cfg->dir==PIKA_HAL_GPIO_DIR_OUT) {
        flag ++;
        ec718->PinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
    }
    else if(cfg->dir==PIKA_HAL_GPIO_DIR_IN) {
        flag ++;
        ec718->PinConfig.pinDirection = GPIO_DIRECTION_INPUT;
        // pin->PinConfig.misc.initOutput = 0;
    }
    GPIO_pinConfig(ec718->instance, ec718->pin, &(ec718->PinConfig));
    if(cfg->pull==PIKA_HAL_GPIO_PULL_UP) {
        flag ++;
        ec718->PadConfig.pullSelect = PAD_PULL_INTERNAL;
        ec718->PadConfig.pullUpEnable = PAD_PULL_UP_ENABLE;
        ec718->PadConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    }
    else if(cfg->pull==PIKA_HAL_GPIO_PULL_DOWN) {
        flag ++;
        ec718->PadConfig.pullSelect = PAD_PULL_INTERNAL;
        ec718->PadConfig.pullUpEnable = PAD_PULL_UP_DISABLE;
        ec718->PadConfig.pullDownEnable = PAD_PULL_DOWN_ENABLE;
    }
    PAD_setPinConfig(ec718->pad, &(ec718->PadConfig));
    // pika_debug("GPIO_ioctl %d",cfg->dir);
    return 0;
}

int pika_hal_platform_GPIO_ioctl_enable(pika_dev* dev) 
{
    ec7xx_gpio_t* ec718 = dev->platform_data;
    if(ec718 == NULL) {
        pika_debug("GPIO_ioctl_enable error");
        return 1;
    }
    dev->is_enabled = PIKA_TRUE;
    pika_debug("GPIO_ioctl_enable %d",dev->type);
    return 0;
}

int pika_hal_platform_GPIO_ioctl_disable(pika_dev* dev) 
{
    dev->is_enabled = PIKA_FALSE;
    ec7xx_gpio_t* ec718 = dev->platform_data;
    pika_debug("GPIO_ioctl_disable %d",dev->type);
    return 0;
}


int pika_hal_platform_GPIO_read(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_gpio_t* ec718 = dev->platform_data;
    if(ec718 == NULL) {
        pika_debug("GPIO_read error");
        return 1;
    }
    *(uint32_t*)buf = GPIO_pinRead(ec718->instance,ec718->pin);
    pika_debug("GPIO%d=%d",ec718->pin,*(uint32_t*)buf);
    return 0;
}

int pika_hal_platform_GPIO_write(pika_dev* dev, void* buf, size_t count) 
{
    ec7xx_gpio_t* ec718 = dev->platform_data;
    uint32_t level = *(uint32_t*)buf;
    if(ec718 == NULL || level>1) {
        pika_debug("GPIO_write %d error",level);
        return 1;
    }
    GPIO_pinWrite(ec718->instance, 1 << ec718->pin, level << ec718->pin);
    pika_debug("GPIO%d->%d",ec718->pin,level);
    return 0;
}

