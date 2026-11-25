/**
 * @file lwip_config_cat.h
 * @brief Minimal lwIP options view used by tuyaopen-core translation units.
 *
 * The core only needs the BSD socket API declarations matching the EigenComm
 * lwIP library ABI. The vendor build keeps its own lwIP configuration.
 *
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
#ifndef CORE_LWIP_CONFIG_CAT_H
#define CORE_LWIP_CONFIG_CAT_H

#include "lwip/debug.h"

#define LWIP_NO_ARCH_CPU_H  1
#define LWIP_NO_ARCH_PERF_H 1

#define LWIP_IPV4 1
#define LWIP_IPV6 1

#define LWIP_SOCKET                 1
#define LWIP_NETCONN                1
#define LWIP_COMPAT_SOCKETS         1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1
#define LWIP_SOCKET_SELECT          1
#define LWIP_SOCKET_POLL            1
#define LWIP_SOCKET_OFFSET          0

#define LWIP_DNS           1
#define LWIP_TCP           1
#define LWIP_UDP           1
#define LWIP_RAW           1
#define LWIP_ICMP          1
#define LWIP_TCP_KEEPALIVE 1
#define SO_REUSE           1
#define LWIP_SO_SNDTIMEO   1
#define LWIP_SO_RCVTIMEO   1
#define LWIP_SO_RCVBUF     1
#define LWIP_SO_SNDBUF     1
#define LWIP_SO_LINGER     1

#define LWIP_TIMEVAL_PRIVATE 0

#define MEMP_NUM_NETCONN 23

#define LWIP_ERRNO_INCLUDE <errno.h>

#endif /* CORE_LWIP_CONFIG_CAT_H */
