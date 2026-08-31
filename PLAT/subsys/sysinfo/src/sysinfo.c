#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "mw_aon_info.h"
#include "ps_lib_api.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "sysinfo.h"


static int32_t getImei(char *imei)
{
    int32_t retVal = -1;
    char    buffer[SYSINFO_IMEI_LENGTH + 1] = {0};

    if (imei != NULL)
    {
        memset(buffer, 0, sizeof(buffer));
        if (appGetImeiNumSync(buffer) == CMS_RET_SUCC)
        {
            memcpy(imei, buffer, SYSINFO_IMEI_LENGTH);
            retVal = 0;
        }
    }

    return retVal;
}

static int32_t getSn(char *sn)
{
    int32_t retVal = -1;
    char    buffer[SYSINFO_SN_LENGTH + 1] = {0};

    if (sn != NULL)
    {
        memset(buffer, 0, sizeof(buffer));
        if (appGetSNNumSync(buffer) == TRUE)
        {
            memcpy(sn, buffer, SYSINFO_SN_LENGTH);
            retVal = 0;
        }
        else
        {
            retVal = getImei(sn);
        }
    }

    return retVal;
}

static int32_t getIccid(char *iccid)
{
    int32_t retVal = -1;
    char    buffer[SYSINFO_ICCID_LENGTH + 1] = {0};

    if (iccid != NULL)
    {
        memset(buffer, 0, sizeof(buffer));
        if (appGetIccidNumSync(buffer) == CMS_RET_SUCC)
        {
            memcpy(iccid, buffer, SYSINFO_ICCID_LENGTH);
            retVal = 0;
        }
    }

    return retVal;
}

static int32_t getModel(char *model)
{
    int32_t  retVal    = -1;
    char    *modelName = "EC718";

    if (model != NULL)
    {
        memcpy(model, modelName, SYSINFO_MODEL_LENGTH);
        retVal = 0;
    }

    return retVal;
}

static int32_t getVendor(char *vendor)
{
    int32_t  retVal     = -1;
    char    *vendorName = "EigenComm";

    if (vendor != NULL)
    {
        memcpy(vendor, vendorName, SYSINFO_VENDOR_LENGTH);
        retVal = 0;
    }

    return retVal;
}

static int32_t getMac(char *mac)
{
    int32_t retVal     = -1;
    uint8_t macData[6] = {0};

    if (mac != NULL)
    {
        memset(macData, 0, sizeof(macData));
        if (appGetLocalMacSync(macData) == CMS_RET_SUCC)
        {
            snprintf(mac, SYSINFO_MAC_LENGTH + 1, "%02x:%02x:%02x:%02x:%02x:%02x", macData[0], macData[1], macData[2], macData[3], macData[4], macData[5]);
            retVal = 0;
        }
    }

    return retVal;
}

static int32_t getIpAddress(char *ip)
{
    int32_t          retVal      = -1;
    MWAonNetParamCfg netParamCfg = {0};

    if (ip != NULL)
    {
        mwAonGetNetParamCfg(&netParamCfg);
        snprintf(ip, SYSINFO_IP_LENGTH + 1, "%d.%d.%d.%d", netParamCfg.localHostAddr[0], netParamCfg.localHostAddr[1], netParamCfg.localHostAddr[2], netParamCfg.localHostAddr[3]);
        retVal = 0;
    }

    return retVal;
}

static int32_t getNetMask(char *mask)
{
    int32_t          retVal      = -1;
    MWAonNetParamCfg netParamCfg = {0};

    if (mask != NULL)
    {
        mwAonGetNetParamCfg(&netParamCfg);
        snprintf(mask, SYSINFO_IP_LENGTH + 1, "%d.%d.%d.%d", netParamCfg.mask[0], netParamCfg.mask[1], netParamCfg.mask[2], netParamCfg.mask[3]);
        retVal = 0;
    }

    return retVal;
}

static int32_t getGateway(char *gateway)
{
    int32_t          retVal      = -1;
    MWAonNetParamCfg netParamCfg = {0};

    if (gateway != NULL)
    {
        mwAonGetNetParamCfg(&netParamCfg);
        snprintf(gateway, SYSINFO_IP_LENGTH + 1, "%d.%d.%d.%d", netParamCfg.gateway[0], netParamCfg.gateway[1], netParamCfg.gateway[2], netParamCfg.gateway[3]);
        retVal = 0;
    }

    return retVal;
}

static int32_t getOsName(char *name)
{
    int32_t  retVal = -1;
    char    *osName = "FreeRTOS";

    if (name != NULL)
    {
        memcpy(name, osName, SYSINFO_OS_NAME_LENGTH);
        retVal = 0;
    }

    return retVal;
}

static int32_t getOsVersion(char *version)
{
    int32_t retVal = -1;

    if (version != NULL)
    {
        memcpy(version, tskKERNEL_VERSION_NUMBER, SYSINFO_OS_VERSION_LENGTH);
        retVal = 0;
    }

    return retVal;
}

static int32_t getChipArch(char *arch)
{
    int32_t  retVal   = -1;
    char    *chipArch = "arm";

    if (arch != NULL)
    {
        memcpy(arch, chipArch, SYSINFO_CHIP_ARCH_LENGTH);
        retVal = 0;
    }

    return retVal;
}

static int32_t getCpuType(char *cpu)
{
    int32_t  retVal  = -1;
    char    *cpuType = "Cortex-M3";

    if (cpu != NULL)
    {
        memcpy(cpu, cpuType, SYSINFO_CPU_TYPE_LENGTH);
        retVal = 0;
    }

    return retVal;
}

int32_t getSysinfo(uint8_t type, char *out)
{
    int32_t retVal = -1;

    switch (type)
    {
        case SYSINFO_SN:
            retVal = getSn(out);
            break;

        case SYSINFO_IMEI:
            retVal = getImei(out);
            break;

        case SYSINFO_MODEL:
            retVal = getModel(out);
            break;

        case SYSINFO_VENDOR:
            retVal = getVendor(out);
            break;

        case SYSINFO_MAC:
            retVal = getMac(out);
            break;

        case SYSINFO_ICCID:
            retVal = getIccid(out);
            break;

        case SYSINFO_IP:
            retVal = getIpAddress(out);
            break;

        case SYSINFO_NETMASK:
            retVal = getNetMask(out);
            break;

        case SYSINFO_GATEWAY:
            retVal = getGateway(out);
            break;

        case SYSINFO_OS_NAME:
            retVal = getOsName(out);
            break;

        case SYSINFO_OS_VERSION:
            retVal = getOsVersion(out);
            break;

        case SYSINFO_CHIP_ARCH:
            retVal = getChipArch(out);
            break;

        case SYSINFO_CPU_TYPE:
            retVal = getCpuType(out);
            break;

        default:
            break;
    }

    return retVal;
}

#ifdef SYSINFO_TEST
void sysinfoTest(void)
{
    int32_t resVal      = 0;
    char    buffer[256] = {0};

    for (uint32_t i=0; i<SYSINFO_MAX; i++)
    {
        memset(buffer, 0, sizeof(buffer));
        resVal = getSysinfo(i, buffer);
        SYSLOG_DEBUG("[%d] %s\r\n", i, (resVal == 0) ? buffer : "Failed");
    }
}
#endif
