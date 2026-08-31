
#ifndef __BH1750_H__
#define __BH1750_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "bh1750_defs.h"

// 定义BH1750的设备地址和控制字节
#define BH1750_7b_ADDR      0x23


uint8_t bh1750_init(bus_t bus);
uint16_t bh1750_lux_once(bus_t bus);

#ifdef __cplusplus
}
#endif
#endif
