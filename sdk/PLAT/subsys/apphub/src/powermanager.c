#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "powermanager.h"


#define POWER_MANAGER_LIST_SIZE     20


static uint32_t               gPowerManagerListCount                     = 0;
static PowerManagerFuncArrayT gPowerManagerList[POWER_MANAGER_LIST_SIZE] = {0};


void powerManagerInit(void)
{
    memset(gPowerManagerList, 0, sizeof(gPowerManagerList));
}

void powerManagerHandler(int32_t state)
{
    if (gPowerManagerListCount <= 0)
    {
        return;
    }

    switch (state)
    {
        case POWERMANAGER_FULL_POWER:
            for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
            {
                if (gPowerManagerList[i].full != NULL)
                {
                    gPowerManagerList[i].full();
                }
            }
            break;

        case POWERMANAGER_NORMAL_POWER:
            for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
            {
                if (gPowerManagerList[i].normal != NULL)
                {
                    gPowerManagerList[i].normal();
                }
            }
            break;

        case POWERMANAGER_LOW_POWER:
            for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
            {
                if (gPowerManagerList[i].low != NULL)
                {
                    gPowerManagerList[i].low();
                }
            }
            break;

        case POWERMANAGER_SAVE_POWER:
            for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
            {
                if (gPowerManagerList[i].save != NULL)
                {
                    gPowerManagerList[i].save();
                }
            }
            break;

        default:
            break;
    }
}

int32_t powerManagerRegister(PowerManagerFuncArrayT *funcArray)
{
    int32_t retVal = -1;

    if ((funcArray         == NULL)
     || (funcArray->full   == NULL)
     || (funcArray->normal == NULL)
     || (funcArray->low    == NULL)
     || (funcArray->save   == NULL))
    {
        SYSLOG_DEBUG("Param error.\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if ((gPowerManagerList[i].full   == funcArray->full)
         || (gPowerManagerList[i].normal == funcArray->normal)
         || (gPowerManagerList[i].low    == funcArray->low)
         || (gPowerManagerList[i].save   == funcArray->save))
        {
            memcpy(&gPowerManagerList[i], funcArray, sizeof(PowerManagerFuncArrayT));
            retVal = 0;
            goto labelEnd;
        }
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if ((gPowerManagerList[i].full   == NULL)
         && (gPowerManagerList[i].normal == NULL)
         && (gPowerManagerList[i].low    == NULL)
         && (gPowerManagerList[i].save   == NULL))
        {
            retVal = 0;
            memcpy(&gPowerManagerList[i], funcArray, sizeof(PowerManagerFuncArrayT));
            gPowerManagerListCount++;
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Power manager list is full: %d\r\n", POWER_MANAGER_LIST_SIZE);

labelEnd:
    return retVal;
}

int32_t powerManagerUnregister(PowerManagerFuncArrayT *funcArray)
{
    int32_t retVal = -1;

    if ((funcArray         == NULL)
     || (funcArray->full   == NULL)
     || (funcArray->normal == NULL)
     || (funcArray->low    == NULL)
     || (funcArray->save   == NULL))
    {
        SYSLOG_DEBUG("Param error.\n");
        goto labelEnd;
    }

    for (uint32_t i=0; i<POWER_MANAGER_LIST_SIZE; i++)
    {
        if ((gPowerManagerList[i].full   == funcArray->full)
         && (gPowerManagerList[i].normal == funcArray->normal)
         && (gPowerManagerList[i].low    == funcArray->low)
         && (gPowerManagerList[i].save   == funcArray->save))
        {
            retVal = 0;
            memset(&gPowerManagerList[i], 0, sizeof(PowerManagerFuncArrayT));
            gPowerManagerListCount--;
            goto labelEnd;
        }
    }

    SYSLOG_DEBUG("Power manager item don't exist.\r\n");

labelEnd:
    return retVal;
}
