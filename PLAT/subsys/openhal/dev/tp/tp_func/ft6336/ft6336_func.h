#ifndef _FT6336_FUNC_H
#define _FT6336_FUNC_H

#include <stdbool.h>
#include <stdint.h>
#include "api_tp.h"

#define FT6336_ADDR 	          0x38
#define FT6336_GET_FINGERNUM 	  0x02
#define FT6336_GET_LOC0 	      0x03
#define FT6336_GET_LOC1 	      0x09
#define FT6336_OPERATE_MODE 	  0xBC

typedef struct TpDevFt6336_
{
    uint16_t bus_type;
    uint8_t slave_addr;
    uint32_t i2c_id;
    int32_t gpio_irq;
    const TsFunc_t *hyn_fuc_used;
} TpDevFt6336_t;


int ft6336_dev_init(uint8_t port_id, uint32_t speed,
                         void (*tp_bus_cb_func)(uint32_t event));
TpDevFt6336_t *ft6336_dev_get(void);

#endif