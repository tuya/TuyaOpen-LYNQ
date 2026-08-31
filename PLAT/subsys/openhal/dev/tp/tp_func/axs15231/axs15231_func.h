#ifndef _AXS15231_FUNC_H
#define _AXS15231_FUNC_H

#include <stdbool.h>
#include <stdint.h>
#include "api_tp.h"

#define AXS_ADDR 0x3B

typedef struct TpDevAxs15231_
{
    uint16_t bus_type;
    uint8_t slave_addr;
    uint32_t i2c_id;
    int32_t gpio_irq;
    const TsFunc_t *hyn_fuc_used;
} TpDevAxs15231_t;


int axs15231_dev_init(uint8_t port_id, uint32_t speed,
                         void (*tp_bus_cb_func)(uint32_t event));
TpDevAxs15231_t *axs15231_dev_get(void);


#endif
