#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
#include "jsonDb.h"
#endif
#include "phoneCallList.h"


#define CALL_LIST_JSON_PATH           "D:/callListDb.json"
#define CALL_LIST_TYPE_PATH           "D:/callListDb.type"


static char *getCallListJsonPath(void)
{
    return CALL_LIST_JSON_PATH;
}

static char *getCallListTypePath(void)
{
    return CALL_LIST_TYPE_PATH;
}

int32_t addCallList(PhoneCallListT *callList)
{
    int32_t           retVal      = PHONE_STATUS_ERROR;
    PhoneCallListStrT callListStr = {0};

    if (callList == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    memset(&callListStr, 0, sizeof(callListStr));
    memcpy(callListStr.number, callList->number, strlen(callList->number));
    itoa(callList->type,      callListStr.type,      10);
    itoa(callList->sim,       callListStr.sim,       10);
    itoa(callList->time,      callListStr.time,      10);
    itoa(callList->timeStamp, callListStr.timeStamp, 10);

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbAddItem(getCallListJsonPath(), getCallListTypePath(), &callListStr);
#endif

labelEnd:
    return retVal;
}

int32_t deleteCallList(uint32_t index)
{
    int32_t retVal = PHONE_STATUS_ERROR;

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbDeleteItem(getCallListJsonPath(), getCallListTypePath(), index);
#endif

    return retVal;
}

int32_t getCallList(uint32_t index, PhoneCallListT *callList)
{
    int32_t            retVal = PHONE_STATUS_ERROR;
    PhoneCallListStrT *item   = NULL; 

    if (callList == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

#ifdef FEATURE_SUBSYS_JSONDB_ENABLE
    retVal = jsonDbGetItemById(getCallListJsonPath(), getCallListTypePath(), index, (void **)&item);
#endif
    if (item != NULL)
    {
        memset(callList, 0, sizeof(PhoneCallListT));
        memcpy(callList->number, item->number, strlen(item->number));
        callList->type      = atoi(item->type);
        callList->sim       = atoi(item->sim);
        callList->time      = atoi(item->time);
        callList->timeStamp = atoi(item->timeStamp);

#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(item);
#else
        free(item);
#endif
        item = NULL;
    }

labelEnd:
    return retVal;
}

int32_t getCallListCount(uint32_t *count)
{
    int32_t retVal = PHONE_STATUS_ERROR;

    if (count == NULL)
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    *count = jsonDbGetTotalCount(getCallListJsonPath(), getCallListTypePath());
    retVal = PHONE_STATUS_OK;

labelEnd:
    return retVal;
}

int32_t getCallListOneItemCount(char *number, uint32_t *count)
{
    int32_t         retVal   = PHONE_STATUS_ERROR;
    PhoneCallListT *callList = NULL;
    uint32_t       *ids      = NULL;
    int32_t         resVal   = -1;

    if ((number == NULL) || (count == NULL))
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    resVal = jsonDbGetMatchedItemsByStr(getCallListJsonPath(), getCallListTypePath(), number, (void **)&callList, &ids);
    if (resVal >= 0)
    {
        *count = resVal;
        retVal = PHONE_STATUS_OK;
    }

labelEnd:
    if (callList != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(callList);
#else
        free(callList);
#endif
        callList = NULL;
    }
    if (ids != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(ids);
#else
        free(ids);
#endif
        ids = NULL;
    }

    return retVal;
}

int32_t getCallListOneItemIds(char *number, uint32_t *ids, uint32_t *length)
{
    int32_t         retVal   = PHONE_STATUS_ERROR;
    PhoneCallListT *callList = NULL;
    uint32_t       *id       = NULL;
    int32_t         resVal   = -1;

    if ((number == NULL) || (ids == NULL) || (length == NULL))
    {
        SYSLOG_DEBUG("param error\r\n");
        goto labelEnd;
    }

    resVal = jsonDbGetMatchedItemsByStr(getCallListJsonPath(), getCallListTypePath(), number, (void **)&callList, &id);
    if (resVal >= 0)
    {
        *length = MIN(resVal, *length);
        memcpy(ids, id, (*length) * 4);
        retVal = PHONE_STATUS_OK;
    }

labelEnd:
    if (callList != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(callList);
#else
        free(callList);
#endif
        callList = NULL;
    }
    if (id != NULL)
    {
#if (defined(PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST == 1) && (!(defined(TYPE_EC718M))))
        vPortFree_Psram(ids);
#else
        free(id);
#endif
        id = NULL;
    }

    return retVal;
}
