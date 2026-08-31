#ifndef _ec7xx_COMMON_H
#define _ec7xx_COMMON_H
#include "PikaObj.h"
#include "../PikaStdDevice/pika_hal.h"
#include <stdint.h>


#undef u16
#undef u8
#undef u32
#define u16 uint16_t
#define u8 uint8_t
#define u32 uint32_t



uint16_t GPIO_get_pin(char* pin);
uint32_t getPinMode(char* mode);
uint8_t GPIO_enable_clock(char* pin);
void delay_us(uint32_t delay);
void delay_unit(uint32_t delay);

#endif
