/****************************************************************************
 *
 * Copy right:   2023 -, Copyrigths of EigenComm Ltd.
 * File name:    opensdk.c
 * Description:  ec7xx opensdk entry source file
 * History:      Rev1.0   2023-11-02
 *
 ****************************************************************************/
#ifdef FEATURE_SUBSYS_OPENSDK_ENABLE
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "slpman.h"
#include "bsp.h"
#include "opensdk.h"
#ifdef FEATURE_SUBSYS_AUDIO_ENABLE
#include "audio.h"
#endif
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_APP_MANAGER_ENABLE
#include "appmanager.h"
#endif


typedef struct GlobalmapStructT
{
    struct GlobalmapStructT *next;
    uint32_t                index;
    uint32_t                length;
    void                    *buffer;
} GlobalmapT;

static GlobalmapT *gGlobalmap = NULL;


/**
  \fn          opensdkInit()
  \brief       opensdk Init entry function.
  \return
*/
void opensdkInit(void)
{
#if defined(CHIP_EC718)
    uint32_t appAddr = __ocsyscall_app[7];
#elif defined(CHIP_EC716)
    uint32_t appAddr = PKGFLXAPP_APP0_LNA;
#endif
    *((uint32_t *)0x17FF006C) = 16;

    if ((strstr((char *)appAddr, "app")))
    {
        SYSLOG_INFO("Jump to APP%c: 0x%X\r\n", *(((uint8_t *)appAddr)+3), appAddr);
        appInstall("app0", appAddr);
        appRun("app0");
    }
    else
    {
        SYSLOG_ERR("APP invalid: 0x%X\r\n", appAddr);
    }
}

int32_t globalmapInit(uint32_t index, void *data, uint32_t length)
{
    int32_t    retVal     = -1;
    GlobalmapT *globalmap = gGlobalmap;

    while ((globalmap != NULL) && (globalmap->next != NULL))
    {
        globalmap = globalmap->next;
    }

    if (globalmap == NULL)
    {
        globalmap = malloc(sizeof(GlobalmapT));
        if (globalmap == NULL)
        {
            SYSLOG_ERR("Failed to malloc %d bytes for globalmap.\r\n", sizeof(GlobalmapT));
            return retVal;
        }
        gGlobalmap = globalmap;
    }
    else
    {
        globalmap->next = malloc(sizeof(GlobalmapT));
        if (globalmap->next == NULL)
        {
            SYSLOG_ERR("Failed to malloc %d bytes for globalmap->next.\r\n", sizeof(GlobalmapT));
            return retVal;
        }
        globalmap = globalmap->next;
    }

    memset(globalmap, 0, sizeof(GlobalmapT));
    globalmap->next   = NULL;
    globalmap->index  = index;
    globalmap->length = length;
    globalmap->buffer = malloc(length);
    if (globalmap->buffer == NULL)
    {
        SYSLOG_ERR("Failed to malloc %d bytes for globalmap->buffer.\r\n", length);
        return retVal;
    }

    if (data == NULL)
    {
        memset(globalmap->buffer, 0, length);
    }
    else
    {
        memcpy(globalmap->buffer, data, length);
    }

    retVal = 0;

    return retVal;
}

int32_t globalItemSet(uint32_t index, void *data)
{
    int32_t    retVal     = -1;
    GlobalmapT *globalmap = gGlobalmap;

    while ((globalmap != NULL) && (globalmap->index != index))
    {
        globalmap = globalmap->next;
    }

    if (globalmap->index == index)
    {
        memcpy(globalmap->buffer, data, globalmap->length);
        retVal = 0;
    }

    return retVal;
}

void *globalItemGet(uint32_t index)
{
    GlobalmapT *globalmap = gGlobalmap;

    while ((globalmap != NULL) && (globalmap->index != index))
    {
        globalmap = globalmap->next;
    }

    return ((globalmap->index == index) ? (globalmap->buffer) : NULL);
}

#endif
