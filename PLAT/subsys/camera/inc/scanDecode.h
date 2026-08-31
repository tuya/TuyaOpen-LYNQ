#ifndef __SCAN_DECODE_H__
#define __SCAN_DECODE_H__


#include <stdint.h>


int32_t scanDecodeInit(void);
int32_t scanAndDecode(uint8_t *buffer, uint32_t *size, uint32_t timeout);


#endif
