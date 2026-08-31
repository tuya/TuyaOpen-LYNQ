#ifndef _FT77922_FUNC_H
#define _FT77922_FUNC_H

#include <stdbool.h>
#include <stdint.h>
#include "api_tp.h"

#define ST77922_ADDR 0x55

typedef struct TpDevSt77922_
{
    uint16_t bus_type;
    uint8_t slave_addr;
    uint32_t i2c_id;
    int32_t gpio_irq;
    const TsFunc_t *hyn_fuc_used;
} TpDevSt77922_t;


int st77922_dev_init(uint8_t port_id, uint32_t speed,
                         void (*tp_bus_cb_func)(uint32_t event));
TpDevSt77922_t *st77922_dev_get(void);

#endif
