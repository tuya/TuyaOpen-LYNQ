/**
 * @file arch/cc.h
 * @brief Compiler abstraction for the core's lwIP header view on L511.
 *
 * Header-only definitions used while compiling tuyaopen-core translation
 * units. The platform lwIP library keeps its own compiler abstraction.
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define LWIP_PLATFORM_BYTESWAP 0
#define LWIP_NOASSERT          1
#define LWIP_NO_STDINT_H       1
#define LWIP_NO_INTTYPES_H     1

typedef uint8_t   u8_t;
typedef int8_t    s8_t;
typedef uint16_t  u16_t;
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;
typedef uint64_t  u64_t;
typedef uintptr_t mem_ptr_t;
typedef int       sys_prot_t;

#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define SZT_F "u"

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT   __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_PLATFORM_DIAG(x)                                                                                          \
    do {                                                                                                               \
    } while (0)
#define LWIP_PLATFORM_ASSERT(x)                                                                                        \
    do {                                                                                                               \
    } while (0)

#endif /* __ARCH_CC_H__ */
