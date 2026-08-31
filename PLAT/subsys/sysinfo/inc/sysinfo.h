#ifndef __SYSINFO_H__
#define __SYSINFO_H__


#include <stdint.h>


#define SYSINFO_IMEI_LENGTH         16
#define SYSINFO_SN_LENGTH           32
#define SYSINFO_ICCID_LENGTH        20
#define SYSINFO_MODEL_LENGTH        5
#define SYSINFO_VENDOR_LENGTH       9
#define SYSINFO_MAC_LENGTH          17
#define SYSINFO_IP_LENGTH           15
#define SYSINFO_OS_NAME_LENGTH      8
#define SYSINFO_OS_VERSION_LENGTH   6
#define SYSINFO_CHIP_ARCH_LENGTH    3
#define SYSINFO_CPU_TYPE_LENGTH     9


typedef enum
{
    SYSINFO_SN = 0,
    SYSINFO_IMEI,
    SYSINFO_MODEL,
    SYSINFO_VENDOR,
    SYSINFO_MAC,
    SYSINFO_ICCID,
    SYSINFO_IP,
    SYSINFO_NETMASK,
    SYSINFO_GATEWAY,
    SYSINFO_OS_NAME,
    SYSINFO_OS_VERSION,
    SYSINFO_CHIP_ARCH,
    SYSINFO_CPU_TYPE,
    SYSINFO_MAX
} SysinfoT;


int32_t getSysinfo(uint8_t type, char *out);


#endif
